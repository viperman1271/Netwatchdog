#include "client.h"

#include "utils.h"

#include <toml.hpp>
#include <stduuid/uuid.h>

#include <iostream>

int main()
{
    const std::filesystem::path& configPath = Utils::GetLocalConfigPath();

    toml::value config;
    if (!std::filesystem::exists(configPath))
    {
        std::cout << "Configuration file [" << configPath.string() << "] not found... creating." << std::endl;
        config["connection"]["server"] = "localhost";
        config["connection"]["port"] = 32000;
        config["client"]["identity"] = uuids::to_string(uuids::uuid_random_generator(g_RNG)());

        std::ofstream ofs(configPath.string());
        ofs << config;
        ofs.close();
    }
    else
    {
        std::cout << "Loading configuration file [" << configPath.string() << "]." << std::endl;
        config = toml::parse(configPath.string());
    }

    const std::string server = toml::find<std::string>(config, "connection", "server");
    const std::string identity = toml::find<std::string>(config, "client", "identity");
    const int port = toml::find<int>(config, "connection", "port");
    NetWatchdogClient client(server, identity, port);
    client.Run();

    return 0;
}