#include "parser/parser.h"
#include "parser/generator.h"
#include <cassert>
#include <cctype>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <stack>

namespace
{
    // ============================================================================
    // Helpers
    // ============================================================================
    
    auto contains = [](const auto& container, auto element)
    {
        for (auto value : container)
        {
            if (value == element)
            {
                return true;
            }
        }

        return false;
    };

}


// ============================================================================
// Parser
// ============================================================================

Parser::~Parser()
{
    //generate_file_reflection_header(m_files);
}

void Parser::parse_file(const fs::path& path)
{
    ParsingProcess parsing_data{ path };
    generate_file_reflection_info(path, parsing_data.m_data);
    m_files.push_back(path);
}


// ============================================================================
// ParsingProcess
// ============================================================================

ParsingProcess::ParsingProcess(const fs::path& path)
{
    auto text = read_file(path);

    if (text.has_value())
    {
        m_text = text.value();
        m_token = m_text.front();
        m_index = 0;
        parse_file();
    }
    else
    {
        assert(false);
    }
}

auto ParsingProcess::is_token(char character) -> bool
{
    return m_token == character;
}

auto ParsingProcess::is_token(const std::string& string) -> bool
{
    if (string.size() > (m_text.size() - m_index))
    {
        return false;
    }

    return std::memcmp(string.data(), m_text.data() + m_index, string.size()) == 0;
}

void ParsingProcess::advance_token(int size)
{
    m_index += size;
    m_token = (m_index >= m_text.size() ? '\0' : m_text[m_index]);
}

void ParsingProcess::advance_token_after(char token)
{
    while (!is_token(token))
    {
        advance_token();
    }

    advance_token();
}

void ParsingProcess::advance_token_after(const std::string& string)
{
    while (!is_token(string))
    {
        advance_token();
    }

    advance_token((int)string.size());
}

auto ParsingProcess::is_finished() -> bool
{
    return m_index >= m_text.size();
}

auto ParsingProcess::read_file(const fs::path& path) -> std::optional<std::string>
{
    // Read file contents.
    std::ifstream file(fs::absolute(path));

    if (!file.is_open())
    {
        return {};
    }

    // Copy file contents to text.
    std::string text;

    // Reserve space for the text.
    file.seekg(0, std::ios::end);
    text.reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    // Copy to string.
    text.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return text;
}

void ParsingProcess::next_token()
{
    advance_token();

    // Line comment
    if (is_token("//"))
    {
        advance_token(2);
        advance_token_after('\n');
    }

    // Block comment
    if (is_token("/*"))
    {
        advance_token(2);
        advance_token_after("*/");
    }
}

auto ParsingProcess::is_word(const std::string& word) -> bool
{
    return m_word == word;
}

void ParsingProcess::advance_word()
{
    m_word.clear();

    while (m_word.empty())
    {
        while (true)
        {
            if (is_finished())
            {
                return;
            }

            // Pack strings together
            if (is_token('\"'))
            {
                if (!m_word.empty())
                {
                    break;
                }

                auto start = m_index;
                advance_token();

                while (!is_token('\"'))
                {
                    advance_token(is_token('\\') ? 2 : 1);
                }

                advance_token();
                m_word = m_text.substr(start, m_index - start);
                break;
            }


            const auto separation_tokens = { ' ' , '\t', '\n', ';', ',', '(', ')', '{', '}', '=', '[', ']'};
            const auto include_tokens = { ';', ',', '(', ')', '{', '}', '=', '[', ']' };

            if (contains(separation_tokens, m_token))
            {
                if (contains(include_tokens, m_token))
                {
                    if (!m_word.empty())
                    {
                        break;
                    }
                    else
                    {
                        m_word.push_back(m_token);
                    }
                }

                next_token();
                break;
            }

            m_word.push_back(m_token);
            next_token();
        }
    }

    // Scopes
    if (is_word("{") || is_word("(")) { m_scope++; }
    if (is_word("}") || is_word(")")) { m_scope--; }
}

void ParsingProcess::advance_word_until(const std::string& word, bool previous_scope)
{
    auto scope = (previous_scope ? m_scope - 1: m_scope);

    while (!(m_scope <= scope && is_word(word)))
    {
        advance_word();
    }
}

void ParsingProcess::advance_word_until(std::vector<std::string> words, bool previous_scope)
{
    auto scope = (previous_scope ? m_scope - 1 : m_scope);

    auto check_word = [this, &words] () -> bool
    {
        bool found = false;
        for (auto i = 0; i < words.size() && !found; ++i)
        {
            found = is_word(words[i]);
        }
        return found;
    };

    while (!(m_scope <= scope && check_word()))
    {
        advance_word();
    }
}

