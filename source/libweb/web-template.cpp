#include "web-template.h"

#include "utils.h"

#include <fstream>

WebTemplate::AutoScope::AutoScope(std::stringstream& ss, WebTemplate& _template, const std::vector<std::pair<std::string, std::string>>& pairs) 
    : m_StringStream(ss)
    , m_Template(_template)
    , m_Pairs(pairs)
{
    m_Template.WriteStart(m_StringStream, m_Pairs);
}

WebTemplate::AutoScope::~AutoScope()
{
    m_Template.WriteEnd(m_StringStream, m_Pairs);
}

WebTemplate::WebTemplate(const std::filesystem::path& path)
{
    if (std::filesystem::exists(path))
    {
        std::ifstream file;
        file.open(path.string());

        if (file.is_open())
        {
            std::stringstream ss;
            ss << file.rdbuf();

            cereal::JSONInputArchive inputSerializer(ss);
            serialize(inputSerializer);
        }
    }
}

void WebTemplate::Write(std::stringstream& ss, const std::vector<std::pair<std::string, std::string>>& pairs)
{
    WriteStart(ss, pairs);
    WriteEnd(ss, pairs);
}

void WebTemplate::WriteStart(std::stringstream& ss, const std::vector<std::pair<std::string, std::string>>& pairs)
{
    std::string start = m_Start;
    for (const std::pair<std::string, std::string>& pair : pairs)
    {
        Utils::ReplaceStrInString(start, pair.first, pair.second);
    }

    ss << start;
    if (m_NewLineAfterStart)
    {
        ss << std::endl;
    }
}

void WebTemplate::WriteEnd(std::stringstream& ss, const std::vector<std::pair<std::string, std::string>>& pairs)
{
    std::string end = m_End;
    for (const std::pair<std::string, std::string>& pair : pairs)
    {
        Utils::ReplaceStrInString(end, pair.first, pair.second);
    }

    ss << end;
    if (m_NewLineAfterEnd)
    {
        ss << std::endl;
    }
}
