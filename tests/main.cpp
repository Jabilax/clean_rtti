#include <iostream>
#include <array>


template<class T, class T2 = void> struct Reflect {};
template<class T, class T2 = void> struct MemberVariable {};
template<class T, class T2 = void> struct EnumValue {};

enum Colours : int;

template<class T2>
struct Reflect<Colours, T2>
{
    static constexpr const char* name{ "Colours" };
    static constexpr std::array<EnumValue<Colours>, 2> values
    {
        EnumValue<Colours>{"first_value", 0},
        EnumValue<Colours>{"second_value", 1}
    };
};

template<class T2>
struct EnumValue<Colours, T2>
{
    constexpr EnumValue(const char* name, int index) : name{ name }, index{ index }{}

    template<class T = Colours, class Fn>
    void apply(Fn callback) const
    {
        switch (index)
        {
        case 0: callback(T::first_value); break;
        case 1: callback(T::second_value); break;
        }
    }

public:  const char* name;
private: int index;
};

enum Colours { first_value, second_value = 10 };

int main()
{
    for (auto var : Reflect<Colours>::values)
    {
        std::cout << var.name << ": ";
        var.apply([](auto value) { std::cout << value << "\n"; });
    }
}