#pragma once
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Forward declarations.
struct WordData;
struct ParsingData;

template<class T>
struct FlowGraph;

struct Parser
{
    Parser();
    ~Parser();
    void parse_file(const fs::path& path);

private:
    // Helpers
    auto read_file(const fs::path& path) -> std::optional<std::string>;
    void create_flows();
    void read_token(ParsingData& parsing_data, const std::string& text, int index);
    void tokenize(FlowGraph<WordData>& parser, ParsingData& parsing_data, char token);

public:
    struct Internal;
    std::unique_ptr<Internal> m;
};

struct Enum
{
    std::string name;
    bool is_enum_class;
    std::string type;
    std::string namespace_scope;
    std::vector<std::string> values;
};

struct Variable
{
    std::string name;
};

struct Struct
{
    std::string name;
    int scope;
    std::string type;
    std::string namespace_scope;
    std::vector<Variable> variables;
};

struct ParsedData
{
    std::vector<Enum> enums;
    std::vector<Struct> structs;
};