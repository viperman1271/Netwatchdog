#include "webserver.h"

#include <httplib.h>

WebServer::WebServer(const Options& options)
    : m_Options(options)
{
    std::filesystem::path certificatePath(*m_Options.web.certificatePath);
    if (!certificatePath.is_absolute())
    {
        std::filesystem::path basePath = Utils::GetBasePath();
        m_Options.web.certificatePath = basePath.append(*m_Options.web.certificatePath).string();
    }

    std::filesystem::path privateyKeyPath(*m_Options.web.privateKeyPath);
    if (!privateyKeyPath.is_absolute())
    {
        std::filesystem::path basePath = Utils::GetBasePath();
        m_Options.web.privateKeyPath = basePath.append(*m_Options.web.privateKeyPath).string();
    }

    std::filesystem::path publicKeyPath(*m_Options.web.publicKeyPath);
    if (!publicKeyPath.is_absolute())
    {
        std::filesystem::path basePath = Utils::GetBasePath();
        m_Options.web.publicKeyPath = basePath.append(*m_Options.web.publicKeyPath).string();
    }
}

bool WebServer::Run()
{
    {
        Mongo mongo(m_Options);
        if (!mongo.IsConnected())
        {
            std::cerr << "Could not connect to database server" << std::endl;
            return false;
        }
    }

    std::unique_ptr<httplib::Server> svr;
    if (m_Options.web.secure)
    {
        if (!std::filesystem::exists(*m_Options.web.certificatePath) || !std::filesystem::exists(*m_Options.web.privateKeyPath) || !std::filesystem::exists(*m_Options.web.publicKeyPath))
        {
            GenerateKeyAndCertificate();
        }

        svr.reset(new httplib::SSLServer(m_Options.web.certificatePath->c_str(), m_Options.web.privateKeyPath->c_str()));
    }
    else
    {
        svr.reset(new httplib::Server);
    }

    std::filesystem::path fileServingDir(*m_Options.web.fileServingDir);
    std::function<std::string(const std::string&)> func = [&](const std::string& str)
    {
        std::filesystem::path fileServingDir(*m_Options.web.fileServingDir);
        fileServingDir /= str;
        return fileServingDir.string();
    };

    const std::string imagesPath = func("images");
    std::cout << "Serving /images from " << imagesPath << std::endl;
    svr->set_mount_point("/images", imagesPath);

    const std::string scriptsPath = func("scripts");
    std::cout << "Serving /scripts from " << scriptsPath << std::endl;
    svr->set_mount_point("/scripts", func("scripts"));

    const std::string stylesPath = func("styles");
    std::cout << "Serving /styles from " << stylesPath << std::endl;
    svr->set_mount_point("/styles", func("styles"));

    svr->Get("/", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::filesystem::path filePath(*m_Options.web.fileServingDir);
        filePath /= "index.html";

        std::string content;
        ReadFile(filePath, res, content);
        res.set_content(content, "text/html");
    });

    std::function<std::filesystem::path(const std::string&)> templateFunc = [&](const std::string& templateName)
    {
        std::filesystem::path fileServingDir(*m_Options.web.fileServingDir);
        fileServingDir /= "templates";
        fileServingDir /= templateName;
        return fileServingDir;
    };

    WebTemplate tbody(templateFunc("tbody.template"));
    WebTemplate thead(templateFunc("thead.template"));
    WebTemplate thead_tr(templateFunc("thead-tr.template"));
    WebTemplate thead_tr_th(templateFunc("thead-tr-th.template"));
    WebTemplate tbody_tr(templateFunc("tbody-tr.template"));
    WebTemplate tbody_tr_td(templateFunc("tbody-tr-td.template"));

    svr->Get("/dashboard.html", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::filesystem::path filePath(*m_Options.web.fileServingDir);
        filePath /= "dashboard.html";

        std::string content;
        ReadFile(filePath, res, content);

        if (req.params.contains("logs"))
        {
            Utils::ReplaceStrInString(content, "${{TABLE_HEADER}}", "Connection Logs");

            std::string clientId{};
            if (req.params.contains("clientId"))
            {
                clientId = req.params.equal_range("clientId").first->second;
            }

            std::vector<ConnectionInfo> connInfos;
            Mongo mongo(m_Options);
            mongo.FetchClientInfo(clientId, connInfos);

            const std::string baseIndent = "                                    ";
            std::stringstream ss;
            {
                WebTemplate::AutoScope tbodyScope(ss, tbody);
                {
                    WebTemplate::AutoScope theadScope(ss, thead);
                    {
                        WebTemplate::AutoScope thead_tr_Scope(ss, thead_tr);
                        {
                            thead_tr_th.Write(ss, { {"${{col_name}}", "conn"}, {"${{col_text}}", "Log Type"} });
                            thead_tr_th.Write(ss, { {"${{col_name}}", "client"}, {"${{col_text}}", "Client ID"} });
                            thead_tr_th.Write(ss, { {"${{col_name}}", "time"}, {"${{col_text}}", "Time"} });
                        }
                    }
                }
                for (const ConnectionInfo& connInfo : connInfos)
                {
                    WebTemplate::AutoScope tbody_tr_scope(ss, tbody_tr);

                    char fullLink[256];
                    sprintf(fullLink, "<a href=\"dashboard.html?logs&clientId=%s\">%s</a>", connInfo.m_UniqueId.c_str(), connInfo.m_UniqueId.c_str());

                    const std::string time = ConvertHighResRepToString(connInfo.m_Time);

                    tbody_tr_td.Write(ss, { {"${{row_text}}", (connInfo.m_Connection == ConnectionInfo::Type::Connection ? "Connection" : "Disconnection")} });
                    tbody_tr_td.Write(ss, { {"${{row_text}}", fullLink} });
                    tbody_tr_td.Write(ss, { {"${{row_text}}", time} });
                }
            }
            Utils::ReplaceStrInString(content, "${{TABLE_CONTENTS}}", ss.str());
        }
        else if (req.params.contains("admin"))
        {
            Utils::ReplaceStrInString(content, "${{TABLE_HEADER}}", "Admin Options");
            Utils::ReplaceStrInString(content, "${{TABLE_CONTENTS}}", "                            <div id=\"protected-content\"></div>");
        }
        else
        {
            Utils::ReplaceStrInString(content, "${{TABLE_HEADER}}", "");
            Utils::ReplaceStrInString(content, "${{TABLE_CONTENTS}}", "");
        }

        res.set_content(content, "text/html");
    });

    svr->Get("/api/protected", [&](const httplib::Request& req, httplib::Response& res)
    {
        Mongo mongo(m_Options);
        ValidateToken(mongo, m_Options, req, res);
    });

    svr->Post("/api/login", [&](const httplib::Request& req, httplib::Response& res) 
    {
        auto body = nlohmann::json::parse(req.body);
        std::string username = body["username"];
        std::string password = body["password"];

        Mongo mongo(m_Options);

        User user;
        if(mongo.FetchUser(username, user) && user.ValidatePassword(password))
        {
            const std::chrono::system_clock::time_point timepoint = std::chrono::system_clock::now() + std::chrono::days(30);

            const std::string token = jwt::create().set_issuer("auth_server").set_type("JWS").set_payload_claim("username", jwt::claim(username)).set_payload_claim("expiry", jwt::claim(timepoint)).sign(jwt::algorithm::hs256{m_Options.server.identity});

            nlohmann::json response = { {"token", token} };
            res.set_content(response.dump(), "application/json");
        }
        else
        {
            res.status = 401;
            res.set_content("Invalid credentials", "text/plain");
        }
    });

    svr->Get("/api/admin", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::string username;
        if (ExtractUsernameFromToken(m_Options, req, username) == TokenResult::Correct)
        {
            Mongo mongo(m_Options);
            User user;
            if (mongo.FetchUser(username, user))
            {
                if (user.m_IsAdmin)
                {
                    res.status = 200;
                    res.set_content("Admin access granted", "text/plain");
                }
                else
                {
                    res.status = 401;
                    res.set_content("Admin access unauthorized", "text/plain");
                }
            }
        }
    });

    svr->Post("/api/client-info/clear", [&](const httplib::Request& req, httplib::Response& res) 
    {
        auto body = nlohmann::json::parse(req.body);
        std::string clientId = body["clientId"];

        Mongo mongo(m_Options);
        mongo.DeleteInfo(Mongo::Database::Stats, Mongo::Collection::Connection, clientId);
    });

    svr->Get(R"(/api/(.*))", [&](const httplib::Request& req, httplib::Response& res)
    {
        res.status = 401;
        res.set_content("Invalid token", "text/plain");
    });

    svr->Get(R"(/(.*))", [&](const httplib::Request& req, httplib::Response& res)
    {
        const std::string file = req.matches[1];

        std::filesystem::path filePath(*m_Options.web.fileServingDir);
        filePath /= file;

        std::string content;
        ReadFile(filePath, res, content);
        res.set_content(content, "text/html");
    });

    svr->listen(m_Options.web.host, m_Options.web.secure ? m_Options.web.securePort : m_Options.web.port);

    return true;
}

