#pragma once
#include "parsed_data.h"
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// Forward declarations
class Parser;
class ParsingProcess;
struct Scope;
struct Attribute;
struct Enum;
struct MemberVariable;
struct MemberFunction;
struct Class;
struct ParsedData;
namespace fs = std::filesystem;


class Parser
{
public:
    // Destruction
    ~Parser();

    // Parse
    void parse_file(const fs::path& path);

private:
    std::vector<fs::path> m_files;
};

class ParsingProcess
{
public:
    ParsingProcess(const fs::path& path);

private:
    // Tokens
    auto is_token(char character) -> bool;
    auto is_token(const std::string& string) -> bool;
    void next_token();
    void advance_token(int size = 1);
    void advance_token_after(char token);
    void advance_token_after(const std::string& string);
    auto is_finished() -> bool;

    // Word
    auto is_word(const std::string& word) -> bool;
    void advance_word();
    void advance_word_until(const std::string& word, bool previous_scope = false);
    void advance_word_until(std::vector<std::string> words, bool previous_scope = false);

    // Parse
    auto read_file(const fs::path& path) -> std::optional<std::string>;
    void parse_file();
    auto parse_class() -> bool;
    auto parse_class_member() -> bool;
    auto parse_free_function() -> bool;
    auto parse_namespace() -> bool;
    void parse_template();
    void parse_attribute();
    
    // Parsed data
    auto current_namespace() -> std::string;

public:
    ParsedData m_data;

private:
    std::string m_text;
    int m_index;
    char m_token;

    int m_scope{ 0 };
    std::vector<Namespace> m_namespaces;
    std::vector<Scope> m_scopes;

    std::string m_word;

    std::string m_saved_word;
    std::vector<Attribute> m_attributes;
};

enum class ScopeType
{
    Namespace,
    Class,
    Ignore, // Function or variable declaration
    Template,
    Attribute,
    Enum,
};

struct Scope
{
    ScopeType type;
    int scope;
};