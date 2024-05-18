#pragma once

#include "messages.h"

#include <cereal/archives/json.hpp>
#include <zmq.hpp>

namespace Communication
{
    template<class TMessage, MessageType TMessageType>
    std::shared_ptr<TMessage> RecvMessage(zmq::socket_t& recvSocket, bool verbose = false)
    {
        zmq::message_t message;
        if (recvSocket.recv(message))
        {
            std::stringstream ss(std::string(static_cast<char*>(message.data()), message.size()));
            std::shared_ptr<Message> msg = Message::Serialize(ss);
            if (msg->GetType() == TMessageType)
            {
                return std::static_pointer_cast<TMessage>(msg);
            }
            return {};
        }

        return {};
    }

    template<class TMessage>
    void SendMessage(TMessage& message, zmq::socket_t& sendSocket, zmq::send_flags sendFlags = zmq::send_flags::none, bool verbose = false)
    {
        std::stringstream ss;
        {
            cereal::JSONOutputArchive serializer(ss);
            message.serialize(serializer);
        }

        sendSocket.send(zmq::buffer(ss.str()), sendFlags);
        if (verbose)
        {
            std::cout << "Sent: " << ss.str() << std::endl;
        }
    }
}