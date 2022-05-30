#pragma once
#include <vector>
#include <string>

struct Attribute
{
    std::string name;
    std::vector<std::string> arguments;
};

struct Enum
{
    std::string name;
    bool is_enum_class;
    std::string type;
    std::string namespace_scope;
    std::vector<std::string> values;
};

struct Namespace
{
    std::string name;
    int scope;
};

struct MemberVariable
{
    std::string name;
    std::vector<Attribute> attributes;
    bool is_public;
};

struct MemberFunction
{
    std::string name;
    std::vector<Attribute> attributes;
    bool is_public;
};

struct Class
{
    std::string name;
    std::string type;
    std::vector<Attribute> attributes;
    std::string _namespace;
    std::vector<MemberVariable> variables;
    std::vector<MemberFunction> functions;
};

struct ParsedData
{
    std::vector<Enum> enums;
    std::vector<Class> classes;
};