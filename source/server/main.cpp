#include "server.h"

#include "utils.h"

#include <toml.hpp>
#include <stduuid/uuid.h>

#include <iostream>

int main()
{
    const std::filesystem::path& configPath = Utils::GetServerConfigPath();

    toml::value config;
    if (!std::filesystem::exists(configPath))
    {
        std::cout << "Configuration file [" << configPath.string() << "] not found... creating." << std::endl;
        config["server"]["listen_address"] = "*";
        config["server"]["port"] = 32000;
        config["server"]["identity"] = uuids::to_string(uuids::uuid_random_generator(g_RNG)());

        std::ofstream ofs(configPath.string());
        ofs << config;
        ofs.close();
    }
    else
    {
        std::cout << "Loading configuration file [" << configPath.string() << "]." << std::endl;
        config = toml::parse(configPath.string());
    }

    const std::string listenAddress = toml::find<std::string>(config, "server", "listen_address");
    const std::string identity = toml::find<std::string>(config, "server", "identity");
    const int port = toml::find<int>(config, "server", "port");

    NetWatchdogServer server(listenAddress, identity, port);
    server.Run();

    return 0;
}