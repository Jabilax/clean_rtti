#pragma once
#include <any>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

template<class T> class ReflectClass;
template<class T> class ReflectClassInstance;
template<class T> class ReflectMemberVariable;
template<class T> class ReflectMemberVariableInstance;
template<class T> class ReflectMemberFunction;
template<class T> class ReflectMemberFunctionInstance;
struct Attribute;


using AttributeMap = std::unordered_map<std::string, Attribute>;

// Internal Use
template<class T, class T2 = T> class Reflect;
