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
    , m_Context(1)
{
}

void NetWatchdogClient::Run(bool runThread)
{
    m_Socket = zmq::socket_t(m_Context, zmq::socket_type::dealer);

    const unsigned long long timeoutMs = std::chrono::duration_cast<std::chrono::milliseconds>(10s).count();
    m_Socket.set(zmq::sockopt::rcvtimeo, static_cast<int>(timeoutMs));

    const unsigned long long lingerMs = std::chrono::duration_cast<std::chrono::milliseconds>(1s).count();
    m_Socket.set(zmq::sockopt::linger, static_cast<int>(lingerMs));

    m_Socket.set(zmq::sockopt::identity, m_Identity);

    ConfigureCurve();

    std::stringstream ss;
    ss << "tcp://" << m_Host << ":" << m_Port;
    std::cout << "Connecting client to " << ss.str() << std::endl;

    m_Socket.connect(ss.str());

    std::function<void()> func = [this]()
    {
        GenericMessage msg;
        msg.SetId(m_Identity);
        msg.SetSuccess(true);
        Communication::SendMessage(msg, m_Socket);

        if (std::shared_ptr<GenericMessage> srvMsg = Communication::RecvMessage<GenericMessage, MessageType::GenericMessage>(m_Socket))
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
    };

    if (runThread)
    {
        m_Thread = std::thread([this, func]()
        {
            std::stringstream ss;
            ss << "NetWatchdogClient::" << m_Identity;
            Utils::SetThreadName(ss.str());

            func();
            m_Socket.close();
        });
    }
    else
    {
        func();
        m_Socket.close();
    }
}

void NetWatchdogClient::Kill()
{
    m_ShouldContinue.store(false);
}

void NetWatchdogClient::Wait()
{
    if (m_Thread.joinable())
    {
        m_Thread.join();
    }
}

bool NetWatchdogClient::ConfigureCurve()
{
    if (!zmq_has("curve"))
    {
        return false;
    }

    std::array<char, 41> public_key;
    std::array<char, 41> secret_key;
    zmq_curve_keypair(public_key.data(), secret_key.data());

    m_Socket.set(zmq::sockopt::curve_server, false);
    m_Socket.set(zmq::sockopt::curve_publickey, public_key.data());
    m_Socket.set(zmq::sockopt::curve_secretkey, secret_key.data());

    zmq::socket_t socket = zmq::socket_t(m_Context, zmq::socket_type::dealer);

    const unsigned long long timeoutMs = std::chrono::duration_cast<std::chrono::milliseconds>(10s).count();
    socket.set(zmq::sockopt::rcvtimeo, static_cast<int>(timeoutMs));

    const unsigned long long lingerMs = std::chrono::duration_cast<std::chrono::milliseconds>(1s).count();
    socket.set(zmq::sockopt::linger, static_cast<int>(lingerMs));

    socket.set(zmq::sockopt::identity, m_Identity);

    std::stringstream ss;
    ss << "tcp://" << m_Host << ":" << m_Port + 1;
    std::cout << "Connecting client to " << ss.str() << std::endl;

    socket.connect(ss.str());

    KeyRequestMessage msg;
    Communication::SendMessage(msg, socket);

    if (std::shared_ptr<KeyResponseMessage> heartbeatMsg = Communication::RecvMessage<KeyResponseMessage, MessageType::KeyResponse>(socket))
    {
        m_Socket.set(zmq::sockopt::curve_serverkey, heartbeatMsg->GetKey());
    }

    return true;
}
