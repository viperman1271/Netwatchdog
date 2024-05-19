#pragma once

#include <string>

struct Options
{
    int port = 32000;
#ifdef NETWATCHDOG_CLIENT
    unsigned int clientCount = 1;
#endif // NETWATCHDOG_CLIENT

#ifdef NETWATCHDOG_CLIENT
    std::string host = "localhost";
#elif defined(NETWATCHDOG_SERVER)
    std::string host = "*";
#endif // NETWATCHDOG_CLIENT / NETWATCHDOG_SERVER
    std::string identity;

#ifdef NETWATCHDOG_SERVER
    struct
    {
        std::string username;
        std::string password;
        std::string host;
        int port;
    } database;
#endif // NETWATCHDOG_SERVER
};