#include "objectmodel.h"

#include <catch.hpp>

#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

TEST_CASE("Ensure that OPENSSL_Applink is available")
{
#ifdef _WIN32
    HMODULE h = GetModuleHandle(NULL);
    CHECK(h != NULL);

    void* applink = GetProcAddress(h, "OPENSSL_Applink");
    CHECK(applink != NULL);
#endif // _WIN32
}