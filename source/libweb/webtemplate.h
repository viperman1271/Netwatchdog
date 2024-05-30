#pragma once

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>

#include <filesystem>
#include <sstream>

class WebTemplate
{
public:
    class AutoScope
    {
    public:
        AutoScope(std::stringstream& ss, WebTemplate& _template, const std::vector<std::pair<std::string, std::string>>& pairs);
        AutoScope(std::stringstream& ss, WebTemplate& _template)
            : AutoScope(ss, _template, {})
        {
        }

        AutoScope(const AutoScope&) = delete;
        AutoScope& operator=(const AutoScope&) = delete;

        ~AutoScope();

    private:
        std::stringstream& m_StringStream;
        WebTemplate& m_Template;
        std::vector<std::pair<std::string, std::string>> m_Pairs;
    };

public:
    WebTemplate(const std::filesystem::path& path);
    WebTemplate() = default;

    template<class TSerializer>
    void serialize(TSerializer& serializer)
    {
        serializer(cereal::make_nvp("start", m_Start), cereal::make_nvp("nl-start", m_NewLineAfterStart), cereal::make_nvp("end", m_End), cereal::make_nvp("nl-end", m_NewLineAfterEnd));
    }

    void Write(std::stringstream& ss, const std::vector<std::pair<std::string, std::string>>& pairs);
    void WriteStart(std::stringstream& ss, const std::vector<std::pair<std::string, std::string>>& pairs);
    void WriteEnd(std::stringstream& ss, const std::vector<std::pair<std::string, std::string>>& pairs);

private:
    friend class AutoScope;

    std::string m_Start;
    bool m_NewLineAfterStart;
    std::string m_End;
    bool m_NewLineAfterEnd;
};