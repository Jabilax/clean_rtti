#pragma once
#include "generic_parser/parser.h"
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

struct Enum
{
    Enum(const std::string& name);

    std::string name;
    std::vector<std::string> values;
};

struct ParsedDataCpp
{
    std::vector<Enum> enums;
};

struct ParserCpp
{
    ParserCpp();
    ~ParserCpp();
    void parse_file(const fs::path& path);

private:
    // Helpers
    auto read_file(const fs::path& path) -> std::optional<std::string>;
    void create_flows();
    void tokenize(Parser<WordData>& parser, ParsingData& parsing_data, char token);

public:
    struct Internal;
    std::unique_ptr<Internal> m;
};
