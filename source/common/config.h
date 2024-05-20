#pragma once

#include "options.h"
#include "utils.h"

#include <CLI/CLI.hpp>
#include <stduuid/uuid.h>
#include <toml.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#include <filesystem>
#include <iostream>

namespace Config
{
    bool ValueExists(toml::value& config, const std::string& category, const std::string& variable);
    void ConfigureIfEnvVarNotEmpty(toml::value& config, const std::string& category, const std::string& variable, const std::string& envVariable);
    bool ValidateIdentity(toml::value& config, const std::string& category, const std::string& variable);

    template<typename T>
    void ConfigureDefaultValue(toml::value& config, const std::string& category, const std::string& variable, const T& defaultValue)
    {
        if (!ValueExists(config, category, variable))
        {
            config[category][variable] = defaultValue;
        }
    }

    template<typename T>
    void GetValue(toml::value& config, const std::string& category, const std::string& variable, T& value)
    {
        if (config.type() != toml::value_t::empty)
        {
            auto& tab = config.as_table();
            if (tab.count(category) != 0)
            {
                auto& subtab = config[category].as_table();
                if (subtab.count(variable) != 0)
                {
                    value = toml::find<T>(config, category, variable);
                }
            }
        }
    }

    void LoadOrCreateConfig(Options& options);
    bool ParseCommandLineOptions(int argc, char** argv, Options& options);
};