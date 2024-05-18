#include "client.h"

#include "utils.h"

#include <chrono>
#include <filesystem>
#include <iostream>

using namespace std::chrono_literals;

NetWatchdogClient::NetWatchdogClient(const std::string& host, const std::string& identity, int port /*= 32000*/)
    : m_Port(port)
    , m_Host(host)
    , m_Identity(identity)
    , m_ShouldContinue(true)
{
}

void NetWatchdogClient::Run()
{
    zmq::context_t context{ 1 };
    m_Socket = zmq::socket_t(context, zmq::socket_type::pair);

    const unsigned long long timeoutMs = std::chrono::duration_cast<std::chrono::milliseconds>(10s).count();
    m_Socket.set(zmq::sockopt::rcvtimeo, static_cast<int>(timeoutMs));

    const unsigned long long lingerMs = std::chrono::duration_cast<std::chrono::milliseconds>(1s).count();
    m_Socket.set(zmq::sockopt::linger, static_cast<int>(lingerMs));

    m_Socket.set(zmq::sockopt::identity, m_Identity);

    std::stringstream ss;
    ss << "tcp://" << m_Host << ":" << m_Port;
    std::cout << "Connecting client to " << ss.str() << std::endl;

    m_Socket.connect(ss.str());
    m_Socket.send(zmq::buffer(m_Identity), zmq::send_flags::none);

    zmq::message_t message;
    if (m_Socket.recv(message))
    {
        while (m_ShouldContinue)
        {
            std::this_thread::sleep_for(1s);
        };
    }
    else
    {
        std::cerr << "Could not connect to server: " << m_Host << ":" << m_Port << std::endl;
    }
    
    m_Socket.close();
}

void NetWatchdogClient::Kill()
{
    m_ShouldContinue.store(false);
}