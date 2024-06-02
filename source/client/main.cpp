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
    if (!Config::ParseCommandLineOptions(argc, argv, options, Config::ParsingType::Client))
    {
        return -1;
    }

    if(options.client.count > 1)
    {
        std::vector<std::unique_ptr<NetWatchdogClient>> clients;

        g_CallbackFunc = [&clients]()
        {
            for (std::unique_ptr<NetWatchdogClient>& client : clients)
            {
                client->Kill();
            }
        };

        for (unsigned int i = 0; i < options.client.count; ++i)
        {
            Options optionsCopy = options;
            
            std::stringstream ss;
            ss << options.client.identity << i;
            optionsCopy.client.identity = ss.str();
            
            clients.push_back(std::unique_ptr<NetWatchdogClient>{ new NetWatchdogClient(optionsCopy) });
            clients[i]->Run(true);
        }

        for (std::unique_ptr<NetWatchdogClient>& client : clients)
        {
            client->Wait();
        }
    }
    else
    {
        NetWatchdogClient client(options);

        g_CallbackFunc = [&client]()
        {
            client.Kill();
        };
        std::signal(SIGINT, handle_signal);
        client.Run();
    }

    return 0;
}