#pragma once
#include "reflection_forward.h"
#include "generated/reflection_generated.h"

// Reflect Interface
// ----------------------------------------------------------------------------------------------
template<class T> auto reflect() -> ReflectClass<T>;
template<class T> auto reflect(T& instance) -> ReflectClassInstance<T>;
//template<class T> auto reflect(T& instance) -> ReflectInstance<T>;
//template<class T> auto reflect(const T& instance) -> const ConstReflectInstance<T>;

// ReflectClass
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectClass
{
public:
    // Info about the type
    static auto name() -> std::string;
    static auto attributes() -> AttributeMap;

    // List of members (returns a std::array& of the members, not defined since we don't yet know the size)
    static auto variables() -> const std::vector<ReflectMemberVariable<T>>&;
    static auto functions() -> const std::vector<ReflectMemberFunction<T>>&;

    // Info of member by name
    static auto variable_by_name(const std::string& name) -> ReflectMemberVariable<T>;
    static auto function_by_name(const std::string& name) -> ReflectMemberFunction<T>;
};

// ReflectMemberVariable
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectMemberVariable
{
public:
    // Constructor
    ReflectMemberVariable(int index);

    // Info about the type
    auto name() const -> std::string;
    // auto attributes() const -> AttributeMap; Todo

    // Get value
    template<class Var> auto value(T& instance) const -> Var&;
    template<class Var> auto value(const T& instance) const -> const Var&;

    // Pass value to callback
    template<class Fn> void apply(T& instance, Fn function) const;
    template<class Fn> void apply(const T& instance, Fn function) const;

private:
    int index;

    friend class ReflectClassInstance<T>;
};


// ReflectMemberFunction
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectMemberFunction
{
public:
    // Constructor
    ReflectMemberFunction(int index);

    // Info about the type
    auto name() const -> std::string;
    //  auto attributes() const -> AttributeMap; TODO

    template<class Ret = void, typename... Args>
    auto call(T& instance, Args&&... args) -> Ret;

private:
    int index;

    friend class ReflectClassInstance<T>;
};




// ReflectInstance
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectClassInstance
{
public:
    // Constructor
    ReflectClassInstance(T& instance);

    // Info about the type
    auto name() const -> std::string;
    auto attributes() const -> AttributeMap;

    // List of members
    auto variables() -> std::vector<ReflectMemberVariableInstance<T>>;
    // auto variables() const; // TODO
    auto functions() -> std::vector<ReflectMemberFunctionInstance<T>>;
    // auto functions() const; // TODO

    // Info of member by name
    auto variable_by_name(const std::string& name) -> ReflectMemberVariableInstance<T>;
    // auto variable(const std::string& name) const -> ReflectMemberVariableInstance<T>; TODO
    auto function_by_name(const std::string& name) -> ReflectMemberFunctionInstance<T>;
    // auto function(const std::string& name) const -> ReflectMemberFunctionInstance<T>; TODO

private:
    T& instance;
};

// ReflectMemberVariableInstance
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectMemberVariableInstance
{
public:
    // Constructor
    ReflectMemberVariableInstance(T& instance, int index);

    // Info about the type
    auto name() const -> std::string;
    auto attributes() const -> AttributeMap;

    // Get value
    template<class Var> auto value() -> Var&;
    template<class Var> auto value() const -> const Var&;

    // Pass value to callback
    template<class Fn> void apply(Fn function);
    template<class Fn> void apply(Fn function) const;

private:
    T& instance;
    int index;
};

// ReflectMemberFunctionInstance
// ----------------------------------------------------------------------------------------------
template<class T>
class ReflectMemberFunctionInstance
{
public:
    // Constructor
    ReflectMemberFunctionInstance(T& instance, int index);

    // Info about the type
    auto name() const -> std::string;
    auto attributes() const -> AttributeMap;

    template<class Ret = void, typename... Args>
    auto call(Args&&... args) -> Ret;

private:
    T& instance;
    int index;
};

#include "reflection.inl"