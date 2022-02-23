#include "parser_cpp.h"
#include "generic_parser/flow.h"
#include "generic_parser/parser.h"
#include <cassert>
#include <cctype>
#include <fstream>
#include <unordered_map>


struct ParsingData
{
    ParsedDataCpp parsed;

    bool inside_comment{ false };
    bool inside_string{ false };
    bool prev_token_is_backslash{ false };
};

struct TokenData
{
    TokenData(ParsingData& data, const std::string& text, int text_index);

    ParsingData& parsing;
    const std::string& text;
    int text_index;
};

struct WordData
{
    WordData(ParsingData& data, const std::string& word);

    ParsingData& parsing;
    const std::string& word;
};

TokenData::TokenData(ParsingData& data, const std::string& text, int text_index)
    : parsing{ data }
    , text{ text }
    , text_index{ text_index }
{
}

WordData::WordData(ParsingData& data, const std::string& word)
    : parsing{ data }
    , word{ word }
{
}

struct ParserCpp::Internal
{
    Parser<TokenData> token_parser;
    std::unordered_map<std::string, ParsingData> data;

    Internal()
    {
    }
};

ParserCpp::ParserCpp()
    : m{ std::make_unique<Internal>() }
{
    create_flows();
}

ParserCpp::~ParserCpp()
{
}

void ParserCpp::parse_file(const fs::path& path)
{
    // Todo: potentially add multithreading to this.

    const auto text = read_file(path);
    assert(text.has_value()); // Todo: handle this.

    ParsingData parsing_data;
    Parser<TokenData> token_parser;

    for (auto i = 0; i < text->size(); ++i)
    {
        token_parser.push(TokenData{ parsing_data, text.value(), i });
    }
}

auto ParserCpp::read_file(const fs::path& path) -> std::optional<std::string>
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

// Declare some usings to make lines shorter.
template<class T> using Node = FutureFlowNode<T>;
template<class T> using BoolFn = BoolNodeFn<T>;
template<class T> using VoidFn = VoidNodeFn<T>;

auto compare(const WordData& data, const std::string& string) -> bool { return data.word == string; }
auto compare(const TokenData& data, const std::string& string) -> bool { return data.text.substr(data.text_index, string.size()) == string; }

template<class T> auto is_start_equal(const std::string& string) -> BoolFn<T> { return [string](T& data){ return !data.parsing.inside_comment && !data.parsing.inside_string && compare(data, string); }; }

