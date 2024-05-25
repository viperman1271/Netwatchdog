#pragma once

#include <bsoncxx/builder/stream/document.hpp>
#include <cereal/cereal.hpp>

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
    std::chrono::system_clock ::duration::rep m_Time;
};