#include "config.h"
#include "mongo.h"
#include "web-template.h"

#include <jwt-cpp/jwt.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <filesystem>

bool readFile(std::filesystem::path& filePath, httplib::Response& res, std::string& content)
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

bool decodeToken(const Options& options, const httplib::Request& req, std::string& token)
{
    const std::string auth_header = req.get_header_value("Authorization");

    if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ")
    {
        return false;
    }

    token = auth_header.substr(7); // Remove "Bearer " prefix

    return true;
}

bool extractToken(const Options& options, const httplib::Request& req, jwt::decoded_jwt<jwt::traits::kazuho_picojson>& jwtToken)
{
    std::string token;
    if (!decodeToken(options, req, token))
    {
        return false;
    }

    jwtToken = jwt::decode(token);
    jwt::verifier verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ options.server.identity }).with_issuer("auth_server");

    std::error_code error;
    verifier.verify(jwtToken, error);

    return !error;
}

enum class TokenResult
{
    Correct,
    Empty,
    Invalid,
};

TokenResult validateToken(Mongo& mongo, const Options& options, const httplib::Request& req)
{
    std::string token;
    if (!decodeToken(options, req, token))
    {
        return TokenResult::Empty;
    }

    jwt::decoded_jwt<jwt::traits::kazuho_picojson> decoded = jwt::decode(token);
    if (extractToken(options, req, decoded))
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

bool validateToken(Mongo& mongo, const Options& options, const httplib::Request& req, httplib::Response& res)
{
    switch (validateToken(mongo, options, req))
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

std::string convertHighResRepToString(std::chrono::system_clock::duration::rep rep)
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

int main(int argc, char** argv)
{
    Options options;
    Config::LoadOrCreateConfig(options);
    if (!Config::ParseCommandLineOptions(argc, argv, options, Config::ParsingType::Server))
    {
        return -1;
    }

    {
        Mongo mongo(options);
        if (!mongo.IsConnected())
        {
            return -1;
        }
    }

    httplib::Server svr;

    std::filesystem::path fileServingDir = options.web.fileServingDir;
    std::function<std::string(const std::string&)> func = [&options](const std::string& str)
    {
        std::filesystem::path fileServingDir = options.web.fileServingDir;
        fileServingDir /= str;
        return fileServingDir.string();
    };

    svr.set_mount_point("/images", func("images"));
    svr.set_mount_point("/scripts", func("scripts"));
    svr.set_mount_point("/styles", func("styles"));

    svr.Get("/", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::filesystem::path filePath = options.web.fileServingDir;
        filePath /= "index.html";

        std::string content;
        readFile(filePath, res, content);
        res.set_content(content, "text/html");
    });

    std::function<std::filesystem::path(const std::string&)> templateFunc = [&options](const std::string& templateName)
    {
        std::filesystem::path fileServingDir = options.web.fileServingDir;
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

    svr.Get("/dashboard.html", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::filesystem::path filePath = options.web.fileServingDir;
        filePath /= "dashboard.html";

        std::string content;
        readFile(filePath, res, content);

//         if (!validateToken(options, req, res))
//         {
//             replaceStrInString(content, "${{TABLE_CONTENTS}}", "");
//             res.set_content(content, "text/html");
//             return;
//         }

        if (req.params.contains("logs"))
        {
            Utils::ReplaceStrInString(content, "${{TABLE_HEADER}}", "Connection Logs");

            std::string clientId{};
            if (req.params.contains("clientId"))
            {
                clientId = req.params.equal_range("clientId").first->second;
            }

            std::vector<ConnectionInfo> connInfos;
            Mongo mongo(options);
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

                    const std::string time = convertHighResRepToString(connInfo.m_Time);

                    tbody_tr_td.Write(ss, { {"${{row_text}}", (connInfo.m_Connection == ConnectionInfo::Type::Connection ? "Connection" : "Disconnection")} });
                    tbody_tr_td.Write(ss, { {"${{row_text}}", fullLink} });
                    tbody_tr_td.Write(ss, { {"${{row_text}}", time} });
                }
            }
            Utils::ReplaceStrInString(content, "${{TABLE_CONTENTS}}", ss.str());
        }
        else
        {
            Utils::ReplaceStrInString(content, "${{TABLE_HEADER}}", "");
            Utils::ReplaceStrInString(content, "${{TABLE_CONTENTS}}", "");
        }

        res.set_content(content, "text/html");
    });

    svr.Get("/api/protected", [&](const httplib::Request& req, httplib::Response& res)
    {
        Mongo mongo(options);
        validateToken(mongo, options, req, res);
    });

    svr.Get(R"(/api/(.*))", [&](const httplib::Request& req, httplib::Response& res)
    {
        res.status = 200;
        res.set_content("Invalid token", "text/plain");
    });

    svr.Get(R"(/(.*))", [&](const httplib::Request& req, httplib::Response& res)
    {
        const std::string file = req.matches[1];

        std::filesystem::path filePath = options.web.fileServingDir;
        filePath /= file;

        std::string content;
        readFile(filePath, res, content);
        res.set_content(content, "text/html");
    });

    svr.Post("/api/login", [&options](const httplib::Request& req, httplib::Response& res) 
    {
        auto body = nlohmann::json::parse(req.body);
        std::string username = body["username"];
        std::string password = body["password"];

        Mongo mongo(options);

        User user;
        if(mongo.FetchUser(username, user) && user.ValidatePassword(password))
        {
            const std::chrono::system_clock::time_point timepoint = std::chrono::system_clock::now() + std::chrono::days(30);

            const std::string token = jwt::create().set_issuer("auth_server").set_type("JWS").set_payload_claim("username", jwt::claim(username)).set_payload_claim("expiry", jwt::claim(timepoint)).sign(jwt::algorithm::hs256{options.server.identity});

            nlohmann::json response = { {"token", token} };
            res.set_content(response.dump(), "application/json");
        }
        else
        {
            res.status = 401;
            res.set_content("Invalid credentials", "text/plain");
        }
    });

    svr.Post("/api/client-info/clear", [&options](const httplib::Request& req, httplib::Response& res) 
    {
        auto body = nlohmann::json::parse(req.body);
        std::string clientId = body["clientId"];

        Mongo mongo(options);
        mongo.DeleteInfo(Mongo::Database::Stats, Mongo::Collection::Connection, clientId);
    });

    svr.listen(options.web.host, options.web.port);

    return 0;
}