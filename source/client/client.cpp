#include "client.h"

#include "utils.h"

#include <stduuid/uuid.h>

#include <chrono>
#include <filesystem>
#include <iostream>

using namespace std::chrono_literals;

NetWatchdogClient::NetWatchdogClient(const std::string& host, int port /*= 32000*/)
    : m_Port(port)
    , m_Host(host)
    , m_Identity(uuids::to_string(uuids::uuid_random_generator(g_RNG)()))
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

    while (true)
    {
        std::this_thread::sleep_for(1s);
    };
}