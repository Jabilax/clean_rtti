#pragma once
#include "reflection_forward.h"
#include "generated/reflection_generated.h"

// Reflect Interface
// ----------------------------------------------------------------------------------------------
template<class T> auto reflect() -> ReflectType<T>;
template<class T> auto reflect(T& instance) -> ReflectInstance<T>;
//template<class T> auto reflect(T& instance) -> ReflectInstance<T>;
//template<class T> auto reflect(const T& instance) -> const ConstReflectInstance<T>;


// ReflectMemberVariableType
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectMemberVariableType
{
public:
    ReflectMemberVariableType(int index);

    auto name() const -> std::string;

    // auto attributes() const -> AttributeMap; Todo

    template<class Var>
    auto value(T& instance) -> Var&;

    template<class Var>
    auto value(const T& instance) const -> const Var&;

    template<class Fn>
    void apply(T& instance, Fn function);

    template<class Fn>
    void apply(const T& instance, Fn function) const;

private:
    int index;
};


// ReflectMemberVariable
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectMemberVariable
{
public:
    ReflectMemberVariable(T& instance, int index);

    auto name() const -> std::string;

    // auto attributes() const -> AttributeMap; Todo

    template<class Var>
    auto value() -> Var&;

    template<class Var>
    auto value() const -> const Var&;

    template<class Fn>
    void apply(Fn function);

    template<class Fn>
    void apply(Fn function) const;

private:
    T& instance;
    int index;
};

// ReflectMemberFunctionType
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectMemberFunctionType
{
public:
    ReflectMemberFunctionType(int index);

    auto name() const -> std::string;

    // auto attributes() const -> AttributeMap; Todo



private:
    int index;
};

// ReflectType
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectType
{
public:
    // Info about the type.
    auto name() const -> std::string;
    auto attributes() const -> AttributeMap;

    // List of members. (returns a std::array& of the members, not defined since we don't yet know the size)
    auto variables() const -> std::vector<ReflectMemberVariableType<T>>;
    auto functions() const -> std::vector<ReflectMemberFunctionType<T>>;

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
    ReflectInstance(T& instance);

    // Info about the type.
    auto name() const -> std::string;
    auto attributes() const -> AttributeMap;

    // List of members.
    auto variables() -> std::vector<ReflectMemberVariable<T>>;
    // auto variables() const; // TODO
    auto functions() -> std::vector<ReflectMemberFunction<T>>;
    // auto functions() const; // -> const std::array<MemberFunction, ?>&

    // Info of member by name.
    auto variable(const std::string& name) -> ReflectMemberVariable<T>&;
    // auto variable(const std::string& name) const -> const ReflectMemberVariable<T>&; Todo
    auto function(const std::string& name) -> ReflectMemberFunction<T>&;
    // auto function(const std::string& name) const -> const ReflectMemberFunction<T>&; Todo

private:
    T& instance;
};

#include "reflection.inl"