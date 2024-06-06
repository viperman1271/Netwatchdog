#include "webserver.h"

#include "config.h"
#include "objectmodel.h"
#include "options.h"

#include <catch.hpp>

#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

void DeleteIfExists(const std::filesystem::path& path)
{
    if (std::filesystem::exists(path))
    {
        std::filesystem::remove(path);
    }
}

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

    std::filesystem::path certificatePath = std::filesystem::current_path();
    certificatePath /= "certificate.pem";

    std::filesystem::path publicKeyPath = std::filesystem::current_path();
    publicKeyPath /= "public_key.pem";

    std::filesystem::path privateKeyPath = std::filesystem::current_path();
    privateKeyPath /= "private_key.pem";

    options.web.certificatePath = certificatePath.string();
    options.web.publicKeyPath = publicKeyPath.string();
    options.web.privateKeyPath = privateKeyPath.string();

    DeleteIfExists(certificatePath);
    DeleteIfExists(publicKeyPath);
    DeleteIfExists(privateKeyPath);

    WebServer webServer(options);
    options = webServer.GetOptions();

    CHECK_FALSE(std::filesystem::exists(*options.web.certificatePath));
    CHECK_FALSE(std::filesystem::exists(*options.web.privateKeyPath));
    CHECK_FALSE(std::filesystem::exists(*options.web.publicKeyPath));
        
    CHECK(webServer.GenerateKeyAndCertificate());

    CHECK(std::filesystem::exists(*options.web.certificatePath));
    CHECK(std::filesystem::exists(*options.web.privateKeyPath));
    CHECK(std::filesystem::exists(*options.web.publicKeyPath));
}