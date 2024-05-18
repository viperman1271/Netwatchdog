#include "client.h"

#include "config.h"
#include "options.h"
#include "utils.h"

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