#pragma once

#include "objectmodel.h"
#include "options.h"

#include <bsoncxx/builder/stream/array.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <nlohmann/json.hpp>

#include <vector>

class Mongo
{
public:
    enum class Database
    {
        Meta,
        Stats,
    };

    enum class Collection
    {
        Connection,
        User,
        ApiKeys,
    };

public:
    Mongo(const Options& options);

    bool IsConnected() const;

    mongocxx::client& GetClient() { return m_Client; }

    void AddConnectionInfo(ConnectionInfo& connInfo);
    void DumpInfo(Database database, Collection collection) const;
    bool DumpClientInfo(const std::string& clientId, std::stringstream& outputStream, std::string lineEnd) const;
    bool FetchClientInfo(const std::string& clientId, std::vector<ConnectionInfo>& connInfo) const;
    void DeleteInfo(Database database, Collection collection, const std::string& clientId);

    void CreateUser(User& user);
    bool UpdateUser(const User& user);
    bool FetchUser(const std::string& username, User& user);

    void CreateApiKey(ApiKey& apiKey);
    void FetchApiKeys(const std::string& userId, std::vector<ApiKey>& apiKeys) const;

private:
    static std::string GetDatabaseName(Database database);
    static std::string GetCollectionName(Collection collection);

    bool DatabaseExists(const std::string& database) const;
    bool CollectionExists(mongocxx::database& database, const std::string& collectionName) const;

    static nlohmann::json DiffJson(const std::stringstream& oldJson, const std::stringstream& newJson);
    static nlohmann::json DiffJson(const nlohmann::json& oldJson, const nlohmann::json& newJson);
    static void Serialize(bsoncxx::builder::stream::array& array, const nlohmann::json& it);
    static void Serialize(bsoncxx::builder::stream::document& updateBuilder, const nlohmann::json& it);

private:
    static mongocxx::instance ms_Instance;

    mongocxx::uri m_Uri;
    mongocxx::client m_Client;
};
