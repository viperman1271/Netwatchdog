#include "messages.h"

#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>

template<class T>
std::shared_ptr<Message> SerializeMessage(const std::stringstream& stream)
{
    std::stringstream ss(stream.str());
    T* msg = new T();
    cereal::JSONInputArchive inputSerializer(ss);
    msg->serialize(inputSerializer);
    return std::shared_ptr<Message>(msg);
}

std::shared_ptr<Message> Message::Serialize(const std::stringstream& stream)
{
    MessageType type;
    {
        std::stringstream ss(stream.str());
        cereal::JSONInputArchive inputSerializer(ss);
        inputSerializer(CEREAL_NVP(type));
    }

    switch (type)
    {
    case MessageType::GenericMessage:
        return SerializeMessage<GenericMessage>(stream);

    case MessageType::Heartbeat:
        return SerializeMessage<HeartbeatMessage>(stream);

    default:
        assert(false);
    };

    return {};
}

template<class T>
void SerializeMessage(std::shared_ptr<Message> message, std::stringstream& ss)
{
    std::shared_ptr<Message> messageType = std::static_pointer_cast<T>(message);

    cereal::JSONOutputArchive inputSerializer(ss);
    messageType->serialize(inputSerializer);
}

void Message::Serialize(std::shared_ptr<Message> message, std::stringstream& ss)
{
    switch (message->GetType())
    {
    case MessageType::GenericMessage:
        return SerializeMessage<GenericMessage>(message, ss);

    case MessageType::Heartbeat:
        return SerializeMessage<HeartbeatMessage>(message, ss);

    default:
        assert(false);
    };
}