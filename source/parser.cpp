#include "parser.h"
#include "reflection.h"
#include "generic/node_graph.h"
#include "generic/flow_graph.h"
#include <cassert>
#include <cctype>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <stack>


struct Namespace{ std::string name; int scope; };

struct ParsingData
{
    ParsedData parsed;

    bool inside_struct{ false };
    bool inside_enum{ false };
    bool inside_block_comment{ false };
    bool inside_line_comment{ false };
    bool inside_string{ false };
    bool prev_token_is_backslash{ false };
    int  template_scope_counter{ 0 };
    int  scope_counter{ 0 };
    int  function_scope{ 0 };
    int  parentheses_scope{ 0 };
    std::vector<Namespace> namespaces;

    auto is_inside_comment() { return inside_block_comment || inside_line_comment; }
    auto is_inside_template() { return template_scope_counter > 0; }
    auto is_inside_function() { return function_scope > 0; }
    auto is_inside_parentheses() { return parentheses_scope > 0; }
    auto get_namespace_scope() { 
        std::string word;
        for (auto& namespace_type : namespaces) { word += (word.empty() ? "" : "::") + (namespace_type.name); }
        return word;
    }

    std::string word;
    std::string var_name;
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

struct Parser::Internal
{
    FlowGraph<WordData> word_flow;
};

Parser::Parser()
    : m{ std::make_unique<Internal>() }
{
    create_flows();
}

Parser::~Parser()
{
}

void Parser::parse_file(const fs::path& path)
{
    // Todo: potentially add multithreading to this.

    const auto text = read_file(path);
    assert(text.has_value()); // Todo: handle this.

    auto word_flow = m->word_flow; // Copy.

    ParsingData parsing_data;
    for (auto i = 0; i < text->size(); ++i)
    {
        read_token(parsing_data, text.value(), i);
        tokenize(word_flow, parsing_data, text.value()[i]);
    }

    generate_reflection_info(path, parsing_data.parsed);
}

void Parser::read_token(ParsingData& parsing_data, const std::string& text, int index)
{
    auto token = text[index];
    auto is_prev_token_equal = [&text, index](char token_to_compare) { return index != 0 && text[index - 1] == token_to_compare; };
    auto is_string_equal = [&text, index](const std::string& text_to_compare) { return text.substr(index, text_to_compare.size()) == text_to_compare; };

    if (token == '\n') { parsing_data.inside_line_comment = false; }
    if (is_string_equal("*/")) { parsing_data.inside_block_comment = false; }

    if (!parsing_data.is_inside_comment())
    {
        if (is_string_equal("//")) { parsing_data.inside_line_comment = true;  }
        if (is_string_equal("/*")) { parsing_data.inside_block_comment = true; }
        if (token == '\"' && !is_prev_token_equal('\\'))  { parsing_data.inside_string = !parsing_data.inside_string; }
        if (token == '{'){ parsing_data.scope_counter++; }
        if (token == '}')
        {
            parsing_data.scope_counter--;
            parsing_data.function_scope = std::min(parsing_data.function_scope, parsing_data.scope_counter);

            if (!parsing_data.namespaces.empty() && parsing_data.namespaces.back().scope > parsing_data.scope_counter)
            {
                parsing_data.namespaces.pop_back();
            }
        }

        if (!parsing_data.is_inside_function())
        {
            if (token == '<') { parsing_data.template_scope_counter++; }
            if (token == '>') { parsing_data.template_scope_counter--; }
        }
    }

}

void Parser::tokenize(FlowGraph<WordData>& word_flow, ParsingData& parsing_data, char token)
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

    const auto separation_tokens = { ' ' , '\t', '\n', ';', ',', '(', ')', '{', '}', '=' };
    const auto include_tokens = { ';', ',', '(', ')', '{', '}', '=' };

    if (!parsing_data.is_inside_comment() && !parsing_data.is_inside_template() && !parsing_data.is_inside_function() && contains(separation_tokens, token))
    {
        if (!parsing_data.word.empty())
        {
            word_flow.push(WordData{ parsing_data, parsing_data.word });
            parsing_data.word.clear();
        }

        if (contains(include_tokens, token))
        {
            word_flow.push(WordData{ parsing_data, std::string(1, token) });
        }
    }
    else
    {
        parsing_data.word.push_back(token);
    }
}

