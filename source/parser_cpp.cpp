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
    int template_scope{ 0 };

    std::string word;
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

Enum::Enum(const std::string& name)
    : name{ name }
{
}


struct ParserCpp::Internal
{
    Parser<TokenData> token_parser;
    Parser<WordData> word_parser;
    std::unordered_map<std::string, ParsingData> data;
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

    auto& parsing_data = m->data[path.string()];
    auto token_parser = m->token_parser; // Copy.
    auto word_parser = m->word_parser;   // Copy.

    for (auto i = 0; i < text->size(); ++i)
    {
        token_parser.push(TokenData{ parsing_data, text.value(), i });
        tokenize(word_parser, parsing_data, text.value()[i]);
    }
}

void ParserCpp::tokenize(Parser<WordData>& parser, ParsingData& parsing_data, char token)
{
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

    auto separation_tokens = { ' ' , '\t', '\n', ';', ',', '(', ')', '{', '}', '=' };
    auto include_tokens = { '\n', ';', ',', '(', ')', '{', '}' };

    if (!parsing_data.inside_comment && !parsing_data.inside_string && (parsing_data.template_scope == 0) && contains(separation_tokens, token))
    {
        if (!parsing_data.word.empty())
        {
            parser.push(WordData{ parsing_data, parsing_data.word });
            parsing_data.word.clear();
        }

        if (contains(include_tokens, token))
        {
            parser.push(WordData{ parsing_data, std::string(1, token) });
        }
    }
    else
    {
        parsing_data.word.push_back(token);
    }
}


auto ParserCpp::read_file(const fs::path& path) -> std::optional<std::string>
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

// Declare some usings to make lines shorter.
template<class T> using Node = FutureFlowNode<T>;
template<class T> using BoolFn = BoolNodeFn<T>;
template<class T> using VoidFn = VoidNodeFn<T>;

