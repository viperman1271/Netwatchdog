#pragma once

#include "monitor.h"

#include <zmq.hpp>

#include <thread>

class NetWatchdogServer final
{
public:
    NetWatchdogServer(const std::string& listenAddress, const std::string& identity, int port = 32000);
    ~NetWatchdogServer() = default;

    void Run();

    NetWatchdogServer(const NetWatchdogServer&) = delete;
    NetWatchdogServer(NetWatchdogServer&&) = delete;

private:
    void Monitor();
    void HandleClientConnected(const zmq_event_t& zmqEvent, const char* addr);
    void HandleClientConnected();
    void HandleClientDisconnected(const zmq_event_t& zmqEvent, const char* addr);

private:
    const int m_Port;
    const std::string m_ListenAddress;
    const std::string m_Identity;

    std::atomic<bool> m_ShouldContinue;

    zmq::socket_t m_ServerSocket;
    zmq::socket_t m_MonitorSocket;

    std::thread m_Thread;
    std::thread m_PendingWorkThread;
    std::thread m_MonitorThread;

    ConnectionMonitor m_ConnectionMonitor;
};