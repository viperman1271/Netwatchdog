#include "config.h"

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
        std::string value = Utils::GetEnvVar(envVariable.c_str());
        if (!value.empty())
        {
            config[category][variable] = value;
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
        if (configFileExists)
        {
            std::cout << "Loading configuration file [" << configPath.string() << "]." << std::endl;
            config = toml::parse(configPath.string());
        }

        options.database.username.Init(config, { "database", "username" }, "MONGO_USERNAME");
        options.database.password.Init(config, { "database", "password" }, "MONGO_PASSWORD");
        options.database.host.Init(config, { "database", "host" }, "MONGO_HOST");
        options.database.port.Init(config, { "database", "port" }, "MONGO_PORT");

        options.client.host.Init(config, { "client", "host" });
        options.client.port.Init(config, { "client", "port" });
        options.client.identity.Init(config, { "client", "identity" });
        options.client.secure.Init(config, { "client", "secure" });

        options.server.host.Init(config, { "server", "host" });
        options.server.port.Init(config, { "server", "port" });
        options.server.identity.Init(config, { "server", "identity" });
        options.server.secure.Init(config, { "server", "secure" });

        options.web.fileServingDir.Init(config, { "web", "serving_dir" });
        options.web.host.Init(config, { "web", "host" });
        options.web.port.Init(config, { "web", "port" });
        options.web.secure.Init(config, { "web", "secure" });
        options.web.securePort.Init(config, { "web", "secure_port" });
        options.web.privateKeyPath.Init(config, { "web", "private_key_path" });
        options.web.publicKeyPath.Init(config, { "web", "public_key_path" });
        options.web.certificatePath.Init(config, { "web", "certificate_path" });

        options.admin.direct.Init(config, { "admin", "direct" });

        if (!configFileExists)
        {
            std::cout << "Configuration file [" << configPath.string() << "] not found... creating." << std::endl;

            std::ofstream ofs(configPath.string());
            ofs << config;
            ofs.close();
        }
    }

    bool ParseCommandLineOptions(int argc, char** argv, Options& options, const ParsingType parsingType)
    {
        CLI::App app{ "NetWatchdog - ZeroMQ based network monitoring tool." };

        if (parsingType == ParsingType::Client)
        {
            app.add_option("-p,--port", options.client.port, "The port to use [defaults to 32000]");
            app.add_option("-i,--identity", options.client.identity, "Identity");
            app.add_option("--host", options.client.host, "Host to connect to");
            app.add_option("-c,--clientCount", options.client.count, "Number of clients to spawn.");
        }
        else if (parsingType == ParsingType::Server || parsingType == ParsingType::Admin || parsingType == ParsingType::Web)
        {
            app.add_option("-p,--port", options.server.port, "The port to use [defaults to 32000]");
            app.add_option("-i,--identity", options.server.identity, "Identity");
            app.add_option("--host", options.server.host, "Listening address for the server [defaults to *]");

            app.add_option("--username", options.database.username, "Username for database access [defaults to root]");
            app.add_option("--password", options.database.password, "Password for database access");
            app.add_option("--db_host", options.database.host, "Database host address");
            app.add_option("--db_port", options.database.port, "Database port [defaults to 27017]");
        }

        if (parsingType == ParsingType::Admin)
        {
            app.add_flag("--direct", options.admin.direct, "Whether or not the connection should be directly to the database.");
            app.add_option("-c,--create_user", options.admin.userToCreate, "Username of the user to create");
            app.add_option("--user_password", options.admin.userPassword, "Password of the user to create");
            app.add_flag("--user_is_admin", options.admin.userIsAdmin, "Whether or not the user should be an admin.");
        }

        if (parsingType == ParsingType::Web)
        {
            app.add_option("--secure_port", options.web.securePort, "The web secure port to use [defaults to 443]");
            app.add_option("--private_key", options.web.privateKeyPath, "Path to find the private key for SSL cert");
            app.add_option("--public_key", options.web.publicKeyPath, "Path to find the public key for SSL cert");
            app.add_option("--certificate", options.web.certificatePath, "Path to find the certificate for SSL cert");
        }

        switch (parsingType)
        {
        case ParsingType::Client:
            app.add_flag("-s,--secure", options.client.secure, "Whether secure communications (i.e https) should be used [defaults to true]");
            break;

        case ParsingType::Server:
            app.add_flag("-s,--secure", options.server.secure, "Whether secure communications (i.e https) should be used [defaults to true]");
            break;

        case ParsingType::Web:
            app.add_flag("-s,--secure", options.web.secure, "Whether secure communications (i.e https) should be used [defaults to true]");
            break;
        }

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
}