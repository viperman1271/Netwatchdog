#include "utils.h"

#include <cstdlib> 
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Utils
{
    std::string GetEnvVar(const char* envVariable)
    {
#ifdef _WIN32
        constexpr int envVarStrLen = 256;
        char* const value = reinterpret_cast<char*>(alloca(envVarStrLen * sizeof(char)));
        memset(value, 0, envVarStrLen);

        GetEnvironmentVariable(envVariable, value, envVarStrLen);
        if (strlen(value) > 0)
        {
            return { value };
        }
#else
        const char* const value = std::getenv(envVariable);
        if (value != nullptr && strlen(value) > 0)
        {
            return { value };
        }
#endif

        return {};
    }

    std::filesystem::path& GetBasePath()
    {
#ifdef _WIN32
        std::string homeDir = GetEnvVar("USERPROFILE");
#else
        std::string homeDir = GetEnvVar("HOME");
#endif

        static std::filesystem::path path{ homeDir };
        static bool isInit = false;
        if (!isInit)
        {
#ifdef _WIN32
            path /= ".netwatchdog";
#else

#endif
            isInit = true;
        }

        if (!std::filesystem::exists(path))
        {
            std::filesystem::create_directory(path);
        }

        return path;
    }

    std::filesystem::path& GetConfigPath()
    {
        static bool isInit = false;
        static std::filesystem::path path;
        if (!isInit)
        {
            path = GetBasePath();
            path /= "config.toml";
            isInit = true;
        }

        return path;
    }

    void SetThreadName(std::thread& thread, const std::string& name)
    {
#ifdef _WIN32
        HANDLE handle = thread.native_handle();
        SetThreadDescription(handle, std::wstring(name.begin(), name.end()).c_str());
#endif
    }

    void SetThreadName(const std::string& name)
    {
#ifdef _WIN32
        SetThreadDescription(GetCurrentThread(), std::wstring(name.begin(), name.end()).c_str());
#endif
    }
}