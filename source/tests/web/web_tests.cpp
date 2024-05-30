#include "webserver.h"

#include "config.h"
#include "objectmodel.h"
#include "options.h"

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

TEST_CASE("Private/Public key + certificate generated as expected")
{
    Options options;
    Config::LoadOrCreateConfig(options);

    options.

    WebServer webServer(options);
    options = webServer.GetOptions();

    CHECK_FALSE(std::filesystem::exists(options.web.certificatePath));
    CHECK_FALSE(std::filesystem::exists(options.web.privateKeyPath));
    CHECK_FALSE(std::filesystem::exists(options.web.publicKeyPath));
        
    CHECK(webServer.GenerateKeyAndCertificate());

    CHECK(std::filesystem::exists(options.web.certificatePath));
    CHECK(std::filesystem::exists(options.web.privateKeyPath));
    CHECK(std::filesystem::exists(options.web.publicKeyPath));
}