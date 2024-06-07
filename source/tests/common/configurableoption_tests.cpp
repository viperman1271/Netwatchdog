#include "configurableoption.h"

#include <catch.hpp>
#include <CLI/CLI.hpp>

TEST_CASE("ConfigurableOption<std::string>")
{
    SECTION("operator*")
    {
        constexpr const char* STR_VALUE = "TESTSTRING";
        const std::string stringValue{ STR_VALUE };

        ConfigurableOption<std::string> option1{ stringValue };
        ConfigurableOption<std::string> option2{ stringValue };

        CHECK(option1 == option2);
        CHECK(option1 == stringValue);
        CHECK(option2 == stringValue);
    }

    SECTION("operator=")
    {
        constexpr const char* STR_VALUE = "TESTSTRING";
        const std::string stringValue{ STR_VALUE };

        ConfigurableOption<std::string> option;
        option = stringValue;

        CHECK(option == stringValue);
    }

    SECTION("operator->")
    {
        constexpr const char* STR_VALUE = "TESTSTRING";
        const std::string stringValue{ STR_VALUE };

        ConfigurableOption<std::string> option;

        CHECK(option->empty());
        CHECK(const_cast<const ConfigurableOption<std::string>&>(option)->empty());

        option = stringValue;

        CHECK_FALSE(option->empty());
        CHECK_FALSE(const_cast<const ConfigurableOption<std::string>&>(option)->empty());
    }

    SECTION("operator<<")
    {
        constexpr const char* STR_VALUE = "TESTSTRING";
        const std::string stringValue{ STR_VALUE };

        ConfigurableOption<std::string> option{ stringValue };

        std::stringstream ss;

        ss << option;

        CHECK(ss.str() == stringValue);
    }

    SECTION("CLI11")
    {
        constexpr const char* STR_VALUE = "TESTSTRING";
        const std::string stringValue{ STR_VALUE };

        ConfigurableOption<std::string> option{ stringValue };
        CLI::App app{ "Unit Tests" };

        app.add_option("-f,--flag", option.GetCommandLine(), "");

        constexpr const char* CLI_STR_VALUE = "CLI_TEST_STRING";

        int argc = 3;
        const char* argv[] = { ".exe", "-f", CLI_STR_VALUE};

        app.parse(argc, argv);

        CHECK(option == std::string{ CLI_STR_VALUE });
        CHECK(option.GetFromCommandLine());
    }
}

TEST_CASE("ConfigurableOption<int>")
{
    SECTION("operator*")
    {
        constexpr int INT_VALUE = 12345;

        ConfigurableOption<int> option1{ INT_VALUE };
        ConfigurableOption<int> option2{ INT_VALUE };

        CHECK(option1 == option2);
        CHECK(option1 == INT_VALUE);
        CHECK(option2 == INT_VALUE);
    }

    SECTION("operator=")
    {
        constexpr int INT_VALUE = 12345;

        ConfigurableOption<int> option;
        option = INT_VALUE;

        CHECK(option == INT_VALUE);
    }

    SECTION("operator<<")
    {
        constexpr int INT_VALUE = 12345;

        ConfigurableOption<int> option{ INT_VALUE };

        std::stringstream ss;

        ss << option;

        CHECK(ss.str() == "12345");
    }

    SECTION("CLI11")
    {
        constexpr int INT_VALUE = 12345;

        ConfigurableOption<int> option{ INT_VALUE };
        CLI::App app{ "Unit Tests" };

        app.add_option("-f,--flag", option.GetCommandLine(), "");

        constexpr int CLI_INT_VALUE = 54321;

        int argc = 3;
        const char* argv[] = { ".exe", "-f", "54321" };

        app.parse(argc, argv);

        CHECK(option == CLI_INT_VALUE);
        CHECK(option.GetFromCommandLine());
    }
}