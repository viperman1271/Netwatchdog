#include "client.h"

#include "communication.h"
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

    GenericMessage msg;
    msg.SetId(m_Identity);
    msg.SetSuccess(true);
    Communication::SendMessage(msg, m_Socket);

    if(std::shared_ptr<GenericMessage> srvMsg = Communication::RecvMessage<GenericMessage, MessageType::GenericMessage>(m_Socket))
    {
        while (m_ShouldContinue)
        {
            if (std::shared_ptr<HeartbeatMessage> heartbeatMsg = Communication::RecvMessage<HeartbeatMessage, MessageType::Heartbeat>(m_Socket))
            {
                Communication::SendMessage(msg, m_Socket);
            }
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