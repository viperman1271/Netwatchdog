#pragma once

#include "utils.h"

#include <toml.hpp>
#include <stduuid/uuid.h>

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

    void Init(const toml::value& config, const std::pair<std::string, std::string>& configPath, const std::string& envVar = {}) requires std::is_trivially_constructible_v<T>
    {
        if (ConfigureIfEnvVarNotEmpty(envVar))
        {
            m_FromEnv = true;
            m_DefaultValue = false;
        }
        else if (ValueExists(config, configPath.first, configPath.second))
        {
            m_Value = toml::find<T>(config, configPath.first, configPath.second);

            m_FromConfig = true;
            m_DefaultValue = false;
        }
    }

    void Init(const toml::value& config, const std::pair<std::string, std::string>& configPath, const std::string& envVar = {}) requires !std::is_trivially_constructible_v<T>
    {
        if (ConfigureIfEnvVarNotEmpty(envVar))
        {
            m_FromEnv = true;
            m_DefaultValue = false;
        }
        else if (ValueExists(config, configPath.first, configPath.second))
        {
            m_Value = toml::find<T>(config, configPath.first, configPath.second);

            m_FromConfig = true;
            m_DefaultValue = false;
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

    ConfigurableOption& operator=(T value) requires std::is_trivially_constructible_v<T>
    {
        m_Value = value;
        return *this;
    }

    ConfigurableOption& operator=(const T& value) requires !std::is_trivially_constructible_v<T>
    {
        m_Value = value;
        return *this;
    }

    ConfigurableOption& operator=(const ConfigurableOption<T>& rhs)
    {
        m_Value = rhs.m_Value;
        return *this;
    }

    T* operator->()
    {
        return &m_Value;
    }

    const T* operator->() const
    {
        return &m_Value;
    }

    friend std::ostream& operator<<(std::ostream& os, const ConfigurableOption<T>& option)
    {
        os << option.m_Value;
        return os;
    }

private:
    bool ConfigureIfEnvVarNotEmpty(const std::string& envVariable)
    {
        if (envVariable.empty())
        {
            return false;
        }

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

    bool ValueExists(const toml::value& config, const std::string& category, const std::string& variable)
    {
        if (config.type() != toml::value_t::empty)
        {
            auto& tab = config.as_table();
            if (tab.count(category) != 0)
            {
                auto& subtab = const_cast<toml::value&>(config)[category].as_table();
                if (subtab.count(variable) != 0)
                {
                    return true;
                }
            }
        }

        return false;
    }

private:
    bool m_FromEnv;
    bool m_FromConfig;
    bool m_FromCommandLine;
    bool m_DefaultValue;

    T m_Value;
    const std::pair<std::string, std::string> m_ConfigPath;
};