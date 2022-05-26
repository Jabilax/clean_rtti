// ------------------------------------------------------------------------------------------------
// Reflect Interface
// ------------------------------------------------------------------------------------------------

template<class T>
auto reflect() -> ReflectClass<T>
{
    return ReflectClass<T>{};
}

template<class T>
auto reflect(T& instance) -> ReflectClassInstance<T>
{
    return ReflectClassInstance<T>(instance);
}

// template<class T> auto reflect(T& instance) -> ReflectInstance<T>
// {
//     TODO
// }
// 
// template<class T> auto reflect(const T& instance) -> ConstReflectInstance<T>
// {
//     TODO
// }


// ------------------------------------------------------------------------------------------------
// ReflectClass
// ------------------------------------------------------------------------------------------------

template<class T>
auto ReflectClass<T>::name() -> std::string
{
    return Reflect<T>::name();
}

template<class T> 
auto ReflectClass<T>::attributes() -> AttributeMap 
{
    return TODO<T>(); // TODO
}

template<class T>
auto ReflectClass<T>::variables() -> const std::vector<ReflectMemberVariable<T>>&
{
    static std::vector<ReflectMemberVariableType<T>> variables;

    if (variables.empty())
    {
        const auto size = Reflect<T>::variable_num();
        variables.reserve(size);

        for (auto i = 0; i < size; ++i)
        {
            variables.emplace_back(i);
        }
    }

    return variables;
}

template<class T>
auto ReflectClass<T>::functions() -> const std::vector<ReflectMemberFunction<T>>&
{
    static std::vector<ReflectMemberFunction<T>> functions;

    if (functions.empty())
    {
        const auto size = Reflect<T>::function_num();
        functions.reserve(size);

        for (auto i = 0; i < size; ++i)
        {
            functions.emplace_back(i);
        }
    }

    return functions;
}

template<class T>
auto ReflectClass<T>::variable_by_name(const std::string& name) -> ReflectMemberVariable<T>
{
    static std::unordered_map<std::string, int> variable_map;

    if (variable_map.empty())
    {
        for (auto i = 0; i < Reflect<T>::variable_num(); ++i)
        {
            variable_map[Reflect<T>::variable_name(i)] = i;
        }
    }

    return ReflectMemberVariable<T>(variable_map[name]);
};

template<class T>
auto ReflectClass<T>::function_by_name(const std::string& name) -> ReflectMemberFunction<T>
{
    static std::unordered_map<std::string, int> function_map;

    if (function_map.empty())
    {
        for (auto i = 0; i < Reflect<T>::function_num(); ++i)
        {
            function_map[Reflect<T>::function_name(i)] = i;
        }
    }

    return ReflectMemberFunction<T>(function_map[name]);
};


// ------------------------------------------------------------------------------------------------
// ReflectMemberVariable
// ------------------------------------------------------------------------------------------------

template<class T>
ReflectMemberVariable<T>::ReflectMemberVariable(int index)
    : index{ index }
{
}

template<class T>
auto ReflectMemberVariable<T>::name() const -> std::string
{
    return Reflect<T>::variable_name(index);
}

// template<class T>
// auto ReflectMemberVariableType<T>::attributes() const -> AttributeMap
// {
//     TODO
// }

template<class T> template<class Var>
auto ReflectMemberVariable<T>::value(T& instance) const -> Var&
{
    return Reflect<T>::template variable<Var>(instance, index);
}

template<class T> template<class Var>
auto ReflectMemberVariable<T>::value(const T& instance) const -> const Var&
{
    return Reflect<T>::template variable<Var>(instance, index);
}

template<class T> template<class Fn>
void ReflectMemberVariable<T>::apply(T& instance, Fn function) const
{
    return Reflect<T>::template variable_apply<Fn>(instance, index, function);
}

template<class T> template<class Fn>
void ReflectMemberVariable<T>::apply(const T& instance, Fn function) const
{
    return Reflect<T>::template variable_apply<Fn>(instance, index, function);
}


// ------------------------------------------------------------------------------------------------
// ReflectMemberFunction
// ------------------------------------------------------------------------------------------------

template<class T>
ReflectMemberFunction<T>::ReflectMemberFunction(int index)
    : index{ index }
{
}

template<class T>
auto ReflectMemberFunction<T>::name() const -> std::string
{
    return Reflect<T>::function_name(index);
}

// template<class T>
// auto ReflectMemberFunction<T>::attributes() const -> AttributeMap
// {
//     TODO
// }


