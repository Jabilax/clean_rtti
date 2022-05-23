#pragma once
#include <map>
#include <string>
#include <vector>
#include <type_traits>

template<class T> class ReflectInstance;
template<class T> class ReflectType;
template<class T> class ReflectMemberVariable;
template<class T> class ReflectMemberVariableType;
template<class T> class ReflectMemberFunction;
template<class T> class ReflectMemberFunctionType;
struct Attribute;


using AttributeMap = std::map<std::string, Attribute>;

template<class T, class T2, class Ret>
using Specialize = typename std::enable_if_t<std::is_same_v<T, T2>, Ret>;

template<class T, class T2, int i, int i2, class Ret>
using SpecializeIndex = typename std::enable_if_t<std::is_same_v<T, T2> && i == i2, Ret>;
