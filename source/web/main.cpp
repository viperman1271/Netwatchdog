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

int main(int argc, char** argv)
{
    Options options;
    Config::LoadOrCreateConfig(options);
    if (!Config::ParseCommandLineOptions(argc, argv, options))
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

        if (req.params.contains("logs"))
        {

        }
        else
        {
            std::stringstream data;
            mongo.DumpClientInfo(clientId, data, "<br/>");

            replaceStrInString(content, "${{TABLE_CONTENTS}}", "");
        }

        res.set_content(content, "text/html");
    });

    svr.Get("/api/protected", [&](const httplib::Request& req, httplib::Response& res)
    {
        const std::string auth_header = req.get_header_value("Authorization");

        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ")
        {
            res.status = 401;
            res.set_content("Authorization required", "text/plain");
            return;
        }

        std::string token = auth_header.substr(7); // Remove "Bearer " prefix
        if (!token.empty())
        {
            jwt::decoded_jwt decoded = jwt::decode(token);
            jwt::verifier verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ options.identity }).with_issuer("auth_server");

            std::error_code error;
            verifier.verify(decoded, error);

            if (!error)
            {
                res.set_content("Access granted to protected resource", "text/plain");
            }
            else
            {
                res.status = 403;
                res.set_content("Invalid token", "text/plain");
            }
        }
        else
        {
            res.status = 403;
            res.set_content("Invalid token", "text/plain");
        }
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
            const std::string token = jwt::create().set_issuer("auth_server").set_type("JWS").set_payload_claim("username", jwt::claim(username)).sign(jwt::algorithm::hs256{ options.identity });

            nlohmann::json response = { {"token", token} };
            res.set_content(response.dump(), "application/json");
        }
        else
        {
            res.status = 401;
            res.set_content("Invalid credentials", "text/plain");
        }
    });

    svr.Get(R"(/client-data/(.*))", [&mongo](const httplib::Request& req, httplib::Response& res) 
    {
        const std::string clientId = req.matches[1];

        std::stringstream data;
        if (mongo.DumpClientInfo(clientId, data, "<br/>"))
        {
            std::stringstream ss;
            ss << "<!DOCTYPE html>" << std::endl;
            ss << "<html lang=\"en\">" << std::endl;
            ss << "<head>" << std::endl;
            ss << "    <meta charset=\"UTF-8\">" << std::endl;
            ss << "    <meta name = \"viewport\" content = \"width=device-width, initial-scale=1.0\">" << std::endl;
            ss << "    <title>netwatchdog: " << clientId << "</title>" << std::endl;
            ss << "    <link rel = \"apple-touch-icon\" sizes=\"180x180\" href=\"/data/apple-touch-icon.png\">" << std::endl;
            ss << "    <link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"/data/favicon-32x32.png\">" << std::endl;
            ss << "    <link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/data/favicon-16x16.png\">" << std::endl;
            ss << "    <link rel=\"icon\" type=\"image/x-icon\" href=\"/data/favicon.ico\">" << std::endl;
            ss << "    <link rel=\"manifest\" href=\"/data/site.webmanifest\">" << std::endl;
            ss << "    <link rel = \"stylesheet\" href=\"/data/styles.css\">" << std::endl;
            ss << "</head>" << std::endl;
            ss << "<body>" << std::endl;
            ss << "    <h1>netwatchdog: " << clientId << "</h1>" << std::endl;
            ss << data.str();
            ss << "</body>" << std::endl;
            ss << "</html>" << std::endl;

            res.set_content(ss.str(), "text/html");
            res.status = 400;
        }
        else
        {
            res.status = 404;
        }
    });

    svr.listen(options.web.host, options.web.port);

    return 0;
}