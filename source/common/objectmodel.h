#pragma once

#include "utils.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/optional.hpp>

#include <random>

struct MongoDatabaseItem
{
    void BaseSerialize(bsoncxx::document::view& view)
    {
        bsoncxx::oid oid = view["_id"].get_oid().value;
        m_Oid = oid.to_string();
    }

    template<class TSerializer, class TValue>
    void SafeSerialize(TSerializer& serializer, cereal::NameValuePair<TValue> nvp)
    {
        try
        {
            serializer(nvp);
        }
        catch (cereal::Exception)
        {

        }
    }

    [[nodiscard]] bsoncxx::oid GetOid() const
    {
        return bsoncxx::oid{ m_Oid };
    }

    std::string m_Oid;
};

struct ConnectionInfo final : public MongoDatabaseItem
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
    void Serialize(TSerializer& serializer)
    {
        serializer(cereal::make_nvp("connection", m_ConnectionInteger), cereal::make_nvp("unique-id", m_UniqueId), cereal::make_nvp("time", m_Time));
    }

    void Serialize(bsoncxx::builder::stream::document& document)
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

struct User final : public MongoDatabaseItem
{
    User();

    template<class TSerializer>
    void Serialize(TSerializer& serializer)
    {
        SafeSerialize(serializer, cereal::make_nvp("id", m_Id));
        SafeSerialize(serializer, cereal::make_nvp("email-address", m_EmailAddress));
        SafeSerialize(serializer, cereal::make_nvp("username", m_Username));
        SafeSerialize(serializer, cereal::make_nvp("password", m_Password));
        SafeSerialize(serializer, cereal::make_nvp("admin", m_IsAdmin));
        SafeSerialize(serializer, cereal::make_nvp("gravatar-email-address", m_GravatarEmailAddress));
    }

    void Serialize(bsoncxx::builder::stream::document& document)
    {
        document << "id" << m_Id << "email-address" << m_EmailAddress.c_str() << "username" << m_Username.c_str() << "password" << m_Password << "admin" << m_IsAdmin;
        if (!m_GravatarEmailAddress.empty())
        {
            document << "gravatar-email-address" << m_GravatarEmailAddress;
        }
    }

    bool ValidatePassword(const std::string& unhashedPassword) const;
    void SetPassword(const std::string& password);

    bool m_IsAdmin;
    std::string m_Id;
    std::string m_EmailAddress;
    std::string m_GravatarEmailAddress;
    std::string m_Username;

private:
    std::string m_Password;
};

struct ApiKey final : public MongoDatabaseItem
{
    ApiKey()
        : m_CreationTime(std::chrono::system_clock::now().time_since_epoch().count())
        , m_Expiration(std::numeric_limits<std::chrono::system_clock::rep>::max())
        , m_Permissions(Type::None)
    {
        Generate();
    }

    enum class Type : unsigned int
    {
        None                = 0b0000000,

        ConnectionInfoRead  = 0b0000001,
        ConnecitonInfoWrite = 0b0000010,
        ApiKeyRead          = 0b0000100,
        ApiKeyWrite         = 0b0001000,
        ProfileRead         = 0b0010000,
        ProfileUpdate       = 0b0100000,
        Administration      = 0b1000000,

        All                 = ConnectionInfoRead | ConnecitonInfoWrite | ApiKeyRead | ApiKeyWrite | ProfileRead | ProfileUpdate | Administration,
    };

    template<class TSerializer>
    void Serialize(TSerializer& serializer)
    {
        serializer(cereal::make_nvp("user", m_User), cereal::make_nvp("api-key", m_ApiKey), cereal::make_nvp("creation", m_CreationTime), cereal::make_nvp("expiration", m_Expiration), cereal::make_nvp("permissions", m_PermissionsInteger));
    }

    void Serialize(bsoncxx::builder::stream::document& document)
    {
        document << "user" << m_User.c_str() << "api-key" << m_ApiKey.c_str() << "creation" << m_CreationTime << "expiration" << m_Expiration << "permissions" << m_PermissionsInteger;
    }

    void Generate();

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