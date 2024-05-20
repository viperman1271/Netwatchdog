#include "bson_utils.h"

#include <bson/bson.h>

namespace Utils::BSON
{
bool ToJSON(bsoncxx::document::view view, std::string& str)
{
    bson_t bson;
    bson_init_static(&bson, view.data(), view.length());

    size_t size;
    char* result = bson_as_json(&bson, &size);

    if (!result)
    {
        return false;
    }

    str = std::string{ result, size };

    bson_free(result);

    return true;
}

std::string ToJSON(bsoncxx::document::view view)
{
    bson_t bson;
    bson_init_static(&bson, view.data(), view.length());

    size_t size;
    char* result = bson_as_json(&bson, &size);

    if (!result)
    {
        return {};
    }

    const auto deleter = [](char* result) { bson_free(result); };
    const std::unique_ptr<char[], decltype(deleter)> cleanup(result, deleter);

    return { result, size };
}

std::string OIDToStr(bsoncxx::oid oid)
{
    bson_oid_t _oid;
    std::memcpy(_oid.bytes, oid.bytes(), oid.size());
    char str[25];

    bson_oid_to_string(&_oid, str);

    return std::string(str);
}
}