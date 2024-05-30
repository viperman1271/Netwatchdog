#include "config.h"
#include "mongo.h"
#include "webtemplate.h"

#include <jwt-cpp/jwt.h>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>

class WebServer
{
public:
    WebServer(const Options& options);

    bool Run();

public:
    enum class TokenResult
    {
        Correct,
        Empty,
        Invalid,
    };

    static bool ReadFile(std::filesystem::path& filePath, httplib::Response& res, std::string& content);
    static bool DecodeToken(const Options& options, const httplib::Request& req, std::string& token);
    static bool ExtractToken(const Options& options, const httplib::Request& req, jwt::decoded_jwt<jwt::traits::kazuho_picojson>& jwtToken);
    static TokenResult ValidateToken(Mongo& mongo, const Options& options, const httplib::Request& req);
    static bool ValidateToken(Mongo& mongo, const Options& options, const httplib::Request& req, httplib::Response& res);
    static std::string ConvertHighResRepToString(std::chrono::system_clock::duration::rep rep);

    bool GenerateKeyAndCertificate() const;

    const Options& GetOptions() const { return m_Options; }

private:
    bool WritePublicKey(EVP_PKEY* pkey) const;
    bool WritePrivateKey(EVP_PKEY* const pkey) const;
    bool WriteCertificate(X509* const x509) const;

private:
    Options m_Options;
};