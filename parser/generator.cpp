#include "parser/generator.h"
#include "parser/parser.h"
#include <fstream>
#include <string>


const char* file_comment
{
    " //------------------------------------------------------------------------------  \n"
    " // <auto-generated>                                                               \n"
    " //    This code was generated from a parser.                                      \n"
    " //                                                                                \n"
    " //    Manual changes to this file will be overwritten if the code is regenerated. \n"
    " // </auto-generated>                                                              \n"
    " //------------------------------------------------------------------------------  \n"
};

void generate_reflection_info(std::ofstream& file, const ParsedData& data);

void generate_file_reflection_info(const fs::path& path, ParsedData& parsing_data)
{
    // Get the name of the meta file.
    const std::string root = "F:\\others\\clean_rtti\\include\\generated\\";
    const auto filename = "reflection_generated_" + path.stem().string() + ".h";

    // Open the meta file and write the autogenerated header comment.
    const auto reflection_file_path = root + filename;
    std::ofstream reflection_file{ reflection_file_path };
    reflection_file << file_comment;
    generate_reflection_info(reflection_file, parsing_data);
}

void generate_file_reflection_header(const std::vector<fs::path>& files)
{
    const std::string root = "F:\\others\\clean_rtti\\include\\generated\\";
    const auto filename = "reflection_generated.h";

    // Open the meta file and write the autogenerated header comment.
    const auto reflection_file_path = root + filename;
    std::ofstream reflection_file{ reflection_file_path };
    reflection_file << file_comment;
    reflection_file << "#pragma once" << "\n\n";

    for (auto file : files)
    {
        const auto meta_file = "reflection_generated_" + file.stem().string() + ".h";
        reflection_file << "#include \"" << meta_file << "\"\n";
    }
}



std::size_t replace_all(std::string& inout, std::string_view what, std::string_view with)
{
    std::size_t count{};
    for (std::string::size_type pos{}; inout.npos != (pos = inout.find(what.data(), pos, what.length())); pos += with.length(), ++count)
    {
        inout.replace(pos, what.length(), with.data(), with.length());
    }
    return count;
}

// Forward declarations
// ------------------------------------------------------------------------------------
void generate_reflection_info(std::ofstream& file, const ParsedData& data);
void generate_enum_reflection_info(std::ofstream& file, const ParsedData& data);
void generate_struct_reflection_info(std::ofstream& file, const ParsedData& data);

// struct generators
void generate_forward_declare(std::ofstream& file, const Struct& _struct);
void generate_name(std::ofstream& file, const Struct& _struct);
void generate_variable_num(std::ofstream& file, const Struct& _struct);
void generate_variable_name(std::ofstream& file, const Struct& _struct);
void generate_variable(std::ofstream& file, const Struct& _struct);
void generate_variable_apply(std::ofstream& file, const Struct& _struct);
void generate_function_num(std::ofstream& file, const Struct& _struct);
void generate_function_name(std::ofstream& file, const Struct& _struct);
void generate_function_call(std::ofstream& file, const Struct& _struct);

// Helpers
auto type_name(const Struct& _struct) -> std::string;
// ------------------------------------------------------------------------------------

void generate_reflection_info(std::ofstream& file, const ParsedData& data)
{
    file << "#pragma once\n";
    file << "#include \"../reflection_forward.h\"\n\n";

    generate_enum_reflection_info(file, data);
    generate_struct_reflection_info(file, data);
}

void generate_enum_reflection_info(std::ofstream& /*file*/, const ParsedData& /*data*/)
{
}

void generate_struct_reflection_info(std::ofstream& file, const ParsedData& data)
{
    for (auto& _struct : data.structs)
    {
        generate_forward_declare(file, _struct);

        file << "template<class T>\n";
        file << "struct Reflect<" << type_name(_struct) << ", T>\n";
        file << "{\n";

        generate_name(file, _struct);
        generate_variable_num(file, _struct);
        generate_variable_name(file, _struct);
        generate_variable(file, _struct);
        generate_variable_apply(file, _struct);
        generate_function_num(file, _struct);
        generate_function_name(file, _struct);
        generate_function_call(file, _struct);

        file << "};\n\n\n";
    }
}

