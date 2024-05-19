#pragma once

#include "options.h"

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

class Mongo
{
public:
    Mongo(const Options& options);

    bool IsConnected() const;

    mongocxx::client& GetClient() { return m_Client; }

    void AddConnectionInfo();

    void Test();

private:
    static mongocxx::instance ms_Instance;

    mongocxx::uri m_Uri;
    mongocxx::client m_Client;
};
