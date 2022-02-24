#pragma once
#include <functional>

// Forward declarations.
template<class T> struct FlowNode;
template<class T> struct FutureFlowNode;
template<class T> struct FutureFlowNodeRef;
template<class T> using FlowNodePtr = FlowNode<T>*;
template<class T> using VoidNodeFn = std::function<void(T& data)>;
template<class T> using BoolNodeFn = std::function<bool(T& data)>;

// The Flow class is a node graph that we use to parse specific parts of the text.
template<class T>
struct Flow
{
    Flow(FutureFlowNode<T> start);

    // Returns if the flow is exectuting.
    auto start(T& data) -> bool;
    auto next(T& data) -> bool;

private:
    FutureFlowNodeRef<T> start_node;
    FlowNodePtr<T> next_node;
};

// Base class of a node, to use in a flow.
template<class T>
struct FlowNode
{
    virtual auto execute(FlowNodePtr<T>& next, T& data) -> bool = 0;
};

// This is a node that may be initialized in the future.
template<class T>
struct FutureFlowNode
{
    FutureFlowNode();
    void set(FlowNodePtr<T> node);

private:
    friend struct FutureFlowNodeRef<T>;
    FlowNodePtr<T>* ptr;
};

template<class T>
struct FutureFlowNodeRef
{
    FutureFlowNodeRef(FutureFlowNode<T> node);
    auto operator->() -> FlowNodePtr<T>;
    auto get() -> FlowNodePtr<T>;

    FlowNodePtr<T>* ptr;
};

// This node will check the expected is correct and call the callback.
template<class T>
struct ExecuteNode : public FlowNode<T>
{
    ExecuteNode(BoolNodeFn<T> expect_callback, VoidNodeFn<T> execute_callback, FutureFlowNode<T> next_node);
    auto execute(FlowNodePtr<T>& next, T& data) -> bool final;

    BoolNodeFn<T> expect_callback;
    VoidNodeFn<T> execute_callback;
    FutureFlowNodeRef<T> next_node;
};  

// This node will compare the data and go towards the true or false node.
template<class T>
struct CompareNode : public FlowNode<T>
{
    CompareNode(BoolNodeFn<T> compare_callback, FutureFlowNode<T> true_node, FutureFlowNode<T> false_node);
    auto execute(FlowNodePtr<T>& next, T& data) -> bool final;

    BoolNodeFn<T> compare_callback;
    FutureFlowNodeRef<T> true_node;
    FutureFlowNodeRef<T> false_node;
};

// This node will compare the word, and if true, will execute the callback and go to the true node, otherwise goes to the false node without executing.
template<class T>
struct CompareExecuteNode : public FlowNode<T>
{
    CompareExecuteNode(BoolNodeFn<T> compare_callback, VoidNodeFn<T> execute_callback, FutureFlowNode<T> true_node, FutureFlowNode<T> false_node);
    auto execute(FlowNodePtr<T>& next, T& data) -> bool final;

    BoolNodeFn<T> compare_callback;
    VoidNodeFn<T> execute_callback;
    FutureFlowNodeRef<T> true_node;
    FutureFlowNodeRef<T> false_node;
};

// Same as CompareExecuteNode but it consumes the data. Previously false node would get called with the same data as this.
template<class T>
struct CompareExecuteWaitNode : public FlowNode<T>
{
    CompareExecuteWaitNode(BoolNodeFn<T> compare_callback, VoidNodeFn<T> execute_callback, FutureFlowNode<T> true_node, FutureFlowNode<T> false_node);
    auto execute(FlowNodePtr<T>& next, T& data) -> bool final;

    BoolNodeFn<T> compare_callback;
    VoidNodeFn<T> execute_callback;
    FutureFlowNodeRef<T> true_node;
    FutureFlowNodeRef<T> false_node;
};

// Switch Node.
template<class T>
struct SwitchWaitNode : public FlowNode<T>
{
    SwitchWaitNode(BoolNodeFn<T> first_compare, BoolNodeFn<T> second_compare, FutureFlowNode<T> first_node, FutureFlowNode<T> second_node, FutureFlowNode<T> none_node);
    auto execute(FlowNodePtr<T>& next, T& data) -> bool final;

    BoolNodeFn<T> first_compare;
    BoolNodeFn<T> second_compare;
    FutureFlowNodeRef<T> first_node;
    FutureFlowNodeRef<T> second_node;
    FutureFlowNodeRef<T> none_node;
};


#include "generic_parser/flow.inl"