void ParsingProcess::parse_file()
{
    advance_word(); // Get the first word

    while (!is_finished())
    {
        if (!parse_free_function() && !parse_class() && !parse_namespace())
        {
            advance_word();
            m_attributes.clear();
        }
    }
}

bool ParsingProcess::parse_class()
{
    parse_attribute();
    if (!is_word("struct") && !is_word("class"))
    {
        return false;
    }

    auto class_type = m_word;
    advance_word();
    parse_attribute();
    auto class_name = m_word;
    advance_word();

    while (true)
    {
        if (is_word(";")) { advance_word(); return true; } // Forward declaration, skip
        if (is_word("{")) { advance_word(); break; }
        advance_word();
    }

    auto& _class = m_data.classes.emplace_back();
    _class.name = class_name;
    _class.type = class_type;
    _class.attributes = m_attributes;
    _class._namespace = current_namespace();
    m_attributes.clear();

    // Parse members
    while (!is_word("}"))
    {
        if(!parse_class_member())
        {
            advance_word();
        }
    }

    advance_word_until(";");
    advance_word();

    return true;
}

bool ParsingProcess::parse_class_member()
{
    parse_attribute();
    parse_template();

    if (m_word.size() == 1)
    {
        return false;
    }

    std::string prev_identifier;

    while (true)
    {
        if (is_word(";"))
        {
            auto& _class = m_data.classes.back();
            auto& variable = _class.variables.emplace_back();
            variable.name = prev_identifier;
            variable.attributes = m_attributes;
            m_attributes.clear();

            advance_word();
            return true;
        }
        else if (is_word("=") || is_word("{") || is_word("["))
        {
            auto& _class = m_data.classes.back();
            auto& variable = _class.variables.emplace_back();
            variable.name = prev_identifier;
            variable.attributes = m_attributes;
            m_attributes.clear();

            advance_word_until(";");
            advance_word();
            return true;
        }
        else if (is_word("("))
        {
            auto& _class = m_data.classes.back();
            auto& function = _class.functions.emplace_back();
            function.name = prev_identifier;
            function.attributes = m_attributes;
            m_attributes.clear();

            advance_word_until(std::vector<std::string>{ "}", ";" }, true);
            advance_word();
            return true;
        }
        else
        {
            prev_identifier = m_word;
        }

        advance_word();
    }
}

auto ParsingProcess::parse_free_function() -> bool
{
    // skip free functions
    if (is_word("("))
    {
        advance_word_until(std::vector<std::string>{ "}", ";" }, true);
        advance_word();
        return true;
    }

    return false;
}

void ParsingProcess::parse_template()
{
    // TODO
}

void ParsingProcess::parse_attribute()
{
    if (is_token("[["))
    {
        advance_token_after("]]");
    }

    if (is_word("attribute"))
    {
        advance_word();
        assert(is_word("("));
        advance_word(); // (

        while (!is_word(")"))
        {
            auto& attribute = m_attributes.emplace_back();
            attribute.name = m_word;
            advance_word();

            if (is_word("("))
            {
                auto scope = m_scope - 1;

                while (m_scope > scope)
                {
                    advance_word();
                    attribute.arguments.emplace_back(m_word);
                    advance_word();
                    assert(is_word(",") || is_word(")"));
                }

                advance_word(); // )
            }

            if (!is_word(")"))
            {
                assert(is_word(","));
                advance_word();
            }
        }
        advance_word();
    }
}

bool ParsingProcess::parse_namespace()
{
    if (is_word("namespace"))
    {
        advance_word();

        if (is_word("{")) // Anonymous namespace, skip
        {
            advance_word_until("}");
            advance_word();
            return true;
        }

        auto namespace_name = m_word;
        advance_word();

        while (true)
        {
            if (is_word(";")) { advance_word(); return true; } // Forward declaration, skip
            if (is_word("{")) { advance_word(); break; }
            advance_word();
        }

        const auto name = (m_namespaces.empty() ? namespace_name : m_namespaces.back().name + "::" + namespace_name);
        auto& _namespace = m_namespaces.emplace_back();
        _namespace.name = name;
        _namespace.scope = m_scope - 1; // Considering we reached { that will add one to the scope.
        return true;
    }

    if (!m_namespaces.empty() && is_word("}") && m_scope <= m_namespaces.back().scope)
    {
        m_namespaces.pop_back();
        advance_word();
        return true;
    }

    // This means we didn't handle anything, position remains untouched.
    return false;
}

auto ParsingProcess::current_namespace() -> std::string
{
    return (m_namespaces.empty() ? "" : m_namespaces.back().name);
}