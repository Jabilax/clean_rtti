//#include "reflection.h"
//#include <iostream>
//#include <string>
//#include <cassert>
//#include <vector>
//
//template<class T>
//auto display_class(const T& object)
//{
//    std::cout << "Class: " << Reflect<T>::name << "\n";
//
//    for (auto var : Reflect<T>::variables)
//    {
//        std::cout << " * Variable: " << var.name << "\n"; // Display name.
//
//        for (const auto attribute : var.attributes())
//        {
//            std::cout << "   - Attibute: " << attribute.name << "\n";
//        }
//
//        var.get(object, [](const auto& var)
//        {
//            std::cout << "   - Value: " << var << "\n";
//        });
//    }
//}

#include "base_reflection.h"
#include <iostream>

struct Person
{
    std::string name{"this is my name"};
    int age;

    void foo() { std::cout << "Toby"; };
    void bar(){};
};

int main()
{
    Person person{};
    //person.name = "Sherlock";
    person.age = 38;

    //auto var = Reflect<Person>::get_variable("name");
    //std::cout << var.get<std::string>(person) << std::endl;
    //std::cout << Reflect<Person>::get_variable<int>("age", person) << std::endl;

    for (auto& var : Reflect<Person>::variables)
    {
        var.apply(person, [&var](auto& v)
        {
            std::cout << var.name << ": " << v << std::endl;
        });

        if (var.name == "age")
        {
            std::cout << "my actual age is: " << var.get<int>(person) + 2 << std::endl;
        }
    }

    //Reflect<Person>::call("bar", person);
    //std::cout << "my dog name is: " << Reflect<Person>::call<std::string>("foo", person) << std::endl;
    Reflect<Person>::call<std::string>("foo", person);

    for (auto& function : Reflect<Person>::functions)
    {
        std::cout << "function: " << function.name << std::endl;

        //var.apply(person, [&var](auto& v)
        //{
        //    std::cout << var.name << ": " << v << std::endl;
        //});
        //
        //if (var.name == "age")
        //{
        //    std::cout << "my actual age is: " << var.get<int>(person) + 2 << std::endl;
        //}
    }

    //std::cout << "my pet name is: " Reflect<Person>::get


    //display_class(person);
    //display_class(Manager{});
}