bool WebServer::ReadFile(std::filesystem::path& filePath, httplib::Response& res, std::string& content)
{
    if (std::filesystem::exists(filePath))
    {
        std::ifstream file;
        file.open(filePath.string(), std::ios_base::in);
        if (file.is_open())
        {
            size_t fileSize2 = file.tellg();
            file.seekg(0, std::ios_base::end);
            const size_t fileSize = file.tellg();
            file.seekg(0, std::ios_base::beg);

            char* fileContents = reinterpret_cast<char*>(alloca(fileSize + 1));
            memset(fileContents, 0, fileSize + 1);
            file.read(fileContents, fileSize);

            content = fileContents;

            return true;
        }
        else
        {
            res.status = 404;
        }
    }
    else
    {
        res.status = 404;
    }

    return false;
}

bool WebServer::DecodeToken(const Options& m_Options, const httplib::Request& req, std::string& token)
{
    const std::string auth_header = req.get_header_value("Authorization");

    if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ")
    {
        return false;
    }

    token = auth_header.substr(7); // Remove "Bearer " prefix

    return true;
}

bool WebServer::ExtractToken(const Options& m_Options, const httplib::Request& req, jwt::decoded_jwt<jwt::traits::kazuho_picojson>& jwtToken)
{
    std::string token;
    if (!DecodeToken(m_Options, req, token))
    {
        return false;
    }

    jwtToken = jwt::decode(token);
    jwt::verifier verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ m_Options.server.identity }).with_issuer("auth_server");

    std::error_code error;
    verifier.verify(jwtToken, error);

    return !error;
}

