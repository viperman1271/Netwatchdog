#include "client.h"

#include "utils.h"

#include <CLI/CLI.hpp>
#include <toml.hpp>
#include <stduuid/uuid.h>

#include <csignal>
#include <iostream>

std::function<void()> g_CallbackFunc;

// Signal handler function
void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        std::cout << "SIGINT received, shutting down..." << std::endl;
        if (g_CallbackFunc)
        {
            g_CallbackFunc();
        }
    }
}

struct CommandLineOptions
{
    int port = 32000;
    unsigned int clientCount = 1;

    std::string host = "localhost";
    std::string identity;
};

void loadOrCreateConfig(CommandLineOptions& options)
{
    const std::filesystem::path& configPath = Utils::GetLocalConfigPath();

    toml::value config;
    if (!std::filesystem::exists(configPath))
    {
        std::cout << "Configuration file [" << configPath.string() << "] not found... creating." << std::endl;
        config["connection"]["host"] = "localhost";
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

    options.host = toml::find<std::string>(config, "connection", "host");
    options.port = toml::find<int>(config, "connection", "port");
    options.identity = toml::find<std::string>(config, "client", "identity");
}

bool parseCommandLineOptions(int argc, char** argv, CommandLineOptions& options)
{
    CLI::App app{ "NetWatchdog - ZeroMQ based network monitoring tool." };

    app.add_option("-p,--port", options.port, "The port to use");
    app.add_option("-i,--identity", options.identity, "Host to connect to");
    app.add_option("-c,--clientCount", options.clientCount, "Number of clients to spawn.");
    app.add_option("--host", options.host, "Host to connect to");

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

int main(int argc, char** argv)
{
    CommandLineOptions options;
    loadOrCreateConfig(options);
    if (!parseCommandLineOptions(argc, argv, options))
    {
        return -1;
    }

    if(options.clientCount > 1)
    {
        std::vector<std::unique_ptr<NetWatchdogClient>> clients;

        g_CallbackFunc = [&clients]()
        {
            for (std::unique_ptr<NetWatchdogClient>& client : clients)
            {
                client->Kill();
            }
        };

        for (unsigned int i = 0; i < options.clientCount; ++i)
        {
            std::stringstream ss;
            ss << options.identity << i;
            clients.push_back(std::unique_ptr<NetWatchdogClient>{ new NetWatchdogClient(options.host, ss.str(), options.port) });
            clients[i]->Run(true);
        }

        for (std::unique_ptr<NetWatchdogClient>& client : clients)
        {
            client->Wait();
        }
    }
    else
    {
        NetWatchdogClient client(options.host, options.identity, options.port);

        g_CallbackFunc = [&client]()
        {
            client.Kill();
        };
        std::signal(SIGINT, handle_signal);
        client.Run();
    }

    return 0;
}