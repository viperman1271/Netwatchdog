#include "tests.h"

namespace Tests
{
    static bool g_IsTestMode = false;

    bool IsTestMode()
    {
        return g_IsTestMode;
    }

    void SetIsTestMode(bool value)
    {
        g_IsTestMode = value;
    }
}
    