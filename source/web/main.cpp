#include "config.h"
#include "mongo.h"

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

void replaceStrInString(std::string& baseString, const std::string& strToFind, const std::string& strToReplaceFoundStr) 
{
    size_t pos = 0;
    while ((pos = baseString.find(strToFind, pos)) != std::string::npos) 
    {
        baseString.replace(pos, strToFind.length(), strToReplaceFoundStr);
        pos += strToReplaceFoundStr.length();
    }
}

enum class TokenResult
{
    Correct,
    Empty,
    Invalid,
};

TokenResult validateToken(const Options& options, const httplib::Request& req)
{
    const std::string auth_header = req.get_header_value("Authorization");

    if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ")
    {
        return TokenResult::Empty;
    }

    std::string token = auth_header.substr(7); // Remove "Bearer " prefix
    if (!token.empty())
    {
        jwt::decoded_jwt decoded = jwt::decode(token);
        jwt::verifier verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ options.server.identity }).with_issuer("auth_server");

        std::error_code error;
        verifier.verify(decoded, error);

        if (!error)
        {
            return TokenResult::Correct;
        }
    }
    
    return TokenResult::Invalid;
}

bool validateToken(const Options& options, const httplib::Request& req, httplib::Response& res)
{
    switch (validateToken(options, req))
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

    Mongo mongo(options);
    if (!mongo.IsConnected())
    {
        return -1;
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
            replaceStrInString(content, "${{TABLE_HEADER}}", "Connection Logs");

            std::string clientId{};
            if (req.params.contains("clientId"))
            {
                clientId = req.params.equal_range("clientId").first->second;
            }

            std::vector<ConnectionInfo> connInfos;
            mongo.FetchClientInfo(clientId, connInfos);

            const std::string baseIndent = "                                    ";
            std::stringstream ss;

            ss << baseIndent << "<thead>" << std::endl;
            ss << baseIndent << "    <tr class=\"w-full bg-gray-100 text-gray-900\">" << std::endl;
            ss << baseIndent << "        <th class=\"text-center py-2 border-b border-gray-200 text-left sortable\" data-column=\"conn\">Log Type</th>" << std::endl;
            ss << baseIndent << "        <th class=\"text-center py-2 border-b border-gray-200 text-left sortable\" data-column=\"client\">Client ID</th>" << std::endl;
            ss << baseIndent << "        <th class=\"text-center py-2 border-b border-gray-200 text-left sortable\" data-column=\"time\">Time</th>" << std::endl;
            ss << baseIndent << "    </tr>" << std::endl;
            ss << baseIndent << "</thead>" << std::endl;
            ss << baseIndent << "<tbody>" << std::endl;
            for (const ConnectionInfo& connInfo : connInfos)
            {
                char fullLink[256];
                sprintf(fullLink, "<a href=\"dashboard.html?logs&clientId=%s\">", connInfo.m_UniqueId.c_str());

                const std::string time = convertHighResRepToString(connInfo.m_Time);

                ss << baseIndent << "    <tr class=\"text-gray-900\">" << std::endl;
                ss << baseIndent << "        <td class=\"text-center py-2 border-b border-gray-200\">" << (connInfo.m_Connection == ConnectionInfo::Type::Connection ? "Connection" : "Disconnection") << "</td>" << std::endl;
                ss << baseIndent << "        <td class=\"text-center py-2 border-b border-gray-200\">" << fullLink << connInfo.m_UniqueId << "</a>" << "</td>" << std::endl;
                ss << baseIndent << "        <td class=\"text-center py-2 border-b border-gray-200\">" << time << "</td>" << std::endl;
                ss << baseIndent << "    </tr>" << std::endl;
            }
            ss << baseIndent << "</tbody>" << std::endl;

            replaceStrInString(content, "${{TABLE_CONTENTS}}", ss.str());
        }
        else
        {
            replaceStrInString(content, "${{TABLE_HEADER}}", "");
            replaceStrInString(content, "${{TABLE_CONTENTS}}", "");
        }

        res.set_content(content, "text/html");
    });

    svr.Get("/api/protected", [&](const httplib::Request& req, httplib::Response& res)
    {
        validateToken(options, req, res);
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

        if (username == "asdf" && password == "asdf") 
        {
            // Generate JWT
            const std::string token = jwt::create().set_issuer("auth_server").set_type("JWS").set_payload_claim("username", jwt::claim(username)).sign(jwt::algorithm::hs256{ options.server.identity });

            nlohmann::json response = { {"token", token} };
            res.set_content(response.dump(), "application/json");
        }
        else
        {
            res.status = 401;
            res.set_content("Invalid credentials", "text/plain");
        }
    });

    svr.Post("/api/client-info/clear", [&mongo](const httplib::Request& req, httplib::Response& res) 
    {
        auto body = nlohmann::json::parse(req.body);
        std::string clientId = body["clientId"];

        mongo.DeleteInfo(Mongo::Database::Stats, Mongo::Collection::Connection, clientId);
    });

    svr.listen(options.web.host, options.web.port);

    return 0;
}