#include "server.h"

#include "communication.h"
#include "utils.h"

#include <iostream>
#include <sstream>

using namespace std::chrono_literals;

NetWatchdogServer::NetWatchdogServer(const std::string& listenAddress, const std::string& identity, int port)
    : m_Port(port)
    , m_ListenAddress(listenAddress)
    , m_Identity(identity)
    , m_ShouldContinue(true)
{

}

void NetWatchdogServer::Run()
{
    zmq::context_t context(1);
    {
        m_ServerSocket = zmq::socket_t(context, zmq::socket_type::pair);

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
        std::shared_ptr<GenericMessage> msg = Communication::RecvMessage<GenericMessage, MessageType::GenericMessage>(m_ServerSocket);
        if (msg != nullptr)
        {
            HandleClientConnected(msg->GetId());
            m_ConnectedClients.push_back(msg->GetId());

            GenericMessage respMessage;
            respMessage.SetId(m_Identity);
            respMessage.SetSuccess(true);
            Communication::SendMessage(respMessage, m_ServerSocket);
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
    std::cout << "Connected: " << identity << std::endl;
}

void NetWatchdogServer::HandleClientDisconnected(const zmq_event_t& zmqEvent, const char* addr)
{
    HeartbeatMessage heartbeatMessage;
    Communication::SendMessage(heartbeatMessage, m_ServerSocket, zmq::send_flags::dontwait);

    std::vector<std::string> prevConnectedClients = std::move(m_ConnectedClients);
    m_ConnectedClients = {};

    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    while (m_ConnectedClients.size() < (prevConnectedClients.size() - 1) && (std::chrono::high_resolution_clock::now() - start) < 10s)
    {
        std::this_thread::sleep_for(1s);
    }

    std::unordered_set<std::string> currConnectedClients(m_ConnectedClients.begin(), m_ConnectedClients.end());

    prevConnectedClients.erase(
        std::remove_if(
            prevConnectedClients.begin(),
            prevConnectedClients.end(),
            [&currConnectedClients](const std::string& item) { return currConnectedClients.find(item) != currConnectedClients.end(); }
        ),
        prevConnectedClients.end()
    );

    for (const std::string& disconnectedClient : prevConnectedClients)
    {
        std::cout << "Disconnected" << disconnectedClient << std::endl;
    }
}