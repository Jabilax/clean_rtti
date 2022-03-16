 //------------------------------------------------------------------------------  
 // <auto-generated>                                                               
 //    This code was generated from a parser.                                      
 //                                                                                
 //    Manual changes to this file will be overwritten if the code is regenerated. 
 // </auto-generated>                                                              
 //------------------------------------------------------------------------------  

#pragma once
#include <array>
#include <vector>

template<class T2, class T = T2> class Reflect {};
struct Attribute;
using Args = std::vector<const char*>;
using Attributes = std::vector<Attribute>;

struct Attribute
{
    const char* name;
    std::vector<const char*> arguments;
};

template<class T> struct EnumValue 
{
    const char* name;
    T value; 
};

template<class T>
class MemberVariable
{
public:
    constexpr MemberVariable(int index, const char* name, Attributes&& attribute)
        : name{ name }
        , index{ index }
        , attributes{ attributes }
    {
    }

    template<class Fn>
    auto get(const T& object, Fn callback)
    { 
        return Reflect<T>::get(object, index, callback);
    }

public:
    const char* name;
    Attributes attributes;

private:
    int index;
};

template<class T>
class MemberFunction
{
public:
    constexpr MemberFunction(int index, const char* name, Attributes&& attribute)
        : name{ name }
        , index{ index }
        , attributes{ attributes }
    {
    }

    template<class Fn>
    auto call(const T& object, Fn callback)
    {
        return Reflect<T>::get(object, index, callback);
    }

public:
    const char* name;
    Attributes attributes;

private:
    int index;
};

struct Person;

template<class T>
class Reflect<Person, T>
{
public:
    static constexpr const char* name{ "Person" };
    static constexpr std::array<MemberVariable<T>, 2> variables
    {
        MemberVariable<T>{0, "name", Attributes{ Attribute{"do_not_serialize", Args{}} }},
        MemberVariable<T>{1, "age",  Attributes{}},
    };

private:
    template<class Fn>
    static void get(const T& object, int var_index, Fn callback) const
    {
        switch (var_index)
        {
        case 0: callback(object.name); return;
        case 1: callback(object.age); return;
        }
    }

    friend class MemberVariable<Person>;
};
