#include "utils.h"

#include "tests.h"

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

    const std::filesystem::path& GetBasePath()
    {
        if (Tests::IsTestMode())
        {
            static std::filesystem::path path = std::filesystem::current_path();
            return path;
        }

#ifdef _WIN32
        std::string homeDir = GetEnvVar("USERPROFILE");
#else
        std::string homeDir = "/etc/netwatchdog";
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

    const std::filesystem::path& GetConfigPath()
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

    const std::filesystem::path& GetWebFileServingPath()
    {
        static bool isInit = false;
        static std::filesystem::path path;
        if (!isInit)
        {
#ifdef _WIN32
            path = GetBasePath();
#else
            path = "/var/netwatchdog/";
#endif
            path /= "www/";
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

    void ReplaceStrInString(std::string& baseString, const std::string& strToFind, const std::string& strToReplaceFoundStr)
    {
        size_t pos = 0;
        while ((pos = baseString.find(strToFind, pos)) != std::string::npos)
        {
            baseString.replace(pos, strToFind.length(), strToReplaceFoundStr);
            pos += strToReplaceFoundStr.length();
        }
    }
}