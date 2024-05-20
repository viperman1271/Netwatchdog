#pragma once

#include "objectmodel.h"
#include "options.h"

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

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
        ConnectionInfo
    };

public:
    Mongo(const Options& options);

    bool IsConnected() const;

    mongocxx::client& GetClient() { return m_Client; }

    void AddConnectionInfo(ConnectionInfo& connInfo);
    void DumpInfo(Database database, Collection collection);

private:
    static std::string GetDatabaseName(Database database);
    static std::string GetCollectionName(Collection collection);

    bool DatabaseExists(const std::string& database) const;
    bool CollectionExists(mongocxx::database& database, const std::string& collectionName) const;

private:
    static mongocxx::instance ms_Instance;

    mongocxx::uri m_Uri;
    mongocxx::client m_Client;
};