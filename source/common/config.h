#pragma once

#include "options.h"
#include "utils.h"

#include <CLI/CLI.hpp>
#include <stduuid/uuid.h>
#include <toml.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#include <filesystem>
#include <iostream>

namespace Config
{
    bool ValueExists(toml::value& config, const std::string& category, const std::string& variable)
    {
        if (config.type() != toml::value_t::empty)
        {
            auto& tab = config.as_table();
            if (tab.count(category) != 0)
            {
                auto& subtab = config[category].as_table();
                if (subtab.count(variable) != 0)
                {
                    return true;
                }
            }
        }

        return false;
    }

    void ConfigureIfEnvVarNotEmpty(toml::value& config, const std::string& category, const std::string& variable, const std::string& envVariable)
    {
#ifdef _WIN32
        constexpr int envVarStrLen = 256;
        char* const value = reinterpret_cast<char*>(alloca(envVarStrLen * sizeof(char)));
        memset(value, 0, envVarStrLen);

        GetEnvironmentVariable(envVariable.c_str(), value, envVarStrLen);
        if (strlen(value) > 0)
        {
            config[category][variable] = value;
        }
#else
        const char* const value = std::getenv(envVariable.c_str());
        if (value != nullptr && strlen(value) > 0)
        {
            config[category][variable] = value;
        }
#endif
    }

    template<typename T>
    void ConfigureDefaultValue(toml::value& config, const std::string& category, const std::string& variable, const T& defaultValue)
    {
        if (!ValueExists(config, category, variable))
        {
            config[category][variable] = defaultValue;
        }
    }

    template<typename T>
    void GetValue(toml::value& config, const std::string& category, const std::string& variable, T& value)
    {
        if (config.type() != toml::value_t::empty)
        {
            auto& tab = config.as_table();
            if (tab.count(category) != 0)
            {
                auto& subtab = config[category].as_table();
                if (subtab.count(variable) != 0)
                {
                    value = toml::find<T>(config, category, variable);
                }
            }
        }
    }

    bool ValidateIdentity(toml::value& config, const std::string& category, const std::string& variable)
    {
        std::string identity;
        GetValue(config, category, variable, identity);
        std::optional<uuids::uuid> uuid = uuids::uuid::from_string(identity);
        if (uuid.has_value())
        {
            return true;
        }

        return false;
    }

    void LoadOrCreateConfig(Options& options)
    {
        const std::filesystem::path& configPath = Utils::GetConfigPath();
        const bool configFileExists = std::filesystem::exists(configPath);

        toml::value config;
        if(configFileExists)
        {
            std::cout << "Loading configuration file [" << configPath.string() << "]." << std::endl;
            config = toml::parse(configPath.string());
        }

        const bool validServerIdentity = ValidateIdentity(config, "server", "identity");
        const bool validClientIdentity = ValidateIdentity(config, "client", "identity");
        if (!validServerIdentity || !validClientIdentity)
        {
            if (!validServerIdentity)
            {
                config["server"]["identity"] = uuids::to_string(uuids::uuid_random_generator(g_RNG)());
            }

            if (!validClientIdentity)
            {
                config["client"]["identity"] = uuids::to_string(uuids::uuid_random_generator(g_RNG)());
            }

            std::cout << "Configuration file [" << configPath.string() << "] contained invalid identity fields... updating." << std::endl;

            std::ofstream ofs(configPath.string());
            ofs << config;
            ofs.close();
        }

        ConfigureDefaultValue(config, "server", "host", "*");
        ConfigureDefaultValue(config, "server", "port", options.port);

        ConfigureDefaultValue(config, "client", "host", "localhost");
        ConfigureDefaultValue(config, "client", "port", options.port);

        ConfigureDefaultValue(config, "database", "username", "root");
        ConfigureDefaultValue(config, "database", "password", "password1234");
        ConfigureDefaultValue(config, "database", "host", "localhost");
        ConfigureDefaultValue(config, "database", "port", 27017);

        ConfigureDefaultValue(config, "web", "host", "0.0.0.0");
        ConfigureDefaultValue(config, "web", "port", 8000);
        ConfigureDefaultValue(config, "web", "serving_dir", Utils::GetBasePath().string());

        if (!configFileExists)
        {
            std::cout << "Configuration file [" << configPath.string() << "] not found... creating." << std::endl;

            std::ofstream ofs(configPath.string());
            ofs << config;
            ofs.close();
        }

#ifdef NETWATCHDOG_SERVER
        ConfigureIfEnvVarNotEmpty(config, "database", "username", "MONGO_USERNAME");
        ConfigureIfEnvVarNotEmpty(config, "database", "password", "MONGO_PASSWORD");
        ConfigureIfEnvVarNotEmpty(config, "database", "host", "MONGO_HOST");
        ConfigureIfEnvVarNotEmpty(config, "database", "port", "MONGO_PORT");
#endif // NETWATCHDOG_SERVER

#ifdef NETWATCHDOG_CLIENT
        constexpr const char* CATEGORY = "client";
#elif defined(NETWATCHDOG_SERVER)
        constexpr const char* CATEGORY = "server";
#endif // NETWATCHDOG_CLIENT / NETWATCHDOG_SERVER

        options.host = toml::find<std::string>(config, CATEGORY, "host");
        options.port = toml::find<int>(config, CATEGORY, "port");
        options.identity = toml::find<std::string>(config, CATEGORY, "identity");

        options.web.fileServingDir = toml::find<std::string>(config, "web", "serving_dir");
        options.web.host = toml::find<std::string>(config, "web", "host");
        options.web.port = toml::find<int>(config, "web", "port");

#ifdef NETWATCHDOG_SERVER
        options.database.username = toml::find<std::string>(config, "database", "username");
        options.database.password = toml::find<std::string>(config, "database", "password");
        options.database.host = toml::find<std::string>(config, "database", "host");
        options.database.port = toml::find<int>(config, "database", "port");
#endif // NETWATCHDOG_SERVER
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

        app.add_option("--username", options.database.username, "Username for database access [defaults to root]");
        app.add_option("--password", options.database.password, "Password for database access");
        app.add_option("--db_host", options.database.host, "Database host address");
        app.add_option("--db_port", options.database.port, "Database port [defaults to 27017]");
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