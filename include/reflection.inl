// Reflect Interface
// ----------------------------------------------------------------------------------------------------------------------------------------
template<class T> auto reflect() -> ReflectType<T>                  { return ReflectType<T>{}; }
template<class T> auto reflect(T instance) -> ReflectInstance<T>    { return ReflectInstance<T>(instance); }
//template<class T> auto reflect(T& instance) -> ReflectInstance<T>   { return ReflectInstance<T>(instance); }
//template<class T> auto reflect(const T& instance) -> ConstReflectInstance<T> { return ConstReflectInstance<T>(instance); }


// ReflectMemberVariableType
// ----------------------------------------------------------------------------------------------
template<class T> ReflectMemberVariableType<T>::ReflectMemberVariableType(int index) : index{ index } {}
template<class T> auto ReflectMemberVariableType<T>::name() const -> std::string                      { return reflect_variable_name<T, index>(); }
template<class T> auto ReflectMemberVariableType<T>::attributes() const -> AttributeMap               { return reflect_variable_attributes<T, index>(); }

// ReflectMemberFunctionType
// ----------------------------------------------------------------------------------------------
template<class T> ReflectMemberFunctionType<T>::ReflectMemberFunctionType(int index) : index{ index } {}
template<class T> auto ReflectMemberFunctionType<T>::name() const -> std::string                      { return reflect_name<T, index>(); }
template<class T> auto ReflectMemberFunctionType<T>::attributes() const -> AttributeMap               { return reflect_attributes<T, index>(); }

// ReflectType
// ----------------------------------------------------------------------------------------------------------------------------------------
template<class T> auto ReflectType<T>::name() const -> std::string                                          { return reflect_name<T>(); }
template<class T> auto ReflectType<T>::attributes() const -> AttributeMap                                   { return reflect_attributes<T>(); }
template<class T> auto ReflectType<T>::variables() const                                                    { return reflect_variables<T>(); }
template<class T> auto ReflectType<T>::functions() const                                                    { return reflect_functions<T>(); }
template<class T> auto ReflectType<T>::variable(const std::string& name) -> const ReflectMemberVariable<T>& { return reflect_variable<T>(name); };
template<class T> auto ReflectType<T>::function(const std::string& name) -> const ReflectMemberFunction<T>& { return reflect_function<T>(name); };


// ReflectInstance
// ----------------------------------------------------------------------------------------------------------------------------------------
template<class T> ReflectInstance<T>::ReflectInstance(T instance) : instance{ instance }                                     {}
template<class T> auto ReflectInstance<T>::name() const -> std::string                                                       { return reflect_name<T>(); }
template<class T> auto ReflectInstance<T>::attributes() const -> AttributeMap                                                { return reflect_attributes<T>(); }
template<class T> auto ReflectInstance<T>::variables()                                                                       { return reflect_variables<T>(instance); }
template<class T> auto ReflectInstance<T>::variables() const                                                                 { return reflect_variables<T>(instance); }
template<class T> auto ReflectInstance<T>::functions()                                                                       { return reflect_functions<T>(instance); }
template<class T> auto ReflectInstance<T>::functions() const                                                                 { return reflect_functions<T>(instance); }
template<class T> auto ReflectInstance<T>::variable(const std::string& name) -> ReflectMemberVariable<T>&                    { return reflect_variable<T>(name, instance); }
template<class T> auto ReflectInstance<T>::variable(const std::string& name) const -> const ReflectMemberVariable<T>&        { return reflect_variable<T>(name, instance); }
template<class T> auto ReflectInstance<T>::function(const std::string& name)-> ReflectMemberFunction<T>&                     { return reflect_function<T>(name, instance); }
template<class T> auto ReflectInstance<T>::function(const std::string& name) const -> const ReflectMemberFunction<T>&        { return reflect_function<T>(name, instance); }
template<class T> template<class S> auto ReflectInstance<T>::variable(const std::string& name) -> const S&                   { return reflect_variable<T, S>(name, instance); }
template<class T> template<class... ArgTypes, class R> R ReflectInstance<T>::call(const std::string& name, ArgTypes... args) { return reflect_call<T, ArgTypes..., R>(name, args..., instance); }
template<class T> template<class Fn> void ReflectInstance<T>::apply_variable(const std::string& name, Fn func)               { return reflect_apply_variable<T, Fn>(name, func, instance); }
template<class T> template<class Fn> void ReflectInstance<T>::apply_function(const std::string& name, Fn func)               { return reflect_apply_function<T, Fn>(name, func, instance); }