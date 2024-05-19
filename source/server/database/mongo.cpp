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

        if (result.view()["ok"].get_double() == 1.0)
        {
            return true;
        }
    }
    catch (const mongocxx::exception& e)
    {
        std::cerr << e.what();
    }

    return false;
}

void Mongo::AddConnectionInfo(ConnectionInfo& connInfo)
{
    const std::string databaseStr = std::move(GetDatabaseName(Database::Stats));
    const std::string collectionStr = std::move(GetCollectionName(Collection::ConnectionInfo));

    mongocxx::database db = m_Client["netwatchdog-stats"];
    mongocxx::collection coll = db["conn-info"];

    bsoncxx::builder::stream::document document{};
    connInfo.serialize(document);

    bsoncxx::document::value doc_value = document << bsoncxx::builder::stream::finalize;
    coll.insert_one(doc_value.view());
}

void Mongo::DumpInfo(Database database, Collection collection)
{
    const std::string databaseStr = std::move(GetDatabaseName(database));
    if (!DatabaseExists(databaseStr))
    {
        std::cout << "Database " << databaseStr << " does not exist." << std::endl;
        return;
    }

    const std::string collectionStr = std::move(GetCollectionName(collection));
    if (!CollectionExists(collectionStr))
    {
        std::cout << "Collection " << collectionStr << " does not exist in " << databaseStr << "." << std::endl;
        return;
    }

    mongocxx::database db = m_Client[databaseStr.c_str()];
    mongocxx::collection coll = db[collectionStr.c_str()];

    mongocxx::cursor cursor = coll.find({});
    for (bsoncxx::document::view view : cursor)
    {
        std::cout << Utils::BSON::ToJSON(view) << std::endl;
    }
}

std::string Mongo::GetDatabaseName(Database database)
{
    switch (database)
    {
    case Database::Meta:
        return "netwatchdog-meta";

    case Database::Stats:
        return "netwatchdog-stats";
    };

    return {};
}

std::string Mongo::GetCollectionName(Collection collection)
{
    switch (collection)
    {
    case Collection::ConnectionInfo:
        return "conn-info";
    };

    return {};
}

bool Mongo::DatabaseExists(const std::string& databaseName) const
{
    try 
    {
        mongocxx::cursor databases = m_Client.list_databases();
        for (bsoncxx::document::view db : databases)
        {
            std::string name(db["name"].get_string());
            if (name == databaseName)
            {
                return true;
            }
        }
    }
    catch (const mongocxx::exception& e) 
    {
        std::cerr << "Error listing databases: " << e.what() << std::endl;
    }
    return false;
}

bool Mongo::CollectionExists(mongocxx::database& database, const std::string& collectionName) const
{
    try 
    {
        mongocxx::cursor collections = database.list_collections();
        for (bsoncxx::document::view coll : collections)
        {
            std::string name(coll["name"].get_string());
            if (name == collectionName)
            {
                return true;
            }
        }
    }
    catch (const mongocxx::exception& e) 
    {
        std::cerr << "Error listing collections: " << e.what() << std::endl;
    }
    return false;
}
