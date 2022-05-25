// Reflect Interface
// ----------------------------------------------------------------------------------------------------------------------------------------
template<class T>
auto reflect() -> ReflectType<T>
{
    return ReflectType<T>{};
}

template<class T>
auto reflect(T& instance) -> ReflectInstance<T>
{
    return ReflectInstance<T>(instance);
}

//template<class T> auto reflect(T& instance) -> ReflectInstance<T>   { return ReflectInstance<T>(instance); }
//template<class T> auto reflect(const T& instance) -> ConstReflectInstance<T> { return ConstReflectInstance<T>(instance); }


// ReflectMemberVariableType
// ----------------------------------------------------------------------------------------------
template<class T>
ReflectMemberVariableType<T>::ReflectMemberVariableType(int index)
    : index{ index }
{
}

template<class T>
auto ReflectMemberVariableType<T>::name() const -> std::string
{
    return Reflect<T>::variable_name(index);
}

// template<class T>
// auto ReflectMemberVariableType<T>::attributes() const -> AttributeMap
// {
//     
// }

template<class T> template<class Var>
auto ReflectMemberVariableType<T>::value(T& instance) -> Var&
{
    return Reflect<T>::template variable<Var>(instance, index);
}

template<class T> template<class Var>
auto ReflectMemberVariableType<T>::value(const T& instance) const -> const Var&
{
    return Reflect<T>::template variable<Var>(instance, index);
}

template<class T> template<class Fn>
void ReflectMemberVariableType<T>::apply(T& instance, Fn function)
{
    return Reflect<T>::template variable_apply<Fn>(instance, index, function);
}

template<class T> template<class Fn>
void ReflectMemberVariableType<T>::apply(const T& instance, Fn function) const
{
    return Reflect<T>::template variable_apply<Fn>(instance, index, function);
}

// ReflectMemberVariable
// ----------------------------------------------------------------------------------------------
template<class T>
ReflectMemberVariable<T>::ReflectMemberVariable(T& instance, int index)
    : instance{instance}
    , index{index}
{
}

template<class T>
auto ReflectMemberVariable<T>::name() const -> std::string
{
    return Reflect<T>::variable_name(index);
}

// template<class T>
// auto ReflectMemberVariable<T>::attributes() const -> AttributeMap
// {
// }

template<class T> template<class Var>
auto ReflectMemberVariable<T>::value() -> Var&
{
    return Reflect<T>::template variable<Var>(instance, index);
}

template<class T> template<class Var>
auto ReflectMemberVariable<T>::value() const -> const Var&
{
    return Reflect<T>::template variable<Var>(instance, index);
}

template<class T> template<class Fn>
void ReflectMemberVariable<T>::apply(Fn function)
{
    return Reflect<T>::template variable_apply<Fn>(instance, index, function);
}

template<class T> template<class Fn>
void ReflectMemberVariable<T>::apply(Fn function) const
{
    return Reflect<T>::template variable_apply<Fn>((const T&)instance, index, function);
}


// ReflectMemberFunctionType
// ----------------------------------------------------------------------------------------------
template<class T>
ReflectMemberFunctionType<T>::ReflectMemberFunctionType(int index)
    : index{ index }
{
}

template<class T>
auto ReflectMemberFunctionType<T>::name() const -> std::string
{
    return Reflect<T>::function_name(index);
}

// template<class T>
// auto ReflectMemberFunctionType<T>::attributes() const -> AttributeMap
// {
//     return reflect_function_attributes<T, index>();
// }

// ReflectType
// ----------------------------------------------------------------------------------------------------------------------------------------
template<class T>
auto ReflectType<T>::name() const -> std::string
{
    return Reflect<T>::name();
}

template<class T> auto ReflectType<T>::attributes() const -> AttributeMap                                   { return reflect_attributes<T>(); }

template<class T>
auto ReflectType<T>::variables() const -> std::vector<ReflectMemberVariableType<T>>
{
    const auto size = Reflect<T>::variable_num();
    std::vector<ReflectMemberVariableType<T>> variables;
    variables.reserve(size);

    for (auto i = 0; i < size; ++i)
    {
        variables.emplace_back(i);
    }

    return variables;
}

template<class T>
auto ReflectType<T>::functions() const -> std::vector<ReflectMemberFunctionType<T>>
{
    const auto size = Reflect<T>::function_num();
    std::vector<ReflectMemberFunctionType<T>> functions;
    functions.reserve(size);

    for (auto i = 0; i < size; ++i)
    {
        functions.emplace_back(i);
    }

    return functions;
}

template<class T>
auto ReflectType<T>::variable(const std::string& name) -> const ReflectMemberVariable<T>& { return reflect_variable<T>(name); };

template<class T>
auto ReflectType<T>::function(const std::string& name) -> const ReflectMemberFunction<T>& { return reflect_function<T>(name); };


// ReflectInstance
// ----------------------------------------------------------------------------------------------------------------------------------------
template<class T>
ReflectInstance<T>::ReflectInstance(T& instance)
    : instance{ instance }
{
}

template<class T>
auto ReflectInstance<T>::name() const -> std::string
{
    return Reflect<T>::name();
}

template<class T>
auto ReflectInstance<T>::attributes() const -> AttributeMap                                                { return reflect_attributes<T>(); }

template<class T>
auto ReflectInstance<T>::variables() -> std::vector<ReflectMemberVariable<T>>
{
    const auto size = Reflect<T>::variable_num();
    std::vector<ReflectMemberVariable<T>> variables;
    variables.reserve(size);

    for (auto i = 0; i < size; ++i)
    {
        variables.emplace_back(instance, i);
    }

    return variables;
}

//template<class T> 
//auto ReflectInstance<T>::variables() const 
//{
//    return todo<T>();
//}

template<class T>
auto ReflectInstance<T>::functions() -> std::vector<ReflectMemberFunction<T>>
{
    const auto size = Reflect<T>::function_num();
    std::vector<ReflectMemberFunction<T>> functions;
    functions.reserve(size);

    for (auto i = 0; i < size; ++i)
    {
        functions.emplace_back(i);
    }

    return functions;
}

// template<class T> 
// auto ReflectInstance<T>::functions() const
// {
//     return reflect_functions<T>(instance);
// }



template<class T> auto ReflectInstance<T>::variable(const std::string& name) -> ReflectMemberVariable<T>&
{
    return reflect_variable<T>(name, instance);
}
// template<class T> auto ReflectInstance<T>::variable(const std::string& name) const -> const ReflectMemberVariable<T>&        { return reflect_variable<T>(name, instance); }
template<class T> auto ReflectInstance<T>::function(const std::string& name)-> ReflectMemberFunction<T>&                     { return reflect_function<T>(name, instance); }
// template<class T> auto ReflectInstance<T>::function(const std::string& name) const -> const ReflectMemberFunction<T>&        { return reflect_function<T>(name, instance); }