void ParserCpp::create_flows()
{
    using TokenNode = Node<TokenData>;
    using TokenBoolFn = BoolFn<TokenData>;
    using TokenVoidFn = VoidFn<TokenData>;
    using WordNode = Node<WordData>;
    using WordBoolFn = BoolFn<WordData>;
    using WordVoidFn = VoidFn<WordData>;

    // Create functions that will change the data.
    auto execute_node = []<class T>(Node<T> fnode, BoolFn<T> expect,  VoidFn<T> execute, Node<T> next_node) { fnode.set(new ExecuteNode(expect, execute, next_node)); };
    auto compare_node = []<class T>(Node<T> fnode, BoolFn<T> compare, Node<T> true_node, Node<T> false_node) { fnode.set(new CompareNode(compare, true_node, false_node)); };
    auto compexe_node = []<class T>(Node<T> fnode, BoolFn<T> compare, VoidFn<T> execute, Node<T> true_node, Node<T> false_node) { fnode.set(new CompareExecuteNode(compare, execute, true_node, false_node)); };
    auto join = []<class T>(BoolFn<T> fn_a, BoolFn<T> fn_b) -> BoolFn<T> { auto a = fn_a; auto b = fn_b; return [a, b](T& data) { return a(data) && b(data); }; };

    //auto do_nothing = [](const auto& data) {};

    // Token
    auto set_inside_comment = [](bool value) { return (TokenVoidFn)[value](TokenData& data) { data.parsing.inside_comment = value; }; };
    auto set_inside_string = [](bool value) { return (TokenVoidFn)[value](TokenData& data) { data.parsing.inside_string = value; }; };
    auto set_prev_backslash = [](bool value) { return (TokenVoidFn)[value](TokenData& data) { data.parsing.prev_token_is_backslash = value; }; };
    auto is_token = [](const std::string& string) { return (TokenBoolFn)[string](const TokenData& data) { return compare(data, string); }; };
    auto is_prev_backslash = [](bool value) { return (TokenBoolFn)[value](TokenData& data) { return data.parsing.prev_token_is_backslash == value; }; };
    auto expect_any_token = (TokenBoolFn)[](TokenData&) { return true; };

    // Word
    auto is_word = [](const std::string& string) { return (WordBoolFn)[string](const WordData& data) { return compare(data, string); }; };
    auto is_name = [](const WordData& data) -> bool { return std::isalpha(data.word[0]); };
    auto is_number = [](const WordData& data) -> bool { return std::isdigit(data.word[0]); };
    auto expect_any_word = (WordBoolFn)[](WordData&) { return true; };

    //auto is_string = [](const std::string & word) -> bool { return word[0] == '"'; };
    //auto set_starting_scope = [](const std::string & word) { scope_start = scope_count; };
    //auto check_exit_scope = [](const std::string & word) -> bool { return word[0] == '}' && scope_start > scope_count; };
    //
    //auto is_start_equal = []<typename T>(const std::string& string) -> BoolNodeFn<T>
    //{
    //    return [string](const T& data) -> bool 
    //    {
    //        return !data.parsing.inside_comment && !data.parsing.inside_string && compare(data, string);
    //    };
    //};
    //
    //auto create_enum = [](const std::string& word) { enums.push_back(word); enums.back().file = parser.get_current_file().name; };
    //auto remove_enum = [](const std::string& word) { enums.pop_back(); };
    //auto add_enum_name = [](const std::string& word) { enums.back().elements.emplace_back(word); };

    // Create Flows.
    // ------------------------------------- Single Line Comment ------------------------------------------
    {
        TokenNode A, B, null; using T = TokenData;
        compexe_node(A, is_start_equal<T>("test"), set_inside_comment(true), B, null);
        compexe_node(B, is_token(";"), set_inside_comment(false), null, B);
        m->token_parser.add_flow(A);
    }
    
    // ------------------------------------ MultiLine Comment Flow ----------------------------------------
    {
        TokenNode A, B, null; using T = TokenData;
        compexe_node(A, is_start_equal<T>("/*"), set_inside_comment(true), B, null);
        compexe_node(B, is_token("*/"), set_inside_comment(false), null, B);
        m->token_parser.add_flow(A);
    }
    
    // ----------------------------------------- String Flow ----------------------------------------------
    {
        TokenNode A, B, null; using T = TokenData;
        compexe_node(A, is_start_equal<T>("\""), set_inside_string(true), B, null);
        compexe_node(B, join(is_token("\""), is_prev_backslash(false)), set_inside_string(false), null, B);
        m->token_parser.add_flow(A);
    }
    
    // ---------------------------------------- BackSlash Flow --------------------------------------------
    {
        TokenNode A, B, null;
        compexe_node(A, is_token("\\"), set_prev_backslash(true), B, null);
        execute_node(B, expect_any_token, set_prev_backslash(false), null);
        m->token_parser.add_flow(A);
    }
    
    //// ------------------------------------------ Scope Flow ----------------------------------------------
    //{
    //    TokenNode A, B, null;
    //    compexe_node(A, is_equal("\\"), set_prev_backslash(true), B, null);
    //    execute_node(B, expect_anything, set_prev_backslash(false), null);
    //    parser.word_flows.emplace_back(A);
    //}
    //
    //// ------------------------------------------- Enum Flow ----------------------------------------------
    //{
    //    WordNode A, B, C, D, E, F, G, H, I, J, K, null;
    //    compare_node(A, is_start_equal("enum"), B, null);
    //    execute_node(B, is_name, create_enum, C);
    //    compexe_node(C, is_equal(";"), remove_enum, null, D);
    //    compare_node(D, is_equal(":"), K, J);
    //    execute_node(K, is_name, do_nothing, J);
    //    execute_node(J, is_equal("{"), do_nothing, E);
    //    execute_node(E, is_name, add_enum_name, F);
    //    compare_node(F, is_equal("="), I, G);
    //    compare_node(G, is_equal(","), E, H);
    //    execute_node(H, is_equal("}"), do_nothing, G);
    //    execute_node(I, expect_anything, do_nothing, null);
    //    parser.word_flows.emplace_back(A);
    //}
}

/*namespace fs = std::filesystem;

namespace
{
    bool token_check(const std::string& text, size_t index, const std::vector<char>& tokens)
    {
        for (auto& token : tokens)
            if (token == text[index])
                return true;

        return false;
    }

    Flow* current_filter_flow{ nullptr };
    Flow* current_stream_flow{ nullptr };
}

template<class Container, class Value>
auto contains(Container c, Value v)
{
    return std::find(std::begin(c), std::end(c), v) != std::end(c);
}

void Parser::parse(const std::string& text)
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
        for (auto& stream_flow : flows)
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
}*/