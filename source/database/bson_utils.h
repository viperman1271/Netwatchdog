#pragma once

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>

#include <string>

namespace Utils::BSON
{
    bool ToJSON(bsoncxx::document::view view, std::string& str);
    std::string ToJSON(bsoncxx::document::view view);

    std::string OIDToStr(bsoncxx::oid oid);
}