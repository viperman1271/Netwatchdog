#include "server.h"

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
		m_ConnectionMonitor.Init(m_ServerSocket, "inproc://conmon", events);
        m_ConnectionMonitor.SetCallback(ZMQ_EVENT_CONNECTED, [this](const zmq_event_t& zmqEvent, const char* addr) { HandleClientConnected(zmqEvent, addr); });
		m_ConnectionMonitor.SetCallback(ZMQ_EVENT_DISCONNECTED, [this](const zmq_event_t& zmqEvent, const char* addr) { HandleClientDisconnected(zmqEvent, addr); });
		m_MonitorThread = std::thread([this]()
		{
			Monitor();
		});
    }

    while (m_ShouldContinue.load())
    {
        zmq::message_t message;
        if (m_ServerSocket.recv(message))
        {
            HandleClientConnected();
            std::string str(static_cast<char*>(message.data()), message.size());
            std::cout << "Recv: " << str << std::endl;
        }
    };
}

void NetWatchdogServer::Monitor()
{
    Utils::SetThreadName("Broker::MonitorThread");

    while (m_ShouldContinue)
    {
        m_ConnectionMonitor.CheckEvent(1s);
    };
}

void NetWatchdogServer::HandleClientConnected(const zmq_event_t& zmqEvent, const char* addr)
{
    HandleClientConnected();
}

void NetWatchdogServer::HandleClientConnected()
{
    std::cout << "Connected" << std::endl;
}

void NetWatchdogServer::HandleClientDisconnected(const zmq_event_t& zmqEvent, const char* addr)
{
    std::cout << "Disconnected" << std::endl;
}