#include "config.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <catch.hpp>

#include <stdlib.h>

TEST_CASE("ValueExists Returns Expected Value")
{
    toml::value config;

    CHECK_FALSE(Config::ValueExists(config, "test", "value1"));

    config["test"]["value2"] = true;

    CHECK(Config::ValueExists(config, "test", "value2"));
}

TEST_CASE("ConfigureIfEnvVarNotEmpty") 
{
    toml::value config;

    Config::ConfigureIfEnvVarNotEmpty(config, "test", "value1", "ENV_c11c0f45-aadc-4d9a-af99-f9e33b92d9bd");

    CHECK_FALSE(Config::ValueExists(config, "test", "value1"));

    static const std::string ENV_VALUE{ "23456" };
#ifdef _WIN32
    CHECK(SetEnvironmentVariable("ENV_c11c0f45-aadc-4d9a-af99-f9e33b92d9bd", ENV_VALUE.c_str()));
#else
    CHECK(setenv("ENV_c11c0f45-aadc-4d9a-af99-f9e33b92d9bd", ENV_VALUE.c_str(), 0) == 0);
#endif

    Config::ConfigureIfEnvVarNotEmpty(config, "test", "value1", "ENV_c11c0f45-aadc-4d9a-af99-f9e33b92d9bd");

    std::string value;
    Config::GetValue(config, "test", "value1", value);

    CHECK(value == ENV_VALUE);

    static const std::string CONFIG_VALUE { "34567" };
    config["test"]["value2"] = CONFIG_VALUE;
    Config::ConfigureIfEnvVarNotEmpty(config, "test", "value2", "ENV_c11c0f45-aadc-4d9a-af99-f9e33b92d9bd");

    Config::GetValue(config, "test", "value2", value);

    CHECK(value == CONFIG_VALUE);
}

TEST_CASE("ConfigureDefaultValue")
{
    toml::value config;

    CHECK_FALSE(Config::ValueExists(config, "test", "value1"));

    static const std::string CONFIG_VALUE{ "45678" };
    Config::ConfigureDefaultValue(config, "test", "value1", CONFIG_VALUE);

    std::string value;
    Config::GetValue(config, "test", "value1", value);

    CHECK(value == CONFIG_VALUE);

    static const std::string VALUE{ "56789" };
    config["test"]["value2"] = VALUE;

    Config::ConfigureDefaultValue(config, "test", "value2", CONFIG_VALUE);

    Config::GetValue(config, "test", "value2", value);

    CHECK(value == VALUE);
}

TEST_CASE("GetValue returns expected value")
{
    toml::value config;

    int value = std::numeric_limits<int>::max();

    Config::GetValue(config, "test", "value1", value);

    CHECK(value == std::numeric_limits<int>::max());
    
    constexpr int TEST_VALUE = 12345;
    config["test"]["value2"] = TEST_VALUE;

    Config::GetValue(config, "test", "value1", value);

    CHECK(value == TEST_VALUE);
}

TEST_CASE("ValidateIdentity")
{
    toml::value config;

    config["test"]["value1"] = "1234";
    CHECK_FALSE(Config::ValidateIdentity(config, "test", "value1"));

    config["test"]["value2"] = uuids::to_string(uuids::uuid_random_generator(g_RNG)());
    CHECK(Config::ValidateIdentity(config, "test", "value2"));
}