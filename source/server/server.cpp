#include "server.h"

#include <iostream>
#include <sstream>

using namespace std::chrono_literals;

NetWatchdogServer::NetWatchdogServer(int port)
    : m_Port(port)
{

}

void NetWatchdogServer::Run()
{
    zmq::context_t context(1);
    {
        m_ServerSocket = zmq::socket_t(context, zmq::socket_type::router);

        const unsigned long long lingerMs = std::chrono::duration_cast<std::chrono::milliseconds>(1s).count();
        m_ServerSocket.set(zmq::sockopt::linger, static_cast<int>(lingerMs));

        zmq_socket_monitor(m_ServerSocket, "inproc://monitor", ZMQ_EVENT_CONNECTED | ZMQ_EVENT_DISCONNECTED);
        
        std::stringstream ss;
        ss << "tcp://*:" << m_Port;
        std::cout << "Binding router server to " << ss.str() << std::endl;

        m_ServerSocket.bind(ss.str());
    }

    {
		const int events = ZMQ_EVENT_CONNECTED | ZMQ_EVENT_DISCONNECTED;
		m_ConnectionMonitor.Init(m_ServerSocket, "inproc://conmon", events);
		m_ConnectionMonitor.SetCallback(ZMQ_EVENT_DISCONNECTED, [this](const zmq_event_t& zmqEvent, const char* addr) { HandleClientDisconnected(zmqEvent, addr); });
		m_MonitorThread = std::thread([this]()
		{
			Monitor();
		});
    }
}

void NetWatchdogServer::Monitor()
{
    //Utils::SetThreadName("Broker::MonitorThread");

    //while (m_ShouldContinue)
    {
        m_ConnectionMonitor.CheckEvent(1s);
    };
}

void NetWatchdogServer::HandleClientDisconnected(const zmq_event_t& zmqEvent, const char* addr)
{

}