void generate_forward_declare(std::ofstream& file, const Struct& _struct)
{
    if (!_struct.namespace_scope.empty())
    {
        file << "namespace " << _struct.namespace_scope << " { " << _struct.type << " " << _struct.name << "; }" << "\n";
    }
    else
    {
        file << _struct.type << " " << _struct.name << ";" << "\n";
    }

    file << "\n";
}

void generate_name(std::ofstream& file, const Struct& _struct)
{
    file << "    static auto name() -> std::string\n";
    file << "    {\n";
    file << "        return \"" << _struct.name << "\";\n";
    file << "    }\n\n";
}

void generate_variable_num(std::ofstream& file, const Struct& _struct)
{
    file << "    static auto variable_num() -> int\n";
    file << "    {\n";
    file << "        return " << _struct.variables.size() << ";\n";
    file << "    }\n\n";
}

void generate_variable_name(std::ofstream& file, const Struct& _struct)
{
    file << "    static auto variable_name(int index) -> std::string\n";
    file << "    {\n";
    file << "        std::string variable_names[]\n";
    file << "        {\n";
    for (const auto& var : _struct.variables)
    file << "            \"" << var.name << "\",\n";
    file << "        };\n";
    file << "        return variable_names[index];\n";
    file << "    }\n\n";
}

void generate_variable(std::ofstream& file, const Struct& _struct)
{
    file << "    template<class Var>\n";
    file << "    static auto variable(T& instance, int index) -> Var&\n";
    file << "    {\n";
    file << "        static std::function<std::any(T&)> variables[]\n";
    file << "        {\n";
    for (const auto& var : _struct.variables)
    file << "            [] (T& i) { return std::any(&i." << var.name << "); },\n";
    file << "        };\n";
    file << "        return *std::any_cast<Var*>(variables[index](instance));\n";
    file << "    }\n\n";
}

void generate_variable_apply(std::ofstream& file, const Struct& _struct)
{
    file << "    template<class Fn>\n";
    file << "    static void variable_apply(T & instance, int index, Fn function)\n";
    file << "    {\n";
    file << "        static std::function<void(T&, Fn)> apply_variables[]\n";
    file << "        {\n";
    for (const auto& var : _struct.variables)
    file << "            [](T& i, Fn fn) { return fn(i." << var.name << "); },\n";
    file << "        };\n";
    file << "        return apply_variables[index](instance, function);\n";
    file << "    }\n\n";
}

void generate_function_num(std::ofstream& file, const Struct& _struct)
{
    file << "    static auto function_num() -> int\n";
    file << "    {\n";
    file << "        return " << _struct.functions.size() << ";\n";
    file << "    }\n\n";
}

void generate_function_name(std::ofstream& file, const Struct& _struct)
{
    file << "    static auto function_name(int index) -> std::string\n";
    file << "    {\n";
    file << "        std::string function_names[]\n";
    file << "        {\n";
    for (const auto& function : _struct.functions)
        file << "            \"" << function.name << "\",\n";
    file << "        };\n";
    file << "        return function_names[index];\n";
    file << "    }\n\n";
}

void generate_function_call(std::ofstream& file, const Struct& _struct)
{
    file << "    template<class Ret, typename... Args>\n";
    file << "    static auto function_call(T& instance, int index, Args&&... args) -> Ret\n";
    file << "    {\n";
    file << "        static std::function<std::any(T&, Args&&... args)> call_functions[]\n";
    file << "        {\n";
    for (const auto& function : _struct.functions)
    file << "            [](T& i, Args&&... args){ return call_member_function<decltype(&T::" << function.name << "), T, Args...>(&T::" << function.name << ", i, std::forward<Args>(args)...); },\n";
    file << "        };\n";
    file << "        return std::any_cast<Ret>(call_functions[index](instance, std::forward<Args>(args)...));\n";
    file << "    }\n\n";
}

auto type_name(const Struct& _struct) -> std::string
{
    if (_struct.namespace_scope.empty())
    {
        return _struct.name;
    }
    else
    {
        return _struct.namespace_scope + "::" + _struct.name;
    }
}