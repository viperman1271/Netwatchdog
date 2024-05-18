#pragma once

#include <cereal/cereal.hpp>

enum class MessageType : unsigned int
{
    GenericMessage,
    Heartbeat,
};

class Message
{
public:
    static std::shared_ptr<Message> Serialize(const std::stringstream& stream);
    static void Serialize(std::shared_ptr<Message> message, std::stringstream& ss);

    virtual MessageType GetType() const = 0;

    template<class TSerializer>
    void serialize(TSerializer& serializer)
    {
        MessageType type = GetType();
        serializer(CEREAL_NVP(type));
    }
};

class GenericMessage final : public Message
{
    using super = Message;

public:
    MessageType GetType() const override { return MessageType::GenericMessage; }

    bool GetSuccess() const { return m_Success; }
    void SetSuccess(bool value) { m_Success = value; }

    const std::string& GetId() const { return m_Id; }
    void SetId(const std::string& value) { m_Id = value; }

    template<class TSerializer>
    void serialize(TSerializer& serializer)
    {
        super::serialize(serializer);
        serializer(CEREAL_NVP(m_Success), CEREAL_NVP(m_Id));
    }

private:
    bool m_Success{ false };
    std::string m_Id;
};

class HeartbeatMessage final : public Message
{
    using super = Message;

public:
    MessageType GetType() const override { return MessageType::Heartbeat; }

    template<class TSerializer>
    void serialize(TSerializer& serializer)
    {
        super::serialize(serializer);
    }
};