// template<class T>
// auto ReflectMemberFunction<T>::call()
// {
//     TODO
// }


// ------------------------------------------------------------------------------------------------
// ReflectClassInstance
// ------------------------------------------------------------------------------------------------

template<class T>
ReflectClassInstance<T>::ReflectClassInstance(T& instance)
    : instance{ instance }
{
}

template<class T>
auto ReflectClassInstance<T>::name() const -> std::string
{
    return ReflectClass<T>::name();
}

template<class T>
auto ReflectClassInstance<T>::attributes() const -> AttributeMap
{
    return ReflectClass<T>::attributes();
}

template<class T>
auto ReflectClassInstance<T>::variables() -> std::vector<ReflectMemberVariableInstance<T>>
{
    const auto size = Reflect<T>::variable_num();
    std::vector<ReflectMemberVariableInstance<T>> variables;
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
//    return TODO
//}

template<class T>
auto ReflectClassInstance<T>::functions() -> std::vector<ReflectMemberFunctionInstance<T>>
{
    const auto size = Reflect<T>::function_num();
    std::vector<ReflectMemberFunctionInstance<T>> functions;
    functions.reserve(size);

    for (auto i = 0; i < size; ++i)
    {
        functions.emplace_back(instance, i);
    }

    return functions;
}

// template<class T>
// auto ReflectInstance<T>::functions() const
// {
//     return TODO
// }

template<class T>
auto ReflectClassInstance<T>::variable_by_name(const std::string& name) -> ReflectMemberVariableInstance<T>
{
    return ReflectMemberVariableInstance<T>(instance, ReflectClass<T>::variable_by_name(name).index);
}

// template<class T>
// auto ReflectInstance<T>::variable_by_name(const std::string& name) const -> const ReflectMemberVariable<T>&
// {
//     return TODO
// }

template<class T>
auto ReflectClassInstance<T>::function_by_name(const std::string& name) -> ReflectMemberFunctionInstance<T>
{
    return ReflectMemberFunctionInstance<T>(instance, ReflectClass<T>::function_by_name(name).index);
}


// template<class T>
// auto ReflectInstance<T>::function_by_name(const std::string& name) const -> const ReflectMemberFunction<T>&
// {
//     return TODO
// }


// ------------------------------------------------------------------------------------------------
// ReflectMemberVariableInstance
// ------------------------------------------------------------------------------------------------

template<class T>
ReflectMemberVariableInstance<T>::ReflectMemberVariableInstance(T& instance, int index)
    : instance{instance}
    , index{index}
{
}

template<class T>
auto ReflectMemberVariableInstance<T>::name() const -> std::string
{
    return ReflectMemberVariable<T>(index).name();
}

template<class T>
auto ReflectMemberVariableInstance<T>::attributes() const -> AttributeMap
{
    return ReflectMemberVariable<T>(index).attributes();
}

template<class T> template<class Var>
auto ReflectMemberVariableInstance<T>::value() -> Var&
{
    return ReflectMemberVariable<T>(index).value<Var>(instance);
}

template<class T> template<class Var>
auto ReflectMemberVariableInstance<T>::value() const -> const Var&
{
    return ReflectMemberVariable<T>(index).value<Var>(instance);
}

template<class T> template<class Fn>
void ReflectMemberVariableInstance<T>::apply(Fn function)
{
    return ReflectMemberVariable<T>(index).apply<Fn>(instance, function);
}

template<class T> template<class Fn>
void ReflectMemberVariableInstance<T>::apply(Fn function) const
{
    return ReflectMemberVariable<T>(index).apply<Fn>((const T&)instance, function);
}


// ------------------------------------------------------------------------------------------------
// ReflectMemberFunctionInstance
// ------------------------------------------------------------------------------------------------
template<class T>
ReflectMemberFunctionInstance<T>::ReflectMemberFunctionInstance(T& instance, int index)
    : instance{ instance }
    , index{ index }
{
}

template<class T>
auto ReflectMemberFunctionInstance<T>::name() const -> std::string
{
    return ReflectMemberFunction<T>(index).name();
}

template<class T>
auto ReflectMemberFunctionInstance<T>::attributes() const -> AttributeMap
{
    return ReflectMemberFunction<T>(index).attributes();
}

// template<class T>
// auto ReflectMemberFunctionInstance<T>::call()
// {
//     TODO
// }
// auto call(); TODO