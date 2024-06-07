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
    template<class T1>
    class CommandLineOptionWrapper
    {
    public:
        CommandLineOptionWrapper(ConfigurableOption<T1>& option, std::function<void()> callback)
            : m_Option(option)
            , m_SetValueCallback(callback)
        {
        }

        operator T1() const requires std::is_trivially_constructible_v<T>
        {
            return m_Option;
        }

        T1 operator*() const requires std::is_trivially_constructible_v<T>
        {
            return m_Option;
        }

        operator const T1& () const requires (!std::is_trivially_constructible_v<T>)
        {
            return m_Option;
        }

        operator T1& () requires (!std::is_trivially_constructible_v<T>)
        {
            return m_Option;
        }

        const T1& operator*() const requires (!std::is_trivially_constructible_v<T>)
        {
            return m_Option;
        }

        T1& operator*() requires (!std::is_trivially_constructible_v<T>)
        {
            return m_Option;
        }

        T1* operator->()
        {
            return m_Option;
        }

        const T1* operator->() const
        {
            return m_Option;
        }

        CommandLineOptionWrapper<T1>& operator=(T1 value) requires std::is_trivially_constructible_v<T>
        {
            if (m_SetValueCallback)
            {
                m_SetValueCallback();
            }

            m_Option = value;
            return *this;
        }

        CommandLineOptionWrapper<T1>& operator=(const T1& value) requires (!std::is_trivially_constructible_v<T>)
        {
            if (m_SetValueCallback)
            {
                m_SetValueCallback();
            }
            

            m_Option = value;
            return *this;
        }

    private:
        ConfigurableOption<T1>& m_Option;
        std::function<void()> m_SetValueCallback;
    };

public:
    explicit ConfigurableOption(T value) requires std::is_trivially_constructible_v<T>
        : m_FromEnv(false)
        , m_FromConfig(false)
        , m_FromCommandLine(false)
        , m_FromDefaultValue(true)
        , m_Value(value)
        , m_OptionWrapper(*this, [this]() { SetValueCallback(); })
    {
    }

    explicit ConfigurableOption(const T& value) requires (!std::is_trivially_constructible_v<T>)
        : m_FromEnv(false)
        , m_FromConfig(false)
        , m_FromCommandLine(false)
        , m_FromDefaultValue(true)
        , m_Value(value)
        , m_OptionWrapper(*this, [this]() { SetValueCallback(); })
    {
    }

    ConfigurableOption()
        : m_FromEnv(false)
        , m_FromConfig(false)
        , m_FromCommandLine(false)
        , m_FromDefaultValue(false)
        , m_OptionWrapper(*this, [this]() { SetValueCallback(); })
    {
    }

    ~ConfigurableOption() = default;

    void Init(toml::value& config, const std::pair<std::string, std::string>& configPath, const std::string& envVar = {}) requires std::is_trivially_constructible_v<T>
    {
        if (ConfigureIfEnvVarNotEmpty(envVar))
        {
            m_FromEnv = true;
            m_FromDefaultValue = false;
        }
        else if (ValueExists(config, configPath.first, configPath.second))
        {
            m_Value = toml::find<T>(config, configPath.first, configPath.second);

            m_FromConfig = true;
            m_FromDefaultValue = false;

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
            m_FromDefaultValue = false;
        }
        else if (ValueExists(config, configPath.first, configPath.second))
        {
            m_Value = toml::find<T>(config, configPath.first, configPath.second);

            m_FromConfig = true;
            m_FromDefaultValue = false;

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
        return m_Value == rhs.m_Value;
    }

    bool operator==(ConfigurableOption<T>& rhs)
    {
        return m_Value == rhs.m_Value;
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
    bool GetFromDefaultValue() const { return m_FromDefaultValue; }

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
        m_FromCommandLine = true;
        m_FromDefaultValue = false;
    }

private:
    bool m_FromEnv;
    bool m_FromConfig;
    bool m_FromCommandLine;
    bool m_FromDefaultValue;

    CommandLineOptionWrapper<T> m_OptionWrapper;

    T m_Value;
    const std::pair<std::string, std::string> m_ConfigPath;
};