#include <cassert>

template<class T>
FutureNode<T>::FutureNode()
    : ptr{ new NodePtr<T>{ nullptr } }
{
}

template<class T>
void FutureNode<T>::set(NodePtr<T> node)
{
    *ptr = node;
}

template<class T>
FutureNodeRef<T>::FutureNodeRef(FutureNode<T> node)
    : ptr{node.ptr}
{
}

template<class T>
auto FutureNodeRef<T>::operator->() -> NodePtr<T>
{
    return *ptr;
}

template<class T>
auto FutureNodeRef<T>::get() -> NodePtr<T>
{
    return *ptr;
}

template<class T>
NodeGraph<T>::NodeGraph(FutureNode<T> start)
    : start_node{ start }
    , next_node{ nullptr }
{
}

template<class T>
void NodeGraph<T>::start(T& data)
{
    return start_node->execute(next_node, data);
}

template<class T>
void NodeGraph<T>::next(T& data)
{
    assert(next_node != nullptr);
    return next_node->execute(next_node, data);
}

template<class T>
auto NodeGraph<T>::is_finished() -> bool
{
    return next_node == nullptr;
}

template<class T>
ExecuteNode<T>::ExecuteNode(BoolNodeFn<T> expect_callback, VoidNodeFn<T> execute_callback, FutureNode<T> next_node, const std::string& name)
    : expect_callback{ expect_callback }
    , execute_callback{ execute_callback }
    , next_node{ next_node }
    , name{ name }
{
}

template<class T>
void ExecuteNode<T>::execute(NodePtr<T>& next, T& data)
{
    if (expect_callback)
    {
        execute_callback(data);
        next = next_node.get();
    }
    else
    {
        // expecting a value but receive another one
        // TODO: throw visual studio error
        exit(-1);
    }
}

template<class T>
CompareNode<T>::CompareNode(BoolNodeFn<T> compare_callback, FutureNode<T> true_node, FutureNode<T> false_node, const std::string& name)
    : compare_callback{ compare_callback }
    , true_node{ true_node }
    , false_node{ false_node }
    , name{ name }
{}

template<class T>
void CompareNode<T>::execute(NodePtr<T>& next, T& data)
{
    if (compare_callback(data))
    {
        next = true_node.get();
    }
    else
    {
        next = false_node.get();

        if (false_node.get() != nullptr)
        {
            false_node->execute(next, data);
        }
    }
}

template<class T>
CompareExecuteNode<T>::CompareExecuteNode(BoolNodeFn<T> compare_callback, VoidNodeFn<T> execute_callback, FutureNode<T> true_node, FutureNode<T> false_node, const std::string& name)
    : compare_callback{ compare_callback }
    , execute_callback{ execute_callback }
    , true_node{ true_node }
    , false_node{ false_node }
    , name{ name }
{}

template<class T>
void CompareExecuteNode<T>::execute(NodePtr<T>& next, T& data)
{
    if (compare_callback(data))
    {
        execute_callback(data);
        next = true_node.get();
    }
    else
    {
        next = false_node.get();

        if (false_node.get() != nullptr)
        {
            false_node->execute(next, data);
        }
    }
}

template<class T>
CompareExecuteWaitNode<T>::CompareExecuteWaitNode(BoolNodeFn<T> compare_callback, VoidNodeFn<T> execute_callback, FutureNode<T> true_node, FutureNode<T> false_node, const std::string& name)
    : compare_callback{ compare_callback }
    , execute_callback{ execute_callback }
    , true_node{ true_node }
    , false_node{ false_node }
    , name{ name }
{}

template<class T>
void CompareExecuteWaitNode<T>::execute(NodePtr<T>& next, T& data)
{
    if (compare_callback(data))
    {
        execute_callback(data);
        next = true_node.get();
    }
    else
    {
        next = false_node.get();
    }
}

template<class T>
SwitchWaitNode<T>::SwitchWaitNode(BoolNodeFn<T> first_compare, BoolNodeFn<T> second_compare, FutureNode<T> first_node, FutureNode<T> second_node, FutureNode<T> none_node, const std::string& name)
    : first_compare{ first_compare }
    , second_compare{ second_compare }
    , first_node{ first_node }
    , second_node{ second_node }
    , none_node{ none_node }
    , name{ name }
{}

template<class T>
void SwitchWaitNode<T>::execute(NodePtr<T>& next, T& data)
{
    if (first_compare(data))
    {
        next = first_node.get();
    }
    else if (second_compare(data))
    {
        next = second_node.get();
    }
    else
    {
        next = none_node.get();
    }
}