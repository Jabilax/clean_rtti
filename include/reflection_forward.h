#pragma once
#include <any>
#include <map>
#include <functional>
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

// Internal Use
template<class T, class T2 = T> class Reflect;
