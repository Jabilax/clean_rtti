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
    parser.parse_file("C:\\Repositories\\clean_rtti\\tests\\enums.cpp");
    //parser.parse_file("C:\\Repositories\\clean_rtti\\source\\generic_parser\\flow.h");

    return 0;
}
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