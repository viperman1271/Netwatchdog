#include "database/mongo.h"

#include "bson_utils.h"

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

void Mongo::Test()
{
    //3 DB
    //Users
    //ConnInfo
    //API Keys

    // Access a specific database
    mongocxx::database db = m_Client["testdb"]; // Replace with your database name

    // Access a specific collection
    mongocxx::collection coll = db["testcollection"]; // Replace with your collection name

    // Insert a document into the collection
    bsoncxx::builder::stream::document document{};
    document << "name" << "John Doe"
        << "age" << 30
        << "occupation" << "Software Engineer";

    bsoncxx::document::value doc_value = document << bsoncxx::builder::stream::finalize;
    //coll.insert_one(doc_value.view());

    std::string docStr = Utils::BSON::ToJSON(doc_value.view());

    // Find and print a document from the collection
    mongocxx::cursor cursor = coll.find({});
    for (bsoncxx::document::view view : cursor)
    {
        bsoncxx::oid oid = view["_id"].get_oid().value;
        std::string oidStr = Utils::BSON::OIDToStr(oid);
        std::cout << "Extracted _id: " << oidStr << std::endl;

        std::cout << Utils::BSON::ToJSON(view) << std::endl;
    }

    // Delete multiple documents from the collection
//     bsoncxx::builder::stream::document delete_many_filter{};
//     delete_many_filter << "age" << bsoncxx::builder::stream::open_document << "$gte" << 30 << bsoncxx::builder::stream::close_document;
//     coll.delete_many(delete_many_filter.view());
}
