#include "monitor.h"

#include <iostream>

void ConnectionMonitor::Init(zmq::socket_t& socket, const std::string& addr, int events)
{
    init(socket, addr, events);
}

void ConnectionMonitor::Init(zmq::socket_t& socket, const char* addr, int events)
{
    init(socket, addr, events);
}

bool ConnectionMonitor::CheckEvent(std::chrono::milliseconds timeout)
{
    return check_event(static_cast<int>(timeout.count()));
}

void ConnectionMonitor::SetCallback(int events, std::function<void(const zmq_event_t&, const char*)> callback)
{
    if (events & ZMQ_EVENT_CONNECTED)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_CONNECTED, callback);
    }

    if (events & ZMQ_EVENT_CONNECT_DELAYED)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_CONNECT_DELAYED, callback);
    }

    if (events & ZMQ_EVENT_CONNECT_RETRIED)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_CONNECT_RETRIED, callback);
    }

    if (events & ZMQ_EVENT_LISTENING)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_LISTENING, callback);
    }

    if (events & ZMQ_EVENT_BIND_FAILED)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_BIND_FAILED, callback);
    }

    if (events & ZMQ_EVENT_ACCEPTED)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_ACCEPTED, callback);
    }

    if (events & ZMQ_EVENT_ACCEPT_FAILED)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_ACCEPT_FAILED, callback);
    }

    if (events & ZMQ_EVENT_CLOSED)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_CLOSED, callback);
    }

    if (events & ZMQ_EVENT_CLOSE_FAILED)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_CLOSE_FAILED, callback);
    }

    if (events & ZMQ_EVENT_DISCONNECTED)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_DISCONNECTED, callback);
    }

    if (events & ZMQ_EVENT_MONITOR_STOPPED)
    {
        m_EventCallbacks.emplace_back(ZMQ_EVENT_MONITOR_STOPPED, callback);
    }
}

void ConnectionMonitor::on_event_connected(const zmq_event_t& event_, const char* addr)
{
    DoCallback(ZMQ_EVENT_CONNECTED, event_, addr);
}

void ConnectionMonitor::on_event_connect_delayed(const zmq_event_t& event_, const char* addr)
{
    DoCallback(ZMQ_EVENT_CONNECT_DELAYED, event_, addr);
}

void ConnectionMonitor::on_event_connect_retried(const zmq_event_t& event_, const char* addr)
{
    DoCallback(ZMQ_EVENT_CONNECT_RETRIED, event_, addr);
}

void ConnectionMonitor::on_event_listening(const zmq_event_t& event_, const char* addr)
{
    DoCallback(ZMQ_EVENT_LISTENING, event_, addr);
}

void ConnectionMonitor::on_event_bind_failed(const zmq_event_t& event_, const char* addr)
{
    DoCallback(ZMQ_EVENT_BIND_FAILED, event_, addr);
}

void ConnectionMonitor::on_event_accepted(const zmq_event_t& event_, const char* addr)
{
    DoCallback(ZMQ_EVENT_ACCEPTED, event_, addr);
}

void ConnectionMonitor::on_event_accept_failed(const zmq_event_t& event_, const char* addr)
{
    DoCallback(ZMQ_EVENT_ACCEPT_FAILED, event_, addr);
}

void ConnectionMonitor::on_event_closed(const zmq_event_t& event_, const char* addr)
{
    DoCallback(ZMQ_EVENT_CLOSED, event_, addr);
}

void ConnectionMonitor::on_event_close_failed(const zmq_event_t& event_, const char* addr)
{
    DoCallback(ZMQ_EVENT_CLOSE_FAILED, event_, addr);
}

void ConnectionMonitor::on_event_disconnected(const zmq_event_t& event_, const char* addr)
{
    DoCallback(ZMQ_EVENT_DISCONNECTED, event_, addr);
}

#if ZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 2, 3)
void ConnectionMonitor::on_event_handshake_failed_no_detail(const zmq_event_t& event_, const char* addr)
{
}

void ConnectionMonitor::on_event_handshake_failed_protocol(const zmq_event_t& event_, const char* addr)
{
}

void ConnectionMonitor::on_event_handshake_failed_auth(const zmq_event_t& event_, const char* addr)
{
}

void ConnectionMonitor::on_event_handshake_succeeded(const zmq_event_t& event_, const char* addr)
{
}

#elif ZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 2, 1)
void ConnectionMonitor::on_event_handshake_failed(const zmq_event_t& event_, const char* addr)
{
}

void ConnectionMonitor::on_event_handshake_succeed(const zmq_event_t& event_, const char* addr)
{
}

#endif
void ConnectionMonitor::on_event_unknown(const zmq_event_t& event_, const char* addr) 
{
}

void ConnectionMonitor::DoCallback(int eventId, const zmq_event_t& zmqEvent, const char* addr)
{
    auto it = std::find_if(m_EventCallbacks.begin(), m_EventCallbacks.end(), [eventId](const EventCallback& cb)
    {
        return cb.m_Event == eventId;
    });

    // Check if the value was found
    if (it != m_EventCallbacks.end())
    {
        (*it)(zmqEvent, addr);
    }
}