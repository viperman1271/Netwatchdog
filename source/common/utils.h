#pragma once

#include <chrono>
#include <filesystem>
#include <random>
#include <thread>

namespace Utils
{
    std::string GetEnvVar(const char* envVariable);
    const std::filesystem::path& GetBasePath();
    const std::filesystem::path& GetConfigPath();
    const std::filesystem::path& GetWebFileServingPath();

    void SetThreadName(std::thread& thread, const std::string& name);
    void SetThreadName(const std::string& name);

    void ReplaceStrInString(std::string& baseString, const std::string& strToFind, const std::string& strToReplaceFoundStr);
}

static std::mt19937 g_RNG(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));

namespace zmq::sockopt
{
#ifdef ZMQ_IDENTITY
ZMQ_DEFINE_ARRAY_OPT_BINARY(ZMQ_IDENTITY, identity);
#endif
}