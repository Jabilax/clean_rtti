#include "reflection.h"
#include <iostream>
#include <string>
#include <cassert>
#include <vector>

template<class T>
auto display_class(const T& object)
{
    std::cout << "Class: " << Reflect<T>::name << "\n";

    for (auto var : Reflect<T>::variables)
    {
        std::cout << " * Variable: " << var.name << "\n"; // Display name.

        for (const auto attribute : var.attributes())
        {
            std::cout << "   - Attibute: " << attribute.name << "\n";
        }

        var.get(object, [](const auto& var)
        {
            std::cout << "   - Value: " << var << "\n";
        });
    }
}

struct [[component]] Person
{
    [[do_not_serialize]]
    std::string name; // Without surname.
    int age;
};

//struct Manager
//{
//private:
//    [[do_not_serialize]]
//    std::string window_name{"my_window"};
//
//    [[readonly]]
//    std::string title;
//
//    int foo();
//
//    bool is_active;
//    float padding;
//    long long uid;
//
//    [[event_update]]
//    auto another_function() -> int;
//
//
//    friend class MemberVariable<Manager>;
//};

int main()
{
    Person person{};
    person.name = "Sherlock";
    person.age = 38;

    display_class(person);
    //display_class(Manager{});
}
