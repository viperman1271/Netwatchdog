#include "server.h"

#include "config.h"
#include "options.h"
#include "utils.h"

#include <chrono>
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

int main(int argc, char** argv)
{
    Options options;
    Config::LoadOrCreateConfig(options);
    if (!Config::ParseCommandLineOptions(argc, argv, options))
    {
        return -1;
    }

    NetWatchdogServer server(options);

    g_CallbackFunc = [&server]()
    {
        server.Kill();
    };
    std::signal(SIGINT, handle_signal);

    server.Run();

    return 0;
}