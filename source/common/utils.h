#pragma once

#include <chrono>
#include <random>
#include <thread>

namespace Utils
{
    void SetThreadName(std::thread& thread, const std::string& name);
    void SetThreadName(const std::string& name);
}

static std::mt19937 g_RNG(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));

namespace zmq::sockopt
{
#ifdef ZMQ_IDENTITY
ZMQ_DEFINE_ARRAY_OPT_BINARY(ZMQ_IDENTITY, identity);
#endif
}