#pragma once

#include "monitor.h"

#include <zmq.hpp>

#include <thread>

class NetWatchdogServer final
{
public:
    NetWatchdogServer(int port = 32000);
    ~NetWatchdogServer() = default;

    void Run();

    NetWatchdogServer(const NetWatchdogServer&) = delete;
    NetWatchdogServer(NetWatchdogServer&&) = delete;

private:
    void Monitor();
    void HandleClientDisconnected(const zmq_event_t& zmqEvent, const char* addr);

private:
    const int m_Port;

    zmq::socket_t m_ServerSocket;
    zmq::socket_t m_MonitorSocket;

    std::thread m_Thread;
    std::thread m_PendingWorkThread;
    std::thread m_MonitorThread;

    ConnectionMonitor m_ConnectionMonitor;
};