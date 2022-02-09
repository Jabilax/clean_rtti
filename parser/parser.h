#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <functional>
#include "flow.h"
#include "nodes.h"

namespace fs = std::filesystem;
using fn_token_packer = size_t(*)(const std::string & text, size_t index, bool & add);
using fn_token_checker = void(*)(char token);

struct FileData
{
    std::string name;
    std::string extension;
    std::string path;
};

struct Parser
{
public: // settings
    std::vector<std::string>        directories_to_read;
    std::vector<std::string>        ignore_folders;
    std::vector<std::string>        ignore_files;
    std::vector<std::string>        read_extensions;
    std::vector<fn_token_packer>    token_packers;
    std::vector<char>               remove_tokens;
    std::vector<char>               insert_tokens;
    std::vector<flow>               stream_flows;
    fn_token_checker                custom_tocken_checker{ nullptr };

private:
    FileData current_file;

private: 
    void tokenize(const std::string & text);
    void stream(const std::string& word);

public:  
    auto get_current_file() const -> const FileData&;
    void parse();
};