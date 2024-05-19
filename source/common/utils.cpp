#include "utils.h"

#include <cstdlib> 
#ifdef _WIN32
#include <windows.h>
#endif

namespace Utils
{
    std::filesystem::path& GetBasePath()
    {
#ifdef _WIN32
        const char* homeDir = getenv("USERPROFILE");
#else
        const char* homeDir = getenv("HOME");
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
        HANDLE handle = thread.native_handle();
        SetThreadDescription(handle, std::wstring(name.begin(), name.end()).c_str());
    }

    void SetThreadName(const std::string& name)
    {
        SetThreadDescription(GetCurrentThread(), std::wstring(name.begin(), name.end()).c_str());
    }
}