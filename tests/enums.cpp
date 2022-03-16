#include "reflection.h"
#include <string>
#include <iostream>

enum Colours
{ 
    Red,
    Orange = 10,
    Yellow = 5 // Some comment
};

void test(const std::string& test, bool assert)
{
    std::cout << (assert ? "Succeed" : "Failed") << ": Test " << test << "\n";
}

enum class People{Jeremy, /* test comment*/ Boris};

namespace Widgets
{
    enum Types { filter, timeline, shader_graph, colour_picker };
}

// some comment
namespace Widgets::Canvas
{
    enum Types { point, line, curve };
}

// some comment
enum EnumWithCustomType : short { first, second, third };


void enum_tests()
{
    test("Simple Enum Name", Reflect<Colours>::name == "Colours");
    test("Simple Enum Values Size", Reflect<Colours>::values.size() == 3);
    test("Simple Enum Values", Reflect<Colours>::values[0].name == "Red" 
                            && Reflect<Colours>::values[1].name == "Orange" 
                            && Reflect<Colours>::values[2].name == "Yellow");

    test("Class Enum Name", Reflect<People>::name == "People");
    test("Class Enum Values Size", Reflect<People>::values.size() == 2);
    test("Class Enum Values", Reflect<People>::values[0].name == "Jeremy"
                           && Reflect<People>::values[1].name == "Boris");

    test("Namespace Enum Name", Reflect<Widgets::Types>::name == "Types");
    test("Namespace Enum Values Size", Reflect<Widgets::Types>::values.size() == 4);
    test("Namespace Enum Values", Reflect<Widgets::Types>::values[0].name == "filter"
                               && Reflect<Widgets::Types>::values[1].name == "timeline"
                               && Reflect<Widgets::Types>::values[2].name == "shader_graph"
                               && Reflect<Widgets::Types>::values[3].name == "colour_picker");

    test("Nested Namespace Enum Name", Reflect<Widgets::Canvas::Types>::name == "Types");
    test("Nested Namespace Enum Values Size", Reflect<Widgets::Canvas::Types>::values.size() == 3);
    test("Nested Namespace Enum Values", Reflect<Widgets::Canvas::Types>::values[0].name == "point"
                                      && Reflect<Widgets::Canvas::Types>::values[1].name == "line"
                                      && Reflect<Widgets::Canvas::Types>::values[2].name == "curve");

    test("Custom Type Enum Name", Reflect<EnumWithCustomType>::name == "EnumWithCustomType");
    test("Custom Type Enum Values Size", Reflect<EnumWithCustomType>::values.size() == 3);
    test("Custom Type Enum Values", Reflect<EnumWithCustomType>::values[0].name == "first"
                                 && Reflect<EnumWithCustomType>::values[1].name == "second"
                                 && Reflect<EnumWithCustomType>::values[2].name == "third");
}

int main()
{
    enum_tests();
}