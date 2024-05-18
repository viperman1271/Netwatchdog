#pragma once

#include "options.h"
#include "utils.h"

#include <CLI/CLI.hpp>
#include <stduuid/uuid.h>
#include <toml.hpp>

#include <filesystem>
#include <iostream>

namespace Config
{
    void LoadOrCreateConfig(Options& options)
    {
        const std::filesystem::path& configPath = Utils::GetConfigPath();

        toml::value config;
        if (!std::filesystem::exists(configPath))
        {
            std::cout << "Configuration file [" << configPath.string() << "] not found... creating." << std::endl;

            config["server"]["host"] = "*";
            config["server"]["port"] = options.port;
            config["server"]["identity"] = uuids::to_string(uuids::uuid_random_generator(g_RNG)());

            config["client"]["host"] = "localhost";
            config["client"]["port"] = options.port;
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

#ifdef NETWATCHDOG_CLIENT
        constexpr const char* CATEGORY = "client";
#elif defined(NETWATCHDOG_SERVER)
        constexpr const char* CATEGORY = "server";
#endif // NETWATCHDOG_CLIENT / NETWATCHDOG_SERVER

        options.host = toml::find<std::string>(config, CATEGORY, "host");
        options.port = toml::find<int>(config, CATEGORY, "port");
        options.identity = toml::find<std::string>(config, CATEGORY, "identity");
    }

    bool ParseCommandLineOptions(int argc, char** argv, Options& options)
    {
        CLI::App app{ "NetWatchdog - ZeroMQ based network monitoring tool." };

        app.add_option("-p,--port", options.port, "The port to use [defaults to 32000]");
        app.add_option("-i,--identity", options.identity, "Identity");
#ifdef NETWATCHDOG_CLIENT 
        app.add_option("-c,--clientCount", options.clientCount, "Number of clients to spawn.");
        app.add_option("--host", options.host, "Host to connect to");
#elif defined(NETWATCHDOG_SERVER)
        app.add_option("--host", options.host, "Listening address for the server [defaults to *]");
#endif // NETWATCHDOG_CLIENT / NETWATCHDOG_SERVER

        try
        {
            (app).parse((argc), (argv));
        }
        catch (const CLI::ParseError& e)
        {
            app.exit(e);
            return false;
        }
        return true;
    }
};