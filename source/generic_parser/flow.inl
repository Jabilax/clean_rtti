#include <cassert>

template<class T>
FutureFlowNode<T>::FutureFlowNode()
    : ptr{ new FlowNodePtr<T>{ nullptr } }
{
}

template<class T>
void FutureFlowNode<T>::set(FlowNodePtr<T> node)
{
    *ptr = node;
}

template<class T>
FutureFlowNodeRef<T>::FutureFlowNodeRef(FutureFlowNode<T> node)
    : ptr{node.ptr}
{
}

template<class T>
auto FutureFlowNodeRef<T>::operator->() -> FlowNodePtr<T>
{
    return *ptr;
}

template<class T>
auto FutureFlowNodeRef<T>::get() -> FlowNodePtr<T>
{
    return *ptr;
}

template<class T>
Flow<T>::Flow(FutureFlowNode<T> start)
    : start_node{ start }
    , next_node{ nullptr }
{
}

template<class T>
void Flow<T>::start(T& data)
{
    return start_node->execute(next_node, data);
}

template<class T>
void Flow<T>::next(T& data)
{
    assert(next_node != nullptr);
    return next_node->execute(next_node, data);
}

template<class T>
auto Flow<T>::is_finished() -> bool
{
    return next_node == nullptr;
}

template<class T>
ExecuteNode<T>::ExecuteNode(BoolNodeFn<T> expect_callback, VoidNodeFn<T> execute_callback, FutureFlowNode<T> next_node)
    : expect_callback{ expect_callback }, execute_callback{ execute_callback }
    , next_node{ next_node }
{
}

template<class T>
void ExecuteNode<T>::execute(FlowNodePtr<T>& next, T& data)
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
CompareNode<T>::CompareNode(BoolNodeFn<T> compare_callback, FutureFlowNode<T> true_node, FutureFlowNode<T> false_node)
    : compare_callback{ compare_callback }
    , true_node{ true_node }
    , false_node{ false_node }
{}

template<class T>
void CompareNode<T>::execute(FlowNodePtr<T>& next, T& data)
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
CompareExecuteNode<T>::CompareExecuteNode(BoolNodeFn<T> compare_callback, VoidNodeFn<T> execute_callback, FutureFlowNode<T> true_node, FutureFlowNode<T> false_node)
    : compare_callback{ compare_callback }
    , execute_callback{ execute_callback }
    , true_node{ true_node }
    , false_node{ false_node }
{}

template<class T>
void CompareExecuteNode<T>::execute(FlowNodePtr<T>& next, T& data)
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
CompareExecuteWaitNode<T>::CompareExecuteWaitNode(BoolNodeFn<T> compare_callback, VoidNodeFn<T> execute_callback, FutureFlowNode<T> true_node, FutureFlowNode<T> false_node)
    : compare_callback{ compare_callback }
    , execute_callback{ execute_callback }
    , true_node{ true_node }
    , false_node{ false_node }
{}

template<class T>
void CompareExecuteWaitNode<T>::execute(FlowNodePtr<T>& next, T& data)
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
SwitchWaitNode<T>::SwitchWaitNode(BoolNodeFn<T> first_compare, BoolNodeFn<T> second_compare, FutureFlowNode<T> first_node, FutureFlowNode<T> second_node, FutureFlowNode<T> none_node)
    : first_compare{ first_compare }
    , second_compare{ second_compare }
    , first_node{ first_node }
    , second_node{ second_node }
    , none_node{ none_node }
{}

template<class T>
void SwitchWaitNode<T>::execute(FlowNodePtr<T>& next, T& data)
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