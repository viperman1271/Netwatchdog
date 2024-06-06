#include "configurableoption.h"

#include <catch.hpp>

TEST_CASE("ConfigurableOption<std::string>")
{
    SECTION("operator*")
    {
        constexpr const char* STR_VALUE = "TESTSTRING";
        const std::string stringValue{ STR_VALUE };

        ConfigurableOption<std::string> option1{ stringValue };
        ConfigurableOption<std::string> option2{ stringValue };

        CHECK(*option1 == *option2);
        CHECK(*option1 == stringValue);
        CHECK(*option2 == stringValue);
    }

    SECTION("operator=")
    {
        constexpr const char* STR_VALUE = "TESTSTRING";
        const std::string stringValue{ STR_VALUE };

        ConfigurableOption<std::string> option;
        option = stringValue;

        CHECK(*option == stringValue);
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
}

TEST_CASE("ConfigurableOption<int>")
{
    SECTION("operator*")
    {
        constexpr int INT_VALUE = 12345;

        ConfigurableOption<int> option1{ INT_VALUE };
        ConfigurableOption<int> option2{ INT_VALUE };

        CHECK(*option1 == *option2);
        CHECK(*option1 == INT_VALUE);
        CHECK(*option2 == INT_VALUE);
    }

    SECTION("operator=")
    {
        constexpr int INT_VALUE = 12345;

        ConfigurableOption<int> option;
        option = INT_VALUE;

        CHECK(*option == INT_VALUE);
    }

    SECTION("operator<<")
    {
        constexpr int INT_VALUE = 12345;

        ConfigurableOption<int> option{ INT_VALUE };

        std::stringstream ss;

        ss << option;

        CHECK(ss.str() == "12345");
    }
}