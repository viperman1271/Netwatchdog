#pragma once

#include <zmq.hpp>

#include <functional>

class ConnectionMonitor final : private zmq::monitor_t
{
    using super = zmq::monitor_t;

public:
    void Init(zmq::socket_t& socket, const std::string& addr, int events = ZMQ_EVENT_ALL);
    void Init(zmq::socket_t& socket, const char* addr, int events = ZMQ_EVENT_ALL);

    bool CheckEvent(std::chrono::milliseconds timeout = std::chrono::milliseconds{ 0 });

    void SetCallback(int events, std::function<void(const zmq_event_t&, const char*)> callback);

protected:
    void on_event_connected(const zmq_event_t& event_, const char* addr) override;
    void on_event_connect_delayed(const zmq_event_t& event_,const char* addr) override;
    void on_event_connect_retried(const zmq_event_t& event_,const char* addr) override;
    void on_event_listening(const zmq_event_t& event_, const char* addr) override;
    void on_event_bind_failed(const zmq_event_t& event_, const char* addr) override;
    void on_event_accepted(const zmq_event_t& event_, const char* addr) override;
    void on_event_accept_failed(const zmq_event_t& event_, const char* addr) override;
    void on_event_closed(const zmq_event_t& event_, const char* addr) override;
    void on_event_close_failed(const zmq_event_t& event_, const char* addr) override;
    void on_event_disconnected(const zmq_event_t& event_, const char* addr) override;
#if ZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 2, 3)
    void on_event_handshake_failed_no_detail(const zmq_event_t& event_, const char* addr) override;
    void on_event_handshake_failed_protocol(const zmq_event_t& event_, const char* addr) override;
    void on_event_handshake_failed_auth(const zmq_event_t& event_, const char* addr) override;
    void on_event_handshake_succeeded(const zmq_event_t& event_, const char* addr) override;
#elif ZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 2, 1)
    void on_event_handshake_failed(const zmq_event_t& event_, const char* addr) override;
    void on_event_handshake_succeed(const zmq_event_t& event_, const char* addr) override;
#endif
    void on_event_unknown(const zmq_event_t& event_, const char* addr) override;

    void DoCallback(int eventId, const zmq_event_t& zmqEvent, const char* addr);
private:
    struct EventCallback
    {
        EventCallback() = default;
        EventCallback(int event, std::function<void(const zmq_event_t&, const char*)> callback)
            : m_Event(event)
            , m_Callback(callback)
        {
        }

        void operator()(const zmq_event_t& zmqEvent, const char* addr)
        {
            m_Callback(zmqEvent, addr);
        }

        int m_Event;
        std::function<void(const zmq_event_t&, const char*)> m_Callback;
    };

    std::vector<EventCallback> m_EventCallbacks;
};