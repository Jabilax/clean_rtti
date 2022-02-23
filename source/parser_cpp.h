#pragma once
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace fs = std::filesystem;


struct ParsedDataCpp
{
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

public:
    struct Internal;
    std::unique_ptr<Internal> m;
};
