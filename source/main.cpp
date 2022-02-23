#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <windows.h>
#include "parser_cpp.h"

int main()
{
    ParserCpp parser;


    return 0;
}

//#define print(what) std::cout << #what << " : " << what << "\n"
//
//std::vector<std::string> directories_to_read;
//std::vector<std::string> ignore_folders;
//std::vector<std::string> ignore_files;
//std::vector<std::string> read_extensions;
//
//std::optional<std::string> read_file(const std::string& path)
//{
//    // Read file contents
//    std::ifstream file(path);
//
//    if (!file.is_open())
//        return {};
//
//    // Copy file contents to text.
//    std::string text;
//
//    // Reserve space for the text
//    file.seekg(0, std::ios::end);
//    text.reserve(file.tellg());
//    file.seekg(0, std::ios::beg);
//
//    // Copy to string
//    text.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//    file.close();
//
//    return text;
//}
//
//struct FileData
//{
//    std::string name;
//    std::string extension;
//    std::string path;
//};
//
//void Parser::parse()
//{
//
//    for (const auto& directory : directories_to_read)
//    {
//        for (const auto& entry : fs::recursive_directory_iterator(fs::absolute(directory)))
//        {
//            auto path = entry.path();
//
//            current_file.extension = path.extension().string();
//            current_file.name = path.filename().replace_extension("").string();
//            current_file.path = path.string();
//
//            // Make sure it maches one of the given extensions.
//            if (!contains(read_extensions, current_file.extension) && !contains(ignore_files, current_file.name))
//                continue;
//
//            auto text = read_file(current_file.path);
//
//            if (text.has_value())
//            {
//                // tokenize
//                tokenize(*text);
//            }
//        }
//    }
//
//}
//
//namespace
//{
//    struct enum_data
//    {
//        enum_data(const std::string& name)
//            : name{ name }
//        {}
//
//        std::vector<std::string> elements;
//        std::string name;
//        std::string file;
//    };
//
//    std::vector<enum_data> enums;
//
//    // used by some sections
//    int scope_start{ 0 };
//    int scope_count{ 0 };
//
//    // some general helpers
//    Parser parser;
//
//    auto execute_node = [](FutureNode fnode, BoolNodeFn expect, VoidNodeFn execute, FutureNode next_node) { fnode.set(new ExecuteNode(expect, execute, next_node)); };
//    auto compare_node = [](FutureNode fnode, BoolNodeFn compare, FutureNode true_node, FutureNode false_node) { fnode.set(new CompareNode(compare, true_node, false_node)); };
//    auto compexe_node = [](FutureNode fnode, BoolNodeFn compare, VoidNodeFn execute, FutureNode true_node, FutureNode false_node) { fnode.set(new CompareExecuteNode(compare, execute, true_node, false_node)); };
//    auto expect_anything = [](const std::string & word) -> bool { return true; };
//    auto do_nothing = [](const std::string & word) {};
//    auto is_equal = [](const std::string& compare) { return [compare](const std::string& word) -> bool { return word == compare; }; };
//    auto is_name = [](const std::string & word) -> bool { return (word[0] > 'a' && word[0] < 'z') || (word[0] > 'A' && word[0] < 'Z'); };
//    auto is_number = [](const std::string & word) -> bool { return (word[0] > '0' && word[0] < '9'); };
//    auto is_string = [](const std::string & word) -> bool { return word[0] == '"'; };
//    auto set_starting_scope = [](const std::string & word) { scope_start = scope_count; };
//    auto check_exit_scope = [](const std::string & word) -> bool { return word[0] == '}' && scope_start > scope_count; };
//
//    auto create_enum = [](const std::string& word) { enums.push_back(word); enums.back().file = parser.get_current_file().name; };
//    auto remove_enum = [](const std::string& word) { enums.pop_back(); };
//    auto add_enum_name = [](const std::string& word) { enums.back().elements.emplace_back(word); };
//
//    auto comment_line_token_packer = [](const std::string& text, size_t index, bool& add) -> size_t 
//    { 
//        if (text.size() - index < 4) return 0;
//        if (text.substr(index, 2) == "/*")
//        {
//            size_t end{ index + 2 };
//            add = false;
//            const auto text_size = text.size() - 1;
//
//            for (; text.substr(end, 2) != "*/" && end < text_size; end++);
//            return end - index + 1;
//        }
//        else return 0;
//    };
//    auto comment_block_token_packer = [](const std::string& text, size_t index, bool& add) -> size_t 
//    { 
//        if (text.size() - index < 3) return 0;
//        if (text.substr(index, 2) == "//")
//        {
//            size_t end{ index + 2 };
//            add = false;
//            const auto text_size = text.size() - 1;
//
//            for (; text[end] != '\n' && end < text_size; end++);
//            return end - index + 1;
//        }
//        else return 0;
//    };
//    auto string_tocken_packer = [](const std::string& text, size_t index, bool& add) -> size_t 
//    { 
//        if (text.size() - index < 2) return 0;
//        if (text[index] == '\"')
//        {
//            size_t end{ index + 1 };
//            add = true;
//
//            for (; !(text[end] == '\"' && text[end - 1] != '\\');  end++);
//            return end - index + 1;
//        }
//        else return 0;
//    };
//    auto custom_tocken_checker = [](char character)
//    {
//        if (character == '{') scope_count++;
//        else if (character == '}') scope_count--;
//    };
//
//    void parse_files();
//
//    const char* header_comment
//    {
//        " //------------------------------------------------------------------------------  \n"
//        " // <auto-generated>                                                               \n"
//        " //    This code was generated from a parser.                                      \n"
//        " //                                                                                \n"
//        " //    Manual changes to this file will be overwritten if the code is regenerated. \n"
//        " // </auto-generated>                                                              \n"
//        " //------------------------------------------------------------------------------  \n"
//    };
//}
//
//int main()
//{
//    parse_files();
//}
//
//namespace
//{
//    void parse_files()
//    {
//        // -------------------------------------------- parser settings --------------------------------------------
//        parser.ignore_files = {};
//        parser.ignore_folders = {};
//        parser.read_extensions = { ".h", ".cpp" };
//        parser.directories_to_read = { "tests" };
//        parser.remove_tokens = { ' ' , '\t', '\n' };
//        parser.insert_tokens = { ':', ',', ';', '{', '}', '(', ')', '<', '>', '*', '&', '=' };
//        parser.token_packers = { comment_line_token_packer, comment_block_token_packer, string_tocken_packer };
//        parser.custom_token_checker = custom_tocken_checker;
//
//        //// ----------- property segment --------------------------------------------
//        //compexe_node(ai, equal_string("{"), set_starting_scope);
//        //compexe_node(aj, equal_string("property"), create_property);
//        //compare_node(ak, check_exit_scope);
//        //compexe_node(al, equal_string("with"), create_property);
//        //execute_node(am, expect_anything, save_property_name);
//        //compare_node(an, equal_string(";"));
//        //compare_node(ap, equal_string("{"));
//        //compexe_node(aae, equal_string("("), set_property_as_function);
//        //compare_node(aaf, equal_string(";"));
//        //execute_node(aag, expect_anything, do_nothing);
//        //execute_node(aq, is_name, add_argument);
//        //compare_node(ar, equal_string(","));
//        //compexe_node(as, is_number, add_numeric_argument);
//        //compexe_node(aad, is_name, add_type_argument);
//        //execute_node(at, is_string, add_string_argument);
//        //execute_node(au, expect_anything, do_nothing);
//        //compare_node(av, equal_string("="));
//        //execute_node(aw, equal_string("("), do_nothing);
//        //execute_node(ax, equal_string(";"), do_nothing);
//        //execute_node(az, equal_string(")"), do_nothing);
//        //execute_node(aaa, equal_string("property"), do_nothing);
//        //compare_node(aab, equal_string("}"));
//        //execute_node(aac, expect_anything, add_default_property_value);
//        //ai.true_node = &al;
//        //aj.false_node = &ak;
//        //aj.true_node = &am;
//        //ak.false_node = &au;
//        //au.next_node = &al;
//        //al.false_node = &aj;
//        //al.true_node = &aw;
//        //aw.next_node = &aq;
//        //am.next_node = &an;
//        //an.false_node = &ap;
//        //an.true_node = &al;
//        //ap.false_node = &aae;
//        //ap.true_node = &aab;
//        //aae.false_node = &am;
//        //aae.true_node = &aaf;
//        //aaf.true_node = &al;
//        //aaf.false_node = &aag;
//        //aag.next_node = &aaf;
//        //aab.false_node = &aac;
//        //aab.true_node = &ax;
//        //aac.next_node = &aab;
//        //ax.next_node = &al;
//        //aq.next_node = &ar;
//        //ar.true_node = &aq;
//        //ar.false_node = &av;
//        //av.false_node = &az;
//        //az.next_node = &aaa;
//        //av.true_node = &as;
//        //as.false_node = &aad;
//        //as.true_node = &ar;
//        //aad.false_node = &at;
//        //aad.true_node = &ar;
//        //at.next_node = &ar;
//        //aaa.next_node = &am;
//        //
//        //// -------------------------------------------- container stream flow --------------------------------------------
//        //flow container_flow;
//        //compare_node(ca, equal_string("container"));
//        //execute_node(cb, is_name, create_container);
//        //compexe_node(cc, equal_string("manager"), set_as_manager);
//        //compare_node(cd, equal_string("extensions"));
//        //execute_node(ce, equal_string("("), do_nothing);
//        //execute_node(cf, is_name, set_manager_extension);
//        //execute_node(cg, equal_string(")"), do_nothing);
//        //container_flow.start = &ca;
//        //ca.true_node = &cb;
//        //cb.next_node = &cc;
//        //cc.false_node = &ai;
//        //cc.true_node = &cd;
//        //cd.false_node = &ai;
//        //cd.true_node = &ce;
//        //ce.next_node = &cf;
//        //cf.next_node = &cg;
//        //cg.next_node = &ai;
//        //
//        //// -------------------------------------------- component stream flow --------------------------------------------
//        //flow component_flow;
//        //compare_node(da, equal_string("component"));
//        //execute_node(db, is_name, create_component);
//        //compare_node(dc, equal_string("with"));
//        //execute_node(dd, equal_string("("), do_nothing);
//        //execute_node(de, is_name, add_dependency_component);
//        //compare_node(df, equal_string(","));
//        //execute_node(dg, equal_string(")"), do_nothing);
//        //compexe_node(dh, equal_string("owner_knowledge"), set_owner_knowledge);
//        //component_flow.start = &da;
//        //da.true_node = &db;
//        //db.next_node = &dc;
//        //dc.true_node = &dd;
//        //dc.false_node = &dh;
//        //dd.next_node = &de;
//        //de.next_node = &df;
//        //df.true_node = &de;
//        //df.false_node = &dg;
//        //dg.next_node = &dh;
//        //dh.true_node = dh.false_node = &ai;
//        //
//        //// -------------------------------------------- resource stream flow --------------------------------------------
//        //flow resource_flow;
//        //compare_node(ea, equal_string("resource"));
//        //execute_node(eb, is_name, create_resource);
//        //execute_node(ec, equal_string("extensions"), do_nothing);
//        //execute_node(ed, equal_string("("), do_nothing);
//        //execute_node(ee, is_name, add_extension);
//        //compare_node(ef, equal_string(","));
//        //execute_node(eg, equal_string(")"), do_nothing);
//        //resource_flow.start = &ea;
//        //ea.true_node = &eb;
//        //eb.next_node = &ec;
//        //ec.next_node = &ed;
//        //ed.next_node = &ee;
//        //ee.next_node = &ef;
//        //ef.true_node = &ee;
//        //ef.false_node = &eg;
//        //eg.next_node = &ai;
//
//        
//        // ------------------------------------------- Enum Flow ----------------------------------------------
//        {
//            FutureNode A, B, C, D, E, F, G, H, I, J, K, null;
//            compare_node(A, is_equal("enum"), B, null);
//            execute_node(B, is_name, create_enum, C);
//            compexe_node(C, is_equal(";"), remove_enum, null, D);
//            compare_node(D, is_equal(":"), K, J);
//            execute_node(K, is_name, do_nothing, J);
//            execute_node(J, is_equal("{"), do_nothing, E);
//            execute_node(E, is_name, add_enum_name, F);
//            compare_node(F, is_equal("="), I, G);
//            compare_node(G, is_equal(","), E, H);
//            execute_node(H, is_equal("}"), do_nothing, G);
//            execute_node(I, expect_anything, do_nothing, null);
//            parser.flows.emplace_back(A);
//        }
//
//        // Execute parser.
//        parser.parse();
//    }
//}