#include "objectmodel.h"

#include <catch.hpp>

#include <stdlib.h>
#include <windows.h>

TEST_CASE("Ensure that OPENSSL_Applink is available")
{
    HMODULE h = GetModuleHandle(NULL);
    CHECK(h != NULL);

    void* applink = GetProcAddress(h, "OPENSSL_Applink");
    CHECK(applink != NULL);
}