#include "config.h"
#include "mongo.h"
#include "webtemplate.h"
#include "webserver.h"

int main(int argc, char** argv)
{
    Options options;
    Config::LoadOrCreateConfig(options);
    if (!Config::ParseCommandLineOptions(argc, argv, options, Config::ParsingType::Web))
    {
        return -1;
    }

    WebServer webServer(options);
    if (!webServer.Run())
    {
        return -1;
    }

    return 0;
}