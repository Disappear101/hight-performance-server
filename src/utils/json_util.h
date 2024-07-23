#ifndef __TAO_JSON_UTIL_H__
#define __TAO_JSON_UTIL_H__

#include <string>
#include <jsoncpp/json/json.h>

namespace tao {

class JsonUtil {
public:
    /// @brief check existence of Escapes
    /// @param v string
    /// @return true/false
    static bool NeedEscape(const std::string& v);

    /// @brief escape 
    /// @param v string
    /// @return escaped string
    static std::string Escape(const std::string& v);

    /// @brief  retrieves a string value from a JSON object
    /// @param json the JSON object to retrieve the value from.
    /// @param name the key for which the value is to be retrieved.
    /// @param default_value  the value to return if the key is not found
    /// @return The string value associated with name in json
    static std::string GetString(const Json::Value& json
                          ,const std::string& name
                          ,const std::string& default_value = "");

    /// @brief retrieves a double value from a JSON object
    /// @param json the JSON object to retrieve the value from.
    /// @param name the key for which the value is to be retrieved.
    /// @param default_value the value to return if the key is not found
    /// @return The double value associated with name in json
    static double GetDouble(const Json::Value& json
                     ,const std::string& name
                     ,double default_value = 0);

    /// @brief retrieves a int32 value from a JSON object
    /// @param json the JSON object to retrieve the value from.
    /// @param name the key for which the value is to be retrieved.
    /// @param default_value the value to return if the key is not found
    /// @return The int32 value associated with name in json
    static int32_t GetInt32(const Json::Value& json
                     ,const std::string& name
                     ,int32_t default_value = 0);

    /// @brief retrieves a uint32 value from a JSON object
    /// @param json the JSON object to retrieve the value from.
    /// @param name the key for which the value is to be retrieved.
    /// @param default_value the value to return if the key is not found
    /// @return The double value associated with name in json
    static uint32_t GetUint32(const Json::Value& json
                       ,const std::string& name
                       ,uint32_t default_value = 0);

    /// @brief retrieves a int64 value from a JSON object
    /// @param json the JSON object to retrieve the value from.
    /// @param name the key for which the value is to be retrieved.
    /// @param default_value the value to return if the key is not found
    /// @return The int64 value associated with name in json
    static int64_t GetInt64(const Json::Value& json
                     ,const std::string& name
                     ,int64_t default_value = 0);

    /// @brief retrieves a uint64 value from a JSON object
    /// @param json the JSON object to retrieve the value from.
    /// @param name the key for which the value is to be retrieved.
    /// @param default_value the value to return if the key is not found
    /// @return The uint64 value associated with name in json
    static uint64_t GetUint64(const Json::Value& json
                       ,const std::string& name
                       ,uint64_t default_value = 0);

    /// @brief parses a JSON string v and stores the result in a JSON object json.
    /// @param json the JSON object to store the parsed result.
    /// @param v the JSON string to parse
    /// @return true/false
    static bool FromString(Json::Value& json, const std::string& v);

    /// @brief converts a JSON object json to its string representation.
    /// @param json the JSON object to convert.
    /// @return A string representation of the JSON object.
    static std::string ToString(const Json::Value& json);
};

}

#endif