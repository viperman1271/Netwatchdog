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
    template<class T>
    class CommandLineOptionWrapper
    {
    public:
        CommandLineOptionWrapper(ConfigurableOption<T>& option)
            : m_Option(option)
        {
        }

        operator T() const requires std::is_trivially_constructible_v<T>
        {
            return m_Option;
        }

        T operator*() const requires std::is_trivially_constructible_v<T>
        {
            return m_Option;
        }

        operator const T& () const requires (!std::is_trivially_constructible_v<T>)
        {
            return m_Option;
        }

        operator T& () requires (!std::is_trivially_constructible_v<T>)
        {
            return m_Option;
        }

        const T& operator*() const requires (!std::is_trivially_constructible_v<T>)
        {
            return m_Option;
        }

        T& operator*() requires (!std::is_trivially_constructible_v<T>)
        {
            return m_Option;
        }

        T* operator->()
        {
            return m_Option;
        }

        const T* operator->() const
        {
            return m_Option;
        }

        CommandLineOptionWrapper<T>& operator=(T value) requires std::is_trivially_constructible_v<T>
        {
            m_Option.SetFromCommandLine(true);

            m_Option = value;
            return *this;
        }

        CommandLineOptionWrapper<T>& operator=(const T& value) requires (!std::is_trivially_constructible_v<T>)
        {
            m_Option.SetFromCommandLine(true);

            m_Option = value;
            return *this;
        }

    private:
        ConfigurableOption<T>& m_Option;
    };

public:
    explicit ConfigurableOption(T value) requires std::is_trivially_constructible_v<T>
        : m_FromEnv(false)
        , m_FromConfig(false)
        , m_FromCommandLine(false)
        , m_DefaultValue(true)
        , m_Value(value)
        , m_OptionWrapper(*this)
    {
    }

    explicit ConfigurableOption(const T& value) requires (!std::is_trivially_constructible_v<T>)
        : m_FromEnv(false)
        , m_FromConfig(false)
        , m_FromCommandLine(false)
        , m_DefaultValue(true)
        , m_Value(value)
        , m_OptionWrapper(*this)
    {
    }

    ConfigurableOption()
        : m_FromEnv(false)
        , m_FromConfig(false)
        , m_FromCommandLine(false)
        , m_DefaultValue(false)
        , m_OptionWrapper(*this)
    {
    }

    ~ConfigurableOption() = default;

    void Init(toml::value& config, const std::pair<std::string, std::string>& configPath, const std::string& envVar = {}) requires std::is_trivially_constructible_v<T>
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

            config[configPath.first][configPath.second] = m_Value;
        }
        else
        {
            config[configPath.first][configPath.second] = m_Value;
        }
    }

    void Init(toml::value& config, const std::pair<std::string, std::string>& configPath, const std::string& envVar = {}) requires (!std::is_trivially_constructible_v<T>)
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

            config[configPath.first][configPath.second] = m_Value;
        }
        else
        {
            config[configPath.first][configPath.second] = m_Value;
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

    operator const T& () const requires (!std::is_trivially_constructible_v<T>)
    {
        return m_Value;
    }

    operator T& () requires (!std::is_trivially_constructible_v<T>)
    {
        return m_Value;
    }

    const T& operator*() const requires (!std::is_trivially_constructible_v<T>)
    {
        return m_Value;
    }

    T& operator*() requires (!std::is_trivially_constructible_v<T>)
    {
        return m_Value;
    }

    ConfigurableOption& operator=(T value) requires std::is_trivially_constructible_v<T>
    {
        m_Value = value;
        return *this;
    }

    ConfigurableOption& operator=(const T& value) requires (!std::is_trivially_constructible_v<T>)
    {
        m_Value = value;
        return *this;
    }

    ConfigurableOption& operator=(const ConfigurableOption<T>& rhs)
    {
        m_Value = rhs.m_Value;
        return *this;
    }

    bool operator==(const ConfigurableOption<T>& rhs) const
    {
        return false;
    }

    bool operator==(ConfigurableOption<T>& rhs)
    {
        return false;
    }

    bool operator==(const T value) const requires std::is_trivially_constructible_v<T>
    {
        return m_Value == value;
    }

    bool operator==(const T& value) const requires (!std::is_trivially_constructible_v<T>)
    {
        return m_Value == value;
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

    bool GetFromEnv() const { return m_FromEnv; }
    bool GetFromConfig() const { return m_FromConfig; }
    bool GetFromCommandLine() const { return m_FromCommandLine; }
    bool GetDefaultValue() const { return m_DefaultValue; }

    void SetFromCommandLine(bool value) { m_FromCommandLine = value; }

    CommandLineOptionWrapper<T>& GetCommandLine() { return m_OptionWrapper; }

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

    bool ValueExists(toml::value& config, const std::string& category, const std::string& variable)
    {
        if (config.type() != toml::value_t::empty)
        {
            auto& tab = config.as_table();
            if (tab.count(category) != 0)
            {
                auto& subtab = config[category].as_table();
                if (subtab.count(variable) != 0)
                {
                    return true;
                }
            }
        }

        return false;
    }

    void SetValueCallback()
    {

    }

private:
    bool m_FromEnv;
    bool m_FromConfig;
    bool m_FromCommandLine;
    bool m_DefaultValue;

    CommandLineOptionWrapper<T> m_OptionWrapper;

    T m_Value;
    const std::pair<std::string, std::string> m_ConfigPath;
};