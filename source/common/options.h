#pragma once

//#include "config.h"
#include "utils.h"

#include <toml.hpp>

#include <string>
#include <type_traits>

template<class T>
class ConfigurableOption
{
public:
    explicit ConfigurableOption(T value) requires std::is_trivially_constructible_v<T>
        : m_FromEnv(false)
        , m_FromConfig(false)
        , m_FromCommandLine(false)
        , m_DefaultValue(true)
        , m_Value(value)
    {
    }

    explicit ConfigurableOption(const T& value) requires !std::is_trivially_constructible_v<T>
        : m_FromEnv(false)
        , m_FromConfig(false)
        , m_FromCommandLine(false)
        , m_DefaultValue(true)
        , m_Value(value)
    {
    }

    ConfigurableOption()
        : m_FromEnv(false)
        , m_FromConfig(false)
        , m_FromCommandLine(false)
        , m_DefaultValue(false)
    {
    }

    ~ConfigurableOption() = default;

    void Init(T value, const toml::value& config, const std::pair<std::string, std::string>& configPath, const std::string& envVar) requires std::is_trivially_constructible_v<T>
    {
        if (ConfigureIfEnvVarNotEmpty(envVar))
        {
            m_FromEnv = true;
        }
        else if (ValueExists(config, configPath.first, configPath.second))
        {
            const std::string val = config[configPath.first][configPath.second];
            std::istringstream iss(value);
            iss >> m_Value;

            m_FromConfig = true;
        }
        else
        {
            m_Value = value;
            m_DefaultValue = true;
        }
    }

    void Init(const T& value, const toml::value& config, const std::pair<std::string, std::string>& configPath, const std::string& envVar) requires !std::is_trivially_constructible_v<T>
    {
        if (ConfigureIfEnvVarNotEmpty(envVar))
        {
            m_FromEnv = true;
        }
        else if (ValueExists(config, configPath.first, configPath.second))
        {
            const std::string val = config[configPath.first][configPath.second];
            std::istringstream iss(value);
            iss >> m_Value;

            m_FromConfig = true;
        }
        else
        {
            m_Value = value;
            m_DefaultValue = true;
        }
    }

    operator T() const requires std::is_trivially_constructible_v<T>
    {
        return m_Value;
    }

    T operator*() const requires std::is_trivially_constructible_v<T>
    {
        return m_Value;
    }

    operator const T& () const requires !std::is_trivially_constructible_v<T>
    {
        return m_Value;
    }

    operator T& () requires !std::is_trivially_constructible_v<T>
    {
        return m_Value;
    }

    const T& operator*() const requires !std::is_trivially_constructible_v<T>
    {
        return m_Value;
    }

    T& operator*() requires !std::is_trivially_constructible_v<T>
    {
        return m_Value;
    }

private:
    bool ConfigureIfEnvVarNotEmpty(const std::string& envVariable)
    {
        std::string value = Utils::GetEnvVar(envVariable.c_str());
        if (!value.empty())
        {
            std::istringstream iss(value);
            if (iss >> m_Value)
            {
                return true;
            }
        }

        return false;
    }

    static bool ValueExists(toml::value& config, const std::string& category, const std::string& variable);

private:
    bool m_FromEnv;
    bool m_FromConfig;
    bool m_FromCommandLine;
    bool m_DefaultValue;

    T m_Value;
    const std::pair<std::string, std::string> m_ConfigPath;
};

struct Options
{
    struct
    {
        unsigned int count = 1;
        std::string host = "localhost";
        std::string identity;
        int port = 32000;
        bool secure;
    } client;

    struct
    {
        std::string host = "*";
        std::string identity;
        int port = 32000;
        bool secure;
    } server;

    struct
    {
        std::string username;
        std::string password;
        std::string host;
        int port;
    } database;

    struct
    {
        std::string fileServingDir;
        std::string host;
        std::string privateKeyPath;
        std::string publicKeyPath;
        std::string certificatePath;
        int port;
        int securePort;
        bool secure;
    } web;

    struct
    {
        std::string userToCreate;
        std::string userPassword;
        bool direct;
        bool userIsAdmin;
    } admin;
};