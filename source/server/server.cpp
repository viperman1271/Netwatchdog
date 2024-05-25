#include "server.h"

#include "communication.h"
#include "utils.h"

#include <iostream>
#include <sstream>

using namespace std::chrono_literals;

NetWatchdogServer::NetWatchdogServer(const Options& options)
    : m_Port(options.server.port)
    , m_ListenAddress(options.server.host)
    , m_Identity(options.server.identity)
    , m_ShouldContinue(true)
    , m_Options(options)
{
}

void NetWatchdogServer::Run()
{
    zmq::context_t context(1);
    {
        m_ServerSocket = zmq::socket_t(context, zmq::socket_type::router);

        const unsigned long long timeoutMs = std::chrono::duration_cast<std::chrono::milliseconds>(1s).count();
        m_ServerSocket.set(zmq::sockopt::rcvtimeo, static_cast<int>(timeoutMs));

        const unsigned long long lingerMs = std::chrono::duration_cast<std::chrono::milliseconds>(1s).count();
        m_ServerSocket.set(zmq::sockopt::linger, static_cast<int>(lingerMs));

        zmq_socket_monitor(m_ServerSocket, "inproc://monitor", ZMQ_EVENT_CONNECTED | ZMQ_EVENT_DISCONNECTED);
        
        std::stringstream ss;
        ss << "tcp://" << m_ListenAddress << ":" << m_Port;
        std::cout << "Binding server to " << ss.str() << std::endl;

        m_ServerSocket.bind(ss.str());
    }

    {
		const int events = ZMQ_EVENT_CONNECTED | ZMQ_EVENT_DISCONNECTED;
        m_ConnectionMonitor.reset(new ConnectionMonitor());
		m_ConnectionMonitor->Init(m_ServerSocket, "inproc://conmon", events);
		m_ConnectionMonitor->SetCallback(ZMQ_EVENT_DISCONNECTED, [this](const zmq_event_t& zmqEvent, const char* addr) { HandleClientDisconnected(zmqEvent, addr); });
		m_MonitorThread = std::thread([this]()
		{
			Monitor();
		});
    }

    while (m_ShouldContinue.load())
    {
        std::string identity;
        std::shared_ptr<GenericMessage> msg = Communication::RecvMessage<GenericMessage, MessageType::GenericMessage>(m_ServerSocket, identity);
        if (msg != nullptr)
        {
            HandleClientConnected(msg->GetId());
            {
                std::lock_guard<std::mutex> lock(m_ClientsLock);
                m_ConnectedClients.push_back(msg->GetId());
            }

            GenericMessage respMessage;
            respMessage.SetId(m_Identity);
            respMessage.SetSuccess(true);
            Communication::SendMessage(respMessage, m_ServerSocket, identity);
        }
    };

    m_ServerSocket.close();
    context.close();

    if (m_MonitorThread.joinable())
    {
        m_MonitorThread.join();
    }
}

void NetWatchdogServer::Kill()
{
    m_ShouldContinue.store(false);
}

void NetWatchdogServer::Monitor()
{
    Utils::SetThreadName("NetWatchdogServer::MonitorThread");

    while (m_ShouldContinue.load())
    {
        m_ConnectionMonitor->CheckEvent(1s);
    };

    m_ConnectionMonitor.reset();
}

void NetWatchdogServer::HandleClientConnected(const std::string& identity)
{
    Mongo database(m_Options);
    if (!database.IsConnected())
    {
        return;
    }

    std::vector<std::string>::const_iterator iter = std::find_if(
        m_PrevConnectedClients.begin(),
        m_PrevConnectedClients.end(),
        [&identity](const std::string& item) { return item == identity; });

    if (iter == m_PrevConnectedClients.end())
    {
        ConnectionInfo connInfo;

        std::cout << "Connected: " << identity << std::endl;

        connInfo.m_UniqueId = identity;
        connInfo.m_Connection = ConnectionInfo::Type::Connection;
        database.AddConnectionInfo(connInfo);
    }
}

void NetWatchdogServer::HandleClientDisconnected(const zmq_event_t& zmqEvent, const char* addr)
{
    Mongo database(m_Options);
    if (!database.IsConnected())
    {
        return;
    }

    ConnectionInfo connInfo;

    {
        std::lock_guard<std::mutex> lock(m_ClientsLock);
        m_PrevConnectedClients = std::move(m_ConnectedClients);
        m_ConnectedClients = {};
    }

    HeartbeatMessage heartbeatMessage;
    for (const std::string& prevConnectedClient : m_PrevConnectedClients)
    {
        Communication::SendMessage(heartbeatMessage, m_ServerSocket, prevConnectedClient, zmq::send_flags::dontwait);
    }

    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    while (m_ConnectedClients.size() < (m_PrevConnectedClients.size() - 1) && (std::chrono::high_resolution_clock::now() - start) < 10s)
    {
        std::this_thread::sleep_for(1s);
    }

    std::unordered_set<std::string> currConnectedClients;
    {
        std::lock_guard<std::mutex> lock(m_ClientsLock);
        currConnectedClients = { m_ConnectedClients.begin(), m_ConnectedClients.end() };
    }

    m_PrevConnectedClients.erase(
        std::remove_if(
            m_PrevConnectedClients.begin(),
            m_PrevConnectedClients.end(),
            [&currConnectedClients](const std::string& item) { return currConnectedClients.find(item) != currConnectedClients.end(); }
        ),
        m_PrevConnectedClients.end()
    );

    for (const std::string& disconnectedClient : m_PrevConnectedClients)
    {
        connInfo.m_UniqueId = disconnectedClient;
        connInfo.m_Connection = ConnectionInfo::Type::Disconnection;
        database.AddConnectionInfo(connInfo);

        std::cout << "Disconnected: " << disconnectedClient << std::endl;
    }

    m_PrevConnectedClients.clear();
}