WebServer::TokenResult WebServer::ExtractUsernameFromToken(const Options& m_Options, const httplib::Request& req, std::string& out_username)
{
    std::string token;
    if (!DecodeToken(m_Options, req, token))
    {
        return TokenResult::Empty;
    }

    jwt::decoded_jwt<jwt::traits::kazuho_picojson> decoded = jwt::decode(token);
    if (ExtractToken(m_Options, req, decoded))
    {
        if (!decoded.has_payload_claim("username"))
        {
            return TokenResult::Invalid;
        }
        jwt::claim usernameClaim = decoded.get_payload_claim("username");

        out_username = usernameClaim.as_string();

        return TokenResult::Correct;
    }

    return TokenResult::Invalid;
}

WebServer::TokenResult WebServer::ValidateToken(Mongo& mongo, const Options& m_Options, const httplib::Request& req)
{
    std::string token;
    if (!DecodeToken(m_Options, req, token))
    {
        return TokenResult::Empty;
    }

    jwt::decoded_jwt<jwt::traits::kazuho_picojson> decoded = jwt::decode(token);
    if (ExtractToken(m_Options, req, decoded))
    {
        if (!decoded.has_payload_claim("username"))
        {
            return TokenResult::Invalid;
        }
        jwt::claim usernameClaim = decoded.get_payload_claim("username");

        User user;
        if (!mongo.FetchUser(usernameClaim.as_string(), user))
        {
            return TokenResult::Invalid;
        }

        if (!decoded.has_payload_claim("expiry"))
        {
            return TokenResult::Invalid;
        }

        jwt::claim expiryClaim = decoded.get_payload_claim("expiry");
        if (expiryClaim.as_date() < std::chrono::system_clock::now())
        {
            return TokenResult::Invalid;
        }

        return TokenResult::Correct;
    }

    return TokenResult::Invalid;
}

