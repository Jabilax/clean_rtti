#include <cassert>

template<class T>
FutureFlowNode<T>::FutureFlowNode()
    : ptr{ new FlowNodePtr<T> }
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
auto Flow<T>::start(T& data) -> bool
{
    return start_node->execute(next_node, data);
}

template<class T>
auto Flow<T>::next(T& data) -> bool
{
    assert(next_node != nullptr);
    return next_node->execute(next_node, data);
}

template<class T>
ExecuteNode<T>::ExecuteNode(BoolNodeFn<T> expect_callback, VoidNodeFn<T> execute_callback, FutureFlowNode<T> next_node)
    : expect_callback{ expect_callback }, execute_callback{ execute_callback }
    , next_node{ next_node }
{
}

template<class T>
auto ExecuteNode<T>::execute(FlowNodePtr<T>& next, T& data) -> bool
{
    if (expect_callback)
    {
        execute_callback(data);
        next = next_node.get();
        return true;
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
auto CompareNode<T>::execute(FlowNodePtr<T>& next, T& data) -> bool
{
    if (compare_callback(data))
    {
        next = true_node.get();
        return true;
    }
    else
    {
        next = false_node.get();

        if (false_node.get() != nullptr)
        {
            return false_node->execute(next, data);
        }
        else
        {
            return false;
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
auto CompareExecuteNode<T>::execute(FlowNodePtr<T>& next, T& data) -> bool
{
    if (compare_callback(data))
    {
        execute_callback(data);
        next = true_node.get();
        return true;
    }
    else
    {
        next = false_node.get();

        if (false_node.get() != nullptr)
        {
            return false_node->execute(next, data);
        }
        else
        {
            return false;
        }
    }
}