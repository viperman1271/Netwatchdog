#include "utils.h"

#include <windows.h>

namespace Utils
{
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