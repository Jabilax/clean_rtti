#include <iostream>
#include <array>

/*template<class T2, class T = T2>
struct Reflect
{};

template<class T2, class T = T2>
struct MemberVariable
{};

struct MyStruct;

template<class T>
struct MemberVariable<MyStruct, T>
{
    constexpr MemberVariable(const char* name, int index) : name{ name }, index{ index } {}

    template<class Fn>
    void apply(T& object, Fn callback) const
    {
        switch (index)
        {
        case 0: callback(object.first_var); break;
        case 1: callback(object.second_var); break;
        }
    }

    const char* name;

private:
    int index;
};

template<class T>
struct Reflect<MyStruct, T>
{
    static constexpr const char* name{ "MyStruct" };
    static constexpr std::array<MemberVariable<MyStruct>, 2> variables
    {
        MemberVariable<MyStruct>{"first_var", 0},
        MemberVariable<MyStruct>{"second_var", 1}
    };
};*/


#include "reflection.h"

struct MyStruct
{
    int first_var{ 2 };
    float second_var{ 3.f }; /*some comment*/
    const int* ptr = nullptr;

};

int main()
{
    MyStruct object{};

    for (const auto& var : Reflect<MyStruct>::variables)
    {
        std::cout << var.name << ": ";

        var.apply(object, [](auto& var)
        {
            std::cout << var << std::endl; // Do anything with the var here.
        });
    }
}
