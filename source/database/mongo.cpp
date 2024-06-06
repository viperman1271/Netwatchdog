#include "mongo.h"

#include "bson_utils.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <mongocxx/exception/exception.hpp>

#include <sstream>

mongocxx::instance Mongo::ms_Instance{};

Mongo::Mongo(const Options& options)
{
    std::stringstream ss;
    if (!options.database.username->empty() && !options.database.password->empty())
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
    const std::string collectionStr = std::move(GetCollectionName(Collection::Connection));

    mongocxx::database db = m_Client[databaseStr.c_str()];
    mongocxx::collection coll = db[collectionStr.c_str()];

    bsoncxx::builder::stream::document document{};
    connInfo.serialize(document);

    bsoncxx::document::value doc_value = document << bsoncxx::builder::stream::finalize;
    coll.insert_one(doc_value.view());
}

void Mongo::DumpInfo(Database database, Collection collection) const
{
    const std::string databaseStr = std::move(GetDatabaseName(database));
    if (!DatabaseExists(databaseStr))
    {
        std::cout << "Database " << databaseStr << " does not exist." << std::endl;
        return;
    }

    mongocxx::database db = m_Client[databaseStr.c_str()];

    const std::string collectionStr = std::move(GetCollectionName(collection));
    if (!CollectionExists(db, collectionStr))
    {
        std::cout << "Collection " << collectionStr << " does not exist in " << databaseStr << "." << std::endl;
        return;
    }

    mongocxx::collection coll = db[collectionStr.c_str()];

    mongocxx::cursor cursor = coll.find({});
    for (bsoncxx::document::view view : cursor)
    {
        std::cout << Utils::BSON::ToJSON(view) << std::endl;
    }
}

bool Mongo::DumpClientInfo(const std::string& clientInfo, std::stringstream& outputStream, std::string lineEnd) const
{
    const std::string databaseStr = std::move(GetDatabaseName(Database::Stats));
    if (!DatabaseExists(databaseStr))
    {
        return false;
    }

    mongocxx::database database = m_Client[databaseStr.c_str()];

    std::string collectionStr = GetCollectionName(Collection::Connection);
    if (!CollectionExists(database, collectionStr))
    {
        return false;
    }

    mongocxx::collection collection = database[collectionStr.c_str()];

    std::function<mongocxx::cursor(const std::string&)> find = [&collection](const std::string& clientInfo)
    {
        if (!clientInfo.empty())
        {
            bsoncxx::builder::stream::document document{};
            document << "unique-id" << clientInfo;

            return collection.find(document.view());
        }
        else
        {
            return collection.find({});
        }
    };

    mongocxx::cursor cursor = find(clientInfo);
    const bool anyResults = cursor.begin() != cursor.end();
    for (bsoncxx::document::view view : cursor)
    {
        outputStream << Utils::BSON::ToJSON(view) << lineEnd;
    }

    return anyResults;
}

bool Mongo::FetchClientInfo(const std::string& clientId, std::vector<ConnectionInfo>& connInfo) const
{
    const std::string databaseStr = std::move(GetDatabaseName(Database::Stats));
    if (!DatabaseExists(databaseStr))
    {
        return false;
    }

    mongocxx::database database = m_Client[databaseStr.c_str()];

    std::string collectionStr = GetCollectionName(Collection::Connection);
    if (!CollectionExists(database, collectionStr))
    {
        return false;
    }

    mongocxx::collection collection = database[collectionStr.c_str()];

    std::function<mongocxx::cursor(const std::string&)> find = [&collection](const std::string& clientInfo)
    {
        if (!clientInfo.empty())
        {
            bsoncxx::builder::stream::document document{};
            document << "unique-id" << clientInfo;

            return collection.find(document.view());
        }
        else
        {
            return collection.find({});
        }
    };

    mongocxx::cursor cursor = find(clientId);
    const bool anyResults = cursor.begin() != cursor.end();
    for (bsoncxx::document::view view : cursor)
    {
        const std::string json = Utils::BSON::ToJSON(view);
        std::stringstream ss(json);
        cereal::JSONInputArchive inputSerializer(ss);

        ConnectionInfo info;
        info.serialize(inputSerializer);

        connInfo.push_back(std::move(info));
    }

    return anyResults;
}

