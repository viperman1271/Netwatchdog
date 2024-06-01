#pragma once

#include "monitor.h"

#include <zmq.hpp>

#include <thread>

class NetWatchdogServer final
{
    ConnectionMonitor m_ConnectionMonitor;
};

class NetWatchdogClient final
{
public:
    NetWatchdogClient(const std::string& host, const std::string& identity, int port = 32000);
    ~NetWatchdogClient() = default;

    void Run(bool runThread = false);
    void Kill();
    void Wait();

    NetWatchdogClient(const NetWatchdogServer&) = delete;
    NetWatchdogClient(NetWatchdogServer&&) = delete;

private:
    bool ConfigureCurve();

private:
    const int m_Port;
    const std::string m_Host;
    const std::string m_Identity;

    std::atomic<bool> m_ShouldContinue;

    zmq::context_t m_Context;
    zmq::socket_t m_Socket;
    std::thread m_Thread;
};