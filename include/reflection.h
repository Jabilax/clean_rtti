#pragma once
#include "generated/reflection_generated.h"

// Reflect Interface
// ----------------------------------------------------------------------------------------------
template<class T> auto reflect() -> ReflectType<T>;
template<class T> auto reflect(T instance) -> ReflectInstance<T>;
//template<class T> auto reflect(T& instance) -> ReflectInstance<T>;
//template<class T> auto reflect(const T& instance) -> const ConstReflectInstance<T>;


// ReflectType
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectMemberVariableType
{
public:
    ReflectMemberVariableType(int index);

    auto name() const -> std::string;
    auto attributes() const -> AttributeMap;

private:
    int index;
};

// ReflectType
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectMemberFunctionType
{
public:
    ReflectMemberFunctionType(int index);

    auto name() const -> std::string;
    auto attributes() const -> AttributeMap;

private:
    int index;
};

// ReflectType
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectType
{
public:
    using Type = T;

    // Info about the type.
    auto name() const -> std::string;
    auto attributes() const -> AttributeMap;

    // List of members. (returns a std::array& of the members, not defined since we don't yet know the size)
    auto variables() const;
    auto functions() const;

    // Info of member by name.
    auto variable(const std::string& name) -> const ReflectMemberVariable<T>&;
    auto function(const std::string& name) -> const ReflectMemberFunction<T>&;
};


// ReflectInstance
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectInstance
{
public:
    // Constructor
    ReflectInstance(T class_instance);

    // Info about the type.
    auto name() const -> std::string;
    auto attributes() const -> AttributeMap;

    // List of members.
    auto variables(); // -> std::array<MemberVariable, ?>&
    auto variables() const; // -> const std::array<MemberVariable, ?>&
    auto functions(); // -> std::array<MemberFunction, ?>&
    auto functions() const; // -> const std::array<MemberFunction, ?>&

    // Info of memeber by name.
    auto variable(const std::string& name) -> ReflectMemberVariable<T>&;
    auto variable(const std::string& name) const -> const ReflectMemberVariable<T>&;
    auto function(const std::string& name) -> ReflectMemberFunction<T>&;
    auto function(const std::string& name) const -> const ReflectMemberFunction<T>&;

    // Access the member by name.
    template<class S>  auto variable(const std::string& name) -> const S&;
    template<class... ArgTypes, class R = void> R call(const std::string& name, ArgTypes... args);

    // Pass the member into the given function.
    template<class Fn> void apply_variable(const std::string& name, Fn func);
    template<class Fn> void apply_function(const std::string& name, Fn func);

private:
    T instance;
};

#include "reflection.inl"