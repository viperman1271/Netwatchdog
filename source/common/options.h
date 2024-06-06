#pragma once

#include "configurableoption.h"
#include "utils.h"

struct Options
{
    struct
    {
        ConfigurableOption<unsigned int> count{ 1 };
        ConfigurableOption<std::string> host{ "localhost" };
        ConfigurableOption<std::string> identity{ uuids::to_string(uuids::uuid_random_generator(g_RNG)()) };
        ConfigurableOption<int> port{ 32000 };
        ConfigurableOption<bool> secure{ true };
    } client;

    struct
    {
        ConfigurableOption<std::string> host{ "*" };
        ConfigurableOption<std::string> identity{ uuids::to_string(uuids::uuid_random_generator(g_RNG)()) };
        ConfigurableOption<int> port{ 32000 };
        ConfigurableOption<bool> secure{ true };
    } server;

    struct
    {
        ConfigurableOption<std::string> username{ "root" };
        ConfigurableOption<std::string> password;
        ConfigurableOption<std::string> host{ "localhost" };
        ConfigurableOption<int> port{ true };
    } database;

    struct
    {
        ConfigurableOption<std::string> fileServingDir{ Utils::GetWebFileServingPath().string() };
        ConfigurableOption<std::string> host{ "0.0.0.0" };
        ConfigurableOption<std::string> privateKeyPath{ "private_key.pem" };
        ConfigurableOption<std::string> publicKeyPath{ "public_key.pem" };
        ConfigurableOption<std::string> certificatePath{ "certificate.pem" };
        ConfigurableOption<int> port{ 80 };
        ConfigurableOption<int> securePort{ 443 };
        ConfigurableOption<bool> secure{ true };
    } web;

    struct
    {
        ConfigurableOption<std::string> userToCreate;
        ConfigurableOption<std::string> userPassword;
        ConfigurableOption<bool> direct{ true };
        ConfigurableOption<bool> userIsAdmin;
    } admin;
};