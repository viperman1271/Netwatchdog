#pragma once

#include "utils.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <cereal/cereal.hpp>

#include <random>

struct ConnectionInfo final
{
    ConnectionInfo()
        : m_Connection(Type::Unknown)
        , m_Time(std::chrono::system_clock::now().time_since_epoch().count())
    {
    }

    enum class Type : int
    {
        Unknown,
        Connection,
        Disconnection,
    };

    template<class TSerializer>
    void serialize(TSerializer& serializer)
    {
        serializer(cereal::make_nvp("connection", m_ConnectionInteger), cereal::make_nvp("unique-id", m_UniqueId), cereal::make_nvp("time", m_Time));
    }

    void serialize(bsoncxx::builder::stream::document& document)
    {
        document << "connection" << m_ConnectionInteger << "unique-id" << m_UniqueId.c_str() << "time" << m_Time;
    }

    union
    {
        Type m_Connection;
        int m_ConnectionInteger;
    };
    std::string m_UniqueId;
    std::chrono::system_clock::duration::rep m_Time;
};

struct User final
{
    template<class TSerializer>
    void serialize(TSerializer& serializer)
    {
        serializer(cereal::make_nvp("id", m_Id), cereal::make_nvp("email-address", m_EmailAddress), cereal::make_nvp("username", m_Username), cereal::make_nvp("password", m_Password), cereal::make_nvp("salt", m_Salt));
    }

    void serialize(bsoncxx::builder::stream::document& document)
    {
        document << "id" << m_Id << "email-address" << m_EmailAddress.c_str() << "username" << m_Username.c_str() << "password" << m_Password << "salt" << m_Salt;
    }

    std::string m_Id;
    std::string m_EmailAddress;
    std::string m_Username;
    std::string m_Password;
    std::string m_Salt;
};

struct ApiKey final
{
    ApiKey()
        : m_CreationTime(std::chrono::system_clock::now().time_since_epoch().count())
        , m_Expiration(std::numeric_limits<std::chrono::system_clock::rep>::max())
        , m_Permissions(Type::None)
    {
        generate();
    }

    enum class Type : unsigned int
    {
        None                = 0b000000,

        ConnectionInfoRead  = 0b000001,
        ConnecitonInfoWrite = 0b000010,
        ApiKeyRead          = 0b000100,
        ApiKeyWrite         = 0b001000,
        ProfileRead         = 0b010000,
        ProfileUpdate       = 0b100000,

        All                 = ConnectionInfoRead | ConnecitonInfoWrite | ApiKeyRead | ApiKeyWrite | ProfileRead | ProfileUpdate,
    };

    template<class TSerializer>
    void serialize(TSerializer& serializer)
    {
        serializer(cereal::make_nvp("user", m_User), cereal::make_nvp("api-key", m_ApiKey), cereal::make_nvp("creation", m_CreationTime), cereal::make_nvp("expiration", m_Expiration), cereal::make_nvp("permissions", m_PermissionsInteger));
    }

    void serialize(bsoncxx::builder::stream::document& document)
    {
        document << "user" << m_User.c_str() << "api-key" << m_ApiKey.c_str() << "creation" << m_CreationTime << "expiration" << m_Expiration << "permissions" << m_PermissionsInteger;
    }

    void generate()
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

    std::string m_User;
    std::string m_ApiKey;
    std::chrono::system_clock::duration::rep m_CreationTime;
    std::chrono::system_clock::duration::rep m_Expiration;
    union
    {
        Type m_Permissions;
        int m_PermissionsInteger;
    };
};