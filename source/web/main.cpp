#include "config.h"
#include "mongo.h"

#include <httplib.h>

#include <filesystem>

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

    svr.set_mount_point("/data", options.web.fileServingDir);

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