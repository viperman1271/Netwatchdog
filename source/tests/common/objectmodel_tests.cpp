#include "objectmodel.h"

#include <catch.hpp>

#include <stdlib.h>

TEST_CASE("ValidatePassword returns expected value based upon value")
{
    constexpr const char* PASSWORD = "asdfasdf";
    constexpr const char* WRONG_PASSWORD = "fdsafdsa";

    User user;
    user.SetPassword(PASSWORD);

    CHECK(user.ValidatePassword(PASSWORD));

    CHECK(strcmp(PASSWORD, WRONG_PASSWORD) != 0);
    CHECK_FALSE(user.ValidatePassword(WRONG_PASSWORD));
}

TEST_CASE("ApiKey generates key on constructor")
{
    ApiKey key;
    CHECK(key.m_ApiKey.size() == 32);

    ApiKey key2;
    CHECK(key.m_ApiKey != key2.m_ApiKey);
}