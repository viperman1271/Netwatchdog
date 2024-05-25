#pragma once

#include <string>

struct Options
{
    struct
    {
        unsigned int count = 1;
        std::string host = "localhost";
        std::string identity;
        int port = 32000;
    } client;

    struct
    {
        std::string host = "*";
        std::string identity;
        int port = 32000;
    } server;

    struct
    {
        std::string username;
        std::string password;
        std::string host;
        int port;
    } database;

    struct
    {
        std::string fileServingDir;
        std::string host;
        int port;
    } web;
};