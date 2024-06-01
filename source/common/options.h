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
        bool secure;
    } client;

    struct
    {
        std::string host = "*";
        std::string identity;
        int port = 32000;
        bool secure;
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
        std::string privateKeyPath;
        std::string publicKeyPath;
        std::string certificatePath;
        int port;
        int securePort;
        bool secure;
    } web;

    struct
    {
        std::string userToCreate;
        std::string userPassword;
        bool direct;
        bool userIsAdmin;
    } admin;
};