bool WebServer::ValidateToken(Mongo& mongo, const Options& m_Options, const httplib::Request& req, httplib::Response& res)
{
    switch (ValidateToken(mongo, m_Options, req))
    {
    case TokenResult::Correct:
        res.status = 200;
        res.set_content("Access granted to protected resource", "text/plain");
        return true;

    case TokenResult::Empty:
        res.status = 401;
        res.set_content("Authorization required", "text/plain");
        break;

    case TokenResult::Invalid:
        res.status = 403;
        res.set_content("Invalid token", "text/plain");
        break;
    };

    return false;
}

std::string WebServer::ConvertHighResRepToString(std::chrono::system_clock::duration::rep rep)
{
    const std::chrono::system_clock::duration system_duration(rep);

    const std::chrono::system_clock::time_point system_time_point(system_duration);

    const std::time_t time_t_epoch = std::chrono::system_clock::to_time_t(system_time_point);

    const std::tm* const local_tm = std::localtime(&time_t_epoch);

    // Step 6: Format tm into a string
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%F %T", local_tm);

    return std::string(buffer);
}

bool WebServer::GenerateKeyAndCertificate() const
{
    std::cout << "Generate public/private key & certificate" << std::endl;
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL), EVP_PKEY_CTX_free);
    if (!ctx) 
    {
        return false;
    }

    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) 
    {
        return false;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), 2048) <= 0) 
    {
        return false;
    }

    EVP_PKEY* pkeytemp = NULL;
    if (EVP_PKEY_keygen(ctx.get(), &pkeytemp) <= 0)
    {
        return false;
    }
    
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> pkey(pkeytemp, EVP_PKEY_free);

    if (!WritePublicKey(pkey.get()) || !WritePrivateKey(pkey.get()))
    {
        return false;
    }

    std::cout << "Saved public key to : " << m_Options.web.publicKeyPath << std::endl;
    std::cout << "Saved private key to : " << m_Options.web.privateKeyPath << std::endl;

    // Generate the X509 certificate
    std::unique_ptr<X509, decltype(&X509_free)> x509(X509_new(), X509_free);
    ASN1_INTEGER_set(X509_get_serialNumber(x509.get()), 1);
    X509_gmtime_adj(X509_get_notBefore(x509.get()), 0);
    X509_gmtime_adj(X509_get_notAfter(x509.get()), 31536000L); // Valid for 1 year
    X509_set_pubkey(x509.get(), pkey.get());

    // Set certificate subject and issuer name
    X509_NAME* name = X509_get_subject_name(x509.get());
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*)"US", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char*)"My Organization", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"My Common Name", -1, -1, 0);
    X509_set_issuer_name(x509.get(), name);

    // Sign the certificate with the private key
    X509_sign(x509.get(), pkey.get(), EVP_sha256());

    return WriteCertificate(x509.get());
}

bool WebServer::WritePublicKey(EVP_PKEY* const pkey) const
{
    std::unique_ptr<BIO, decltype(&BIO_free_all)> bp_public(BIO_new_file(m_Options.web.publicKeyPath->c_str(), "w+"), BIO_free_all);
    if (!bp_public)
    {
        return false;
    }

    if (PEM_write_bio_PUBKEY(bp_public.get(), pkey) <= 0)
    {
        return false;
    }

    return true;
}

bool WebServer::WritePrivateKey(EVP_PKEY* const pkey) const
{
    // 5. Save the private key to a file
    std::unique_ptr<BIO, decltype(&BIO_free_all)> bp_private(BIO_new_file(m_Options.web.privateKeyPath->c_str(), "w+"), BIO_free_all);
    if (!bp_private)
    {
        return false;
    }

    if (PEM_write_bio_PrivateKey(bp_private.get(), pkey, NULL, NULL, 0, NULL, NULL) <= 0)
    {
        return false;
    }

    return true;
}

bool WebServer::WriteCertificate(X509* const x509) const
{
    // Write the certificate to a file
    FILE* x509_file = fopen(m_Options.web.certificatePath->c_str(), "wb");
    PEM_write_X509(x509_file, x509);
    fclose(x509_file);

    std::cout << "Saved certificate file to : " << m_Options.web.certificatePath << std::endl;

    return true;
}
