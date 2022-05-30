#include "include/reflection.h"
#include <iostream>

enum editor
{
    show,
    serialize
};

attribute(editor::show("name", 5.f), editor::serialize)
struct Person
{
    attribute(some_attribute, hello)
    std::string name{ "this is my name" };
    int age;

    void foo() { { std::cout << "Toby"; } }
    auto get_name() -> std::string { return "MyName"; }
    void bar() {}

    attribute(editor::serialize(5.f))
    float my_array[2];
};

struct Human
{
    int some_variable;
    float slider;
};

struct Alien
{
};

int main()
{
    Person person;
    person.age = 5;

    //auto person_reflect = reflect(person);
    //auto name = reflect(person).name();
    //auto name2 = reflect<Human>().name();
    //
    //for (auto& var : reflect<Human>().variables())
    //{
    //    std::cout << var.name() << std::endl;
    //}
    //
    //for (auto& var : reflect<Person>().variables())
    //{
    //    var.apply(person, [](auto& var)
    //    {
    //        std::cout << var << std::endl;
    //    });
    //}

    for (auto& attribute : reflect(person).variable_by_name("my_array").attributes())
    {
        std::cout << std::any_cast<float>(attribute.second[0]) << std::endl;
    }


    //reflect(person).function_by_name("foo").call();
    //auto value = std::any_cast<float>(reflect(person).attributes()["editor::show"][1]);
    //std::cout << string << std::endl;

    //std::cout << value << std::endl;
    //std::cout << reflect<Person>().variable("name") << std::endl;


    //for (auto var : reflect(person).variables())
    //{
    //    std::cout << var.name << "\n";
    //}

    //for (auto var_name : { "name", "age" })
    //{
    //    auto person_reflect = Reflect(person);
    //
    //    if (!Reflect{ person }::variable(var_name).attributes.empty())
    //    {
    //        std::cout << var_name << " has attributes \n";
    //    }
    //    
    //}


    //for (auto& var : Reflect<Person>::variables)
    //{
    //    var.apply(person, [&var](auto& v)
    //    {
    //        std::cout << var.name << ": " << v << std::endl;
    //    });
    //
    //    if (var.name == "age")
    //    {
    //        std::cout << "my actual age is: " << var.get<int>(person) + 2 << std::endl;
    //    }
    //}

    //Reflect<Person>::call("bar", person);
    //std::cout << "my dog name is: " << Reflect<Person>::call<std::string>("foo", person) << std::endl;
    //Reflect<Person>::call<std::string>("foo", person);
    //
    //for (auto& function : Reflect<Person>::functions)
    //{
    //    std::cout << "function: " << function.name << std::endl;
    //
    //    //var.apply(person, [&var](auto& v)
    //    //{
    //    //    std::cout << var.name << ": " << v << std::endl;
    //    //});
    //    //
    //    //if (var.name == "age")
    //    //{
    //    //    std::cout << "my actual age is: " << var.get<int>(person) + 2 << std::endl;
    //    //}
    //}

    //std::cout << "my pet name is: " Reflect<Person>::get


    //display_class(person);
    //display_class(Manager{});

    //system("pause");
}
