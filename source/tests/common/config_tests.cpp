#define CATCH_CONFIG_MAIN

#include <catch.hpp>

TEST_CASE("FunctionReturnsTrue") 
{
    REQUIRE(true);
}

TEST_CASE("FunctionReturnsFalse") 
{
    REQUIRE(false);
}