auto Parser::read_file(const fs::path& path) -> std::optional<std::string>
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

void Parser::create_flows()
{
    // Usings.
    using FNode   = FutureNode<WordData>;
    using BoolFn  = BoolNodeFn<WordData>;
    using VoidFn  = VoidNodeFn<WordData>;

    // Nodes.
    auto execute_node              = [](FNode fnode, BoolFn expect, VoidFn execute, FNode next_node, std::string debug_name = "") { fnode.set(new ExecuteNode(expect, execute, next_node, debug_name)); };
    auto compare_node              = [](FNode fnode, BoolFn compare, FNode true_node, FNode false_node, std::string debug_name = "") { fnode.set(new CompareNode(compare, true_node, false_node, debug_name)); };
    auto compare_execute_node      = [](FNode fnode, BoolFn compare, VoidFn execute, FNode true_node, FNode false_node, std::string debug_name = "") { fnode.set(new CompareExecuteNode(compare, execute, true_node, false_node, debug_name)); };
    auto compare_execute_wait_node = [](FNode fnode, BoolFn compare, VoidFn execute, FNode true_node, FNode false_node, std::string debug_name = "") { fnode.set(new CompareExecuteWaitNode(compare, execute, true_node, false_node, debug_name)); };
    auto switch_wait_node          = [](FNode fnode, BoolFn compare_1, BoolFn compare_2, FNode node_1, FNode node_2, FNode node_none, std::string debug_name = "") { fnode.set(new SwitchWaitNode(compare_1, compare_2, node_1, node_2, node_none, debug_name)); };

    // Helpers.
    auto is_equal            = [](const std::string& string) { return (BoolFn)[string](const WordData& data) { return data.word == string; }; };
    auto is_name             = (BoolFn)[](WordData& data) { 
        return std::isalpha(data.word[0]); 
    };
    auto is_number           = (BoolFn)[](WordData& data) { return std::isdigit(data.word[0]); };
    auto expect_any_word     = (BoolFn)[](WordData& data) { return true; };
    auto do_nothing          = (VoidFn)[](WordData& data) {};
    auto join                = [](BoolFn fn_1, BoolFn fn_2) { auto a = fn_1; auto b = fn_2; return [a, b](WordData& data) { return a(data) && b(data); }; };
    auto join_or             = [](BoolFn fn_1, BoolFn fn_2) { auto a = fn_1; auto b = fn_2; return [a, b](WordData& data) { return a(data) || b(data); }; };

    // Enums.
    auto create_enum     = (VoidFn)[](WordData& data) { data.parsing.parsed.enums.emplace_back("Missing Name", false, "int", data.parsing.get_namespace_scope(), std::vector<std::string>{}); data.parsing.inside_enum = true; };
    auto set_enum_name   = (VoidFn)[](WordData& data) { data.parsing.parsed.enums.back().name = data.word; };
    auto set_enum_type   = (VoidFn)[](WordData& data) { data.parsing.parsed.enums.back().type = data.word; };
    auto set_enum_class  = (VoidFn)[](WordData& data) { data.parsing.parsed.enums.back().is_enum_class = true; };
    auto set_inside_enum = [](bool set_inside) { return (VoidFn)[set_inside](WordData& data) { data.parsing.inside_enum = set_inside; }; };
    auto remove_enum     = (VoidFn)[](WordData& data) { data.parsing.parsed.enums.pop_back(); data.parsing.inside_enum = false; };
    auto add_enum_name   = (VoidFn)[](WordData& data) { data.parsing.parsed.enums.back().values.emplace_back(data.word, 0); };

    // Functions.
    auto set_inside_function = (VoidFn)[](WordData& data) { data.parsing.function_scope = data.parsing.scope_counter; };

    // Namespaces.
    auto push_namespace = (VoidFn)[](WordData& data) { data.parsing.namespaces.emplace_back(data.word, data.parsing.scope_counter + 1); };
    auto pop_namespace  = (VoidFn)[](WordData& data) { data.parsing.namespaces.pop_back(); };

    // Structs.
    auto is_struct_definition = (BoolFn)[](WordData& data) { return !data.parsing.inside_enum && (data.word == "class" || data.word == "struct"); };
    auto create_struct        = (VoidFn)[](WordData& data) { data.parsing.parsed.structs.push_back(Struct{"Missing_Name", data.parsing.scope_counter + 1, data.word, data.parsing.get_namespace_scope(), std::vector<Variable>{} }); data.parsing.inside_struct = true; };
    auto set_struct_name      = (VoidFn)[](WordData& data) { data.parsing.parsed.structs.back().name = data.word; };
    auto remove_struct        = (VoidFn)[](WordData& data) { data.parsing.parsed.structs.pop_back(); data.parsing.inside_struct = false; };
    auto is_struct_ended      = (BoolFn)[](WordData& data) { return data.parsing.parsed.structs.back().scope > data.parsing.scope_counter; };
    auto set_finish_struct    = (VoidFn)[](WordData& data) { data.parsing.inside_struct = false; };
    auto is_inside_struct     = (BoolFn)[](WordData& data) { return data.parsing.inside_struct; };

    // Variables
    auto set_variable_name    = (VoidFn)[](WordData& data) { 
        data.parsing.var_name = data.word; 
    };
    auto create_variable      = (VoidFn)[](WordData& data) { 
        data.parsing.parsed.structs.back().variables.emplace_back(data.parsing.var_name);
    };


    // ------------------------------------------- Enums ----------------------------------------------
    {
        FNode A, B, C, D, E, F, G, H, I, J, null;
        compare_execute_node(A, is_equal("enum"), create_enum, G, null);
        compare_execute_node(G, join_or(is_equal("struct"), is_equal("class")), set_enum_class, B, B);
        execute_node(B, is_name, set_enum_name, H);
        compare_node(H, is_equal(":"), I, C);
        execute_node(I, is_name, set_enum_type, C);
        compare_execute_node(C, is_equal(";"), remove_enum, null, F);
        compare_execute_wait_node(F, is_equal("{"), do_nothing, D, C);
        compare_execute_wait_node(D, is_name, add_enum_name, E, D);
        switch_wait_node(E, is_equal(","), is_equal("}"), D, J, E);
        execute_node(J, is_equal(";"), set_inside_enum(false), null);
        m->word_flow.add_node_graph(A);
    }

    // ----------------------------------------- Functions --------------------------------------------
    {
        FNode A, B, null;
        compare_node(A, is_equal(")"), B, null);
        compare_execute_node(B, is_equal("{"), set_inside_function, null, null);
        m->word_flow.add_node_graph(A);
    }

    // ----------------------------------------- Namespaces -------------------------------------------
    {
        FNode A, B, C, null;
        compare_node(A, is_equal("namespace"), B, null);
        execute_node(B, is_name, push_namespace, C);
        compare_execute_node(C, is_equal(";"), pop_namespace, null, null);
        m->word_flow.add_node_graph(A);
    }

    // ------------------------------------------ Structs ---------------------------------------------
    {
        FNode A, B, C, D, E, F, G, H, I, J, null;
        compare_execute_node(A, is_struct_definition, create_struct, B, null);
        execute_node(B, is_name, set_struct_name, C);

        compare_execute_node(C, is_equal(";"), remove_struct, null, I, "C");
        compare_execute_wait_node(I, is_equal("{"), do_nothing, E, C, "I");

        compare_execute_node(E, is_name, set_variable_name, E, F, "E");
        compare_execute_node(F, is_equal(";"), create_variable, E, H, "F");
        compare_execute_node(H, join_or(is_equal("="), is_equal("{")), create_variable, G, J, "H");
        compare_node(J, is_equal("("), G, D);
        compare_execute_wait_node(G, is_equal(";"), do_nothing, D, G, "G");

        compare_execute_wait_node(D, is_struct_ended, set_finish_struct, null, E, "D");
        m->word_flow.add_node_graph(A);
    }

    //{
    //    FNode A, B, C, D, null;
    //    compare_execute_node(A, is_inside_struct, create_struct, B, null);
    //    execute_node(B, is_name, set_struct_name, C);
    //    compare_execute_node(C, is_equal(";"), remove_struct, null, D);
    //    compare_execute_wait_node(D, is_struct_ended, set_finish_struct, null, D);
    //    m->word_flow.add_node_graph(A);
    //}
}