void Mongo::DeleteInfo(Database db, Collection coll, const std::string& clientId)
{
    if (clientId.empty())
    {
        return;
    }

    const std::string databaseStr = std::move(GetDatabaseName(db));
    if (!DatabaseExists(databaseStr))
    {
        return;
    }

    mongocxx::database database = m_Client[databaseStr.c_str()];

    std::string collectionStr = GetCollectionName(coll);
    if (!CollectionExists(database, collectionStr))
    {
        return;
    }

    mongocxx::collection collection = database[collectionStr.c_str()];

    bsoncxx::builder::stream::document filter_builder;
    filter_builder << "unique-id" << clientId;

    mongocxx::stdx::optional<mongocxx::result::delete_result> result = collection.delete_many(filter_builder.view());
}

void Mongo::CreateUser(User& user)
{
    const std::string databaseStr = std::move(GetDatabaseName(Database::Meta));
    const std::string collectionStr = std::move(GetCollectionName(Collection::User));

    mongocxx::database db = m_Client[databaseStr.c_str()];
    mongocxx::collection coll = db[collectionStr.c_str()];

    bsoncxx::builder::stream::document document{};
    user.serialize(document);

    bsoncxx::document::value doc_value = document << bsoncxx::builder::stream::finalize;
    coll.insert_one(doc_value.view());
}

bool Mongo::FetchUser(const std::string& username, User& user)
{
    if (username.empty())
    {
        return false;
    }

    const std::string databaseStr = std::move(GetDatabaseName(Database::Meta));
    if (!DatabaseExists(databaseStr))
    {
        return false;
    }

    mongocxx::database database = m_Client[databaseStr.c_str()];

    std::string collectionStr = GetCollectionName(Collection::User);
    if (!CollectionExists(database, collectionStr))
    {
        return false;
    }

    mongocxx::collection collection = database[collectionStr.c_str()];

    bsoncxx::builder::stream::document document;
    document << "username" << username;

    mongocxx::cursor cursor = collection.find(document.view());
    const bool anyResults = cursor.begin() != cursor.end();
    for (bsoncxx::document::view view : cursor)
    {
        const std::string json = Utils::BSON::ToJSON(view);
        std::stringstream ss(json);
        cereal::JSONInputArchive inputSerializer(ss);

        user.serialize(inputSerializer);
    }

    return anyResults;
}

void Mongo::CreateApiKey(ApiKey& apiKey)
{
    const std::string databaseStr = std::move(GetDatabaseName(Database::Meta));
    const std::string collectionStr = std::move(GetCollectionName(Collection::ApiKeys));

    mongocxx::database db = m_Client[databaseStr.c_str()];
    mongocxx::collection coll = db[collectionStr.c_str()];

    bsoncxx::builder::stream::document document{};
    apiKey.serialize(document);

    bsoncxx::document::value doc_value = document << bsoncxx::builder::stream::finalize;
    coll.insert_one(doc_value.view());
}

void Mongo::FetchApiKeys(const std::string& userId, std::vector<ApiKey>& apiKeys) const
{
    if (userId.empty())
    {
        return;
    }

    const std::string databaseStr = std::move(GetDatabaseName(Database::Meta));
    if (!DatabaseExists(databaseStr))
    {
        return;
    }

    mongocxx::database database = m_Client[databaseStr.c_str()];

    std::string collectionStr = GetCollectionName(Collection::ApiKeys);
    if (!CollectionExists(database, collectionStr))
    {
        return;
    }

    mongocxx::collection collection = database[collectionStr.c_str()];

    bsoncxx::builder::stream::document document;
    document << "user" << userId;

    mongocxx::cursor cursor = collection.find(document.view());
    const bool anyResults = cursor.begin() != cursor.end();
    for (bsoncxx::document::view view : cursor)
    {
        const std::string json = Utils::BSON::ToJSON(view);
        std::stringstream ss(json);
        cereal::JSONInputArchive inputSerializer(ss);

        ApiKey apiKey;
        apiKey.serialize(inputSerializer);

        apiKeys.push_back(std::move(apiKey));
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
    case Collection::Connection:
        return "conn-info";

    case Collection::User:
        return "user";

    case Collection::ApiKeys:
        return "api-keys";
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