void ParserCpp::create_flows()
{
    using TokenNode   = Node<TokenData>;
    using TokenBoolFn = BoolFn<TokenData>;
    using TokenVoidFn = VoidFn<TokenData>;
    using WordNode    = Node<WordData>;
    using WordBoolFn  = BoolFn<WordData>;
    using WordVoidFn  = VoidFn<WordData>;

    // Create functions that will change the data.
    auto execute_node = []<class T>(Node<T> fnode, BoolFn<T> expect, VoidFn<T> execute, Node<T> next_node) { fnode.set(new ExecuteNode(expect, execute, next_node)); };
    auto compare_node = []<class T>(Node<T> fnode, BoolFn<T> compare, Node<T> true_node, Node<T> false_node) { fnode.set(new CompareNode(compare, true_node, false_node)); };
    auto compare_execute_node = []<class T>(Node<T> fnode, BoolFn<T> compare, VoidFn<T> execute, Node<T> true_node, Node<T> false_node) { fnode.set(new CompareExecuteNode(compare, execute, true_node, false_node)); };
    auto compare_execute_wait_node = []<class T>(Node<T> fnode, BoolFn<T> compare, VoidFn<T> execute, Node<T> true_node, Node<T> false_node) { fnode.set(new CompareExecuteWaitNode(compare, execute, true_node, false_node)); };
    auto switch_wait_node = []<class T>(Node<T> fnode, BoolFn<T> compare_1, BoolFn<T> compare_2, Node<T> node_1, Node<T> node_2, Node<T> node_none) { fnode.set(new SwitchWaitNode(compare_1, compare_2, node_1, node_2, node_none)); };
    
    auto join = []<class T>(BoolFn<T> fn_1, BoolFn<T> fn_2) -> BoolFn<T> { auto a = fn_1; auto b = fn_2; return [a, b](T& data) { return a(data) && b(data); }; };

    // Token
    auto set_inside_comment       = [](bool value) { return (TokenVoidFn)[value](TokenData& data) { data.parsing.inside_comment = value; }; };
    auto set_inside_string        = [](bool value) { return (TokenVoidFn)[value](TokenData& data) { data.parsing.inside_string = value; }; };
    auto set_prev_backslash       = [](bool value) { return (TokenVoidFn)[value](TokenData& data) { data.parsing.prev_token_is_backslash = value; }; };
    auto is_prev_backslash        = [](bool value) { return (TokenBoolFn)[value](TokenData& data) { return data.parsing.prev_token_is_backslash == value; }; };
    auto is_in_string             = [](bool value) { return (TokenBoolFn)[value](TokenData& data) { return data.parsing.inside_string == value; }; };
    auto increment_template_scope = [](int value)  { return (TokenVoidFn)[value](TokenData& data) { data.parsing.template_scope += value; }; };
    auto is_token                 = [](const std::string& string) { return (TokenBoolFn)[string](const TokenData& data) { return data.text.substr(data.text_index, string.size()) == string; }; };
    auto is_token_not_ignored     = (TokenBoolFn)[](TokenData& data) { return !data.parsing.inside_comment && !data.parsing.inside_string; };
    auto expect_any_token         = (TokenBoolFn)[](TokenData&) { return true; };

    // Word
    auto is_word                = [](const std::string& string) { return (WordBoolFn)[string](const WordData& data) { return data.word == string; }; };
    auto is_name                = (WordBoolFn)[](WordData& data) { return std::isalpha(data.word[0]); };
    auto is_number              = (WordBoolFn)[](WordData& data) { return std::isdigit(data.word[0]); };
    auto is_word_not_ignored    = (WordBoolFn)[](WordData& data) { return !data.parsing.inside_comment && !data.parsing.inside_string; };
    auto expect_any_word        = (WordBoolFn)[](WordData& data) { return true; };
    auto do_nothing             = (WordVoidFn)[](WordData& data) {};

    //auto set_starting_scope = [](const std::string & word) { scope_start = scope_count; };
    //auto check_exit_scope = [](const std::string & word) -> bool { return word[0] == '}' && scope_start > scope_count; };

    auto create_enum    = (WordVoidFn)[](WordData& data) { data.parsing.parsed.enums.push_back(data.word); };
    auto remove_enum    = (WordVoidFn)[](WordData& data) { data.parsing.parsed.enums.pop_back(); };
    auto add_enum_name  = (WordVoidFn)[](WordData& data) { data.parsing.parsed.enums.back().values.emplace_back(data.word); };

    // Flows.
    // ------------------------------------- Single Line Comment ------------------------------------------
    {
        TokenNode A, B, null;
        compare_execute_node(A, join(is_token("//"), is_token_not_ignored), set_inside_comment(true), B, null);
        compare_execute_wait_node(B, is_token("\n"), set_inside_comment(false), null, B);
        m->token_parser.add_flow(A);
    }
    
    // ------------------------------------ MultiLine Comment Flow ----------------------------------------
    {
        TokenNode A, B, null;
        compare_execute_node(A, join(is_token("/*"), is_token_not_ignored), set_inside_comment(true), B, null);
        compare_execute_wait_node(B, is_token("*/"), set_inside_comment(false), null, B);
        m->token_parser.add_flow(A);
    }
    
    // ----------------------------------------- String Flow ----------------------------------------------
    {
        TokenNode A, B, null;
        compare_execute_node(A, join(is_token("\""), is_token_not_ignored), set_inside_string(true), B, null);
        compare_execute_wait_node(B, join(is_token("\""), is_prev_backslash(false)), set_inside_string(false), null, B);
        m->token_parser.add_flow(A);
    }

    // -------------------------------------- Template Scope Flow -----------------------------------------
    {
        TokenNode A, B, null;
        compare_execute_wait_node(A, join(is_token("<"), is_token_not_ignored), increment_template_scope(1), null, null);
        compare_execute_wait_node(B, join(is_token(">"), is_token_not_ignored), increment_template_scope(-1), null, null);
        m->token_parser.add_flow(A);
        m->token_parser.add_flow(B);
    }
    
    // ---------------------------------------- BackSlash Flow --------------------------------------------
    {
        TokenNode A, B, null;
        compare_execute_node(A, is_token("\\"), set_prev_backslash(true), B, null);
        execute_node(B, expect_any_token, set_prev_backslash(false), null);
        m->token_parser.add_flow(A);
    }
    
    //// ---------------------------------------- Scope Flow ----------------------------------------------
    //{
    //    TokenNode A, B, null;
    //    compexe_node(A, is_equal("\\"), set_prev_backslash(true), B, null);
    //    execute_node(B, expect_anything, set_prev_backslash(false), null);
    //    parser.word_flows.emplace_back(A);
    //}
    //

    // ------------------------------------------- Enum Flow ----------------------------------------------
    {
        WordNode A, B, C, D, E, F, G, null;
        compare_node(A, join(is_word("enum"), is_word_not_ignored), G, null);
        compare_node(G, join(is_word("struct"), is_word("class")), B, B);
        execute_node(B, is_name, create_enum, C);
        compare_execute_node(C, is_word(";"), remove_enum, null, F);
        compare_execute_wait_node(F, is_word("{"), do_nothing, D, C);
        execute_node(D, is_name, add_enum_name, E);
        switch_wait_node(E, is_word(","), is_word("}"), D, null, E);
        m->word_parser.add_flow(A);
    }
}

// parser.remove_tokens = { ' ' , '\t', '\n' };
// parser.insert_tokens = { ':', ',', ';', '{', '}', '(', ')', '<', '>', '*', '&', '=' };

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