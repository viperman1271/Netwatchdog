#include "objectmodel.h"

#include <bcrypt/BCrypt.hpp>
#include <stduuid/uuid.h>

constexpr int WORKLOAD = 12;

User::User()
    : m_IsAdmin(false)
    , m_Id(uuids::to_string(uuids::uuid_random_generator(g_RNG)()))
{
}

bool User::ValidatePassword(const std::string& unhashedPassword) const
{
    return BCrypt::validatePassword(unhashedPassword, m_Password);
}

void User::SetPassword(const std::string& password)
{
    m_Password = BCrypt::generateHash(password, WORKLOAD);
}

void ApiKey::Generate()
{
    constexpr unsigned int API_KEY_LENGTH = 32;

    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::uniform_int_distribution<> distribution(0, static_cast<unsigned int>(characters.size() - 1));

    m_ApiKey.clear();

    for (unsigned int i = 0; i < API_KEY_LENGTH; ++i)
    {
        m_ApiKey += characters[distribution(g_RNG)];
    }
}