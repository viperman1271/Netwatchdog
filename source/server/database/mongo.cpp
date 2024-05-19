#include "database/mongo.h"

#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/builder/stream/document.hpp>

mongocxx::instance Mongo::ms_Instance{};

Mongo::Mongo(const Options& options)
{
    std::stringstream ss;
    if (!options.database.username.empty() && !options.database.password.empty())
    {
        ss << "mongodb://" << options.database.username << ":" << options.database.password << "@" << options.database.host << ":" << options.database.port;
    }
    else
    {
        ss << "mongodb://" << options.database.host << ":" << options.database.port;
    }

    m_Uri = mongocxx::uri(ss.str().c_str());
    m_Client = mongocxx::client(m_Uri);

    if (!IsConnected())
    {
        return;
    }
}

bool Mongo::IsConnected() const
{
    try
    {
        mongocxx::database admin_db = m_Client["admin"];
        bsoncxx::builder::stream::document ping_cmd{};
        ping_cmd << "ping" << 1;

        bsoncxx::document::value result = admin_db.run_command(ping_cmd.view());

        // Check the result of the ping command
        if (result.view()["ok"].get_double() == 1.0)
        {
            return true;
        }
    }
    catch (const mongocxx::exception& e)
    {
        std::cerr << e.what();
        // Connection failed or ping command failed
    }

    return false;
}
