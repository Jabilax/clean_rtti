#include "parser.h"
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

namespace
{
    bool token_check(const std::string& text, size_t index, const std::vector<char>& tokens)
    {
        for (auto& token : tokens)
             if(token == text[index])
                 return true;

        return false;
    }

    flow * current_filter_flow{ nullptr };
    flow * current_stream_flow{ nullptr };

    std::optional<std::string> read_file(const std::string& path)
    {
        // Read file contents
        std::ifstream file(path);

        if (!file.is_open())
            return {};

        // Copy file contents to text.
        std::string text;

        // Reserve space for the text
        file.seekg(0, std::ios::end);
        text.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        // Copy to string
        text.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        return text;
    }
}

template<class Container, class Value>
auto contains(Container c, Value v)
{
    return std::find(std::begin(c), std::end(c), v) != std::end(c);
}

auto Parser::get_current_file() const -> const FileData&
{
    return current_file;
}

void Parser::parse()
{
    for (const auto& directory : directories_to_read)
    {
        for (const auto& entry : fs::recursive_directory_iterator(fs::absolute(directory)))
        {
            auto path = entry.path();

            current_file.extension = path.extension().string();
            current_file.name = path.filename().replace_extension("").string();
            current_file.path = path.string();

            // Make sure it maches one of the given extensions.
            if (!contains(read_extensions, current_file.extension) && !contains(ignore_files, current_file.name))
                continue;

            auto text = read_file(current_file.path);

            if (text.has_value())
            {
                // tokenize
                tokenize(*text);
            }
        }
    }

}

void Parser::tokenize(const std::string& text)
{
    std::string word;

    for (size_t i = 0; i < text.size(); i++)
    {
        for (auto j = 0; j < token_packers.size(); j++)
        {
            bool add{ true };
            auto& fn_token_packer = token_packers[j];
            auto size = fn_token_packer(text, i, add);

            if (size != 0)
            {
                if (add) stream(text.substr(i, size)); 
                i += size;
                j = -1; // to restart loop
            }
        }

        if (custom_tocken_checker != nullptr)
            custom_tocken_checker(text[i]);
        
        if (token_check(text, i, remove_tokens))
        {
            if (!word.empty())
                stream(word);

            word.clear();
            continue;
        }
         
        if (token_check(text, i, insert_tokens))
        {
            if (!word.empty()) 
                stream(word);

            word.clear();
            stream(text.substr(i, 1));
            continue;
        }

        word.push_back(text[i]);
    }

    if (!word.empty())
        stream(word);
}

void Parser::stream(const std::string& word)
{
    // If next node is not nullptr the flow continues
    if (current_stream_flow == nullptr || current_stream_flow->next == nullptr)
    {
        // Check all the filter flows
        for (auto& stream_flow : stream_flows)
        {
            if (stream_flow.execute(word))
            {
                current_stream_flow = &stream_flow;
                return;
            }
        }
    }
    else
    {
        current_stream_flow->execute(word);
        return;
    }
}
