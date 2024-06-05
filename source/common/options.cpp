#include "options.h"

template<class T>
bool ConfigurableOption<T>::ValueExists(toml::value& config, const std::string& category, const std::string& variable)
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