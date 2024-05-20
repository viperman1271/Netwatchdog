#pragma once

#include "mongo.h"
#include "monitor.h"

#include <zmq.hpp>

#include <mutex>
#include <thread>

class NetWatchdogServer final
{
public:
    NetWatchdogServer(const Options& options);
    ~NetWatchdogServer() = default;

    void Run();
    void Kill();

    NetWatchdogServer(const NetWatchdogServer&) = delete;
    NetWatchdogServer(NetWatchdogServer&&) = delete;

private:
    void Monitor();
    void HandleClientConnected(const std::string& identity);
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

    std::mutex m_ClientsLock;
    std::vector<std::string> m_ConnectedClients;
    std::vector<std::string> m_PrevConnectedClients;
    std::unique_ptr<ConnectionMonitor> m_ConnectionMonitor;

    Mongo m_Database;
};