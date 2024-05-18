#include "server.h"

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

    std::string host = "*";
    std::string identity;
};


void loadOrCreateConfig(CommandLineOptions& options)
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

    options.host = toml::find<std::string>(config, "server", "listen_address");
    options.port = toml::find<int>(config, "server", "port");
    options.identity = toml::find<std::string>(config, "server", "identity");
}

bool parseCommandLineOptions(int argc, char** argv, CommandLineOptions& options)
{
    CLI::App app{ "NetWatchdog - ZeroMQ based network monitoring tool." };

    app.add_option("-p,--port", options.port, "The port to use [defaults to 32000]");
    app.add_option("-i,--identity", options.identity, "Host's identity");
    app.add_option("--host", options.host, "Listening address for the server [defaults to *]");

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

    NetWatchdogServer server(options.host, options.identity, options.port);

    g_CallbackFunc = [&server]()
    {
        server.Kill();
    };
    std::signal(SIGINT, handle_signal);

    server.Run();

    return 0;
}