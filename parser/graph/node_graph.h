#pragma once
#include <functional>

// Forward declarations.
template<class T> struct Node;
template<class T> struct FutureNode;
template<class T> struct FutureNodeRef;
template<class T> using NodePtr = Node<T>*;
template<class T> using VoidNodeFn = std::function<void(T& data)>;
template<class T> using BoolNodeFn = std::function<bool(T& data)>;

// Node graph.
template<class T>
struct NodeGraph
{
    NodeGraph(FutureNode<T> start);

    // Returns if the flow is exectuting.
    void start       (T& data);
    void next        (T& data);
    auto is_finished () -> bool;

private:
    FutureNodeRef<T> start_node;
    NodePtr<T> next_node;
};

// Base class of a node, to use in a node graph.
template<class T>
struct Node
{
    virtual void execute(NodePtr<T>& next, T& data) = 0;
};

// This is a node that may be initialized in the future.
template<class T>
struct FutureNode
{
    FutureNode();
    void set(NodePtr<T> node);

private:
    friend struct FutureNodeRef<T>;
    NodePtr<T>* ptr;
};

template<class T>
struct FutureNodeRef
{
    FutureNodeRef   (FutureNode<T> node);
    auto operator-> () -> NodePtr<T>;
    auto get        () -> NodePtr<T>;

    NodePtr<T>* ptr;
};

// This node will check the expected is correct and call the callback.
template<class T>
struct ExecuteNode : public Node<T>
{
    ExecuteNode(BoolNodeFn<T> expect_callback, VoidNodeFn<T> execute_callback, FutureNode<T> next_node, const std::string& name);
    void execute(NodePtr<T>& next, T& data) final;

    BoolNodeFn<T> expect_callback;
    VoidNodeFn<T> execute_callback;
    FutureNodeRef<T> next_node;
    std::string name;
};  

// This node will compare the data and go towards the true or false node.
template<class T>
struct CompareNode : public Node<T>
{
    CompareNode(BoolNodeFn<T> compare_callback, FutureNode<T> true_node, FutureNode<T> false_node, const std::string& name);
    void execute(NodePtr<T>& next, T& data) final;

    BoolNodeFn<T> compare_callback;
    FutureNodeRef<T> true_node;
    FutureNodeRef<T> false_node;
    std::string name;
};

// This node will compare the word, and if true, will execute the callback and go to the true node, otherwise goes to the false node without executing.
template<class T>
struct CompareExecuteNode : public Node<T>
{
    CompareExecuteNode(BoolNodeFn<T> compare_callback, VoidNodeFn<T> execute_callback, FutureNode<T> true_node, FutureNode<T> false_node, const std::string& name);
    void execute(NodePtr<T>& next, T& data) final;

    BoolNodeFn<T> compare_callback;
    VoidNodeFn<T> execute_callback;
    FutureNodeRef<T> true_node;
    FutureNodeRef<T> false_node;
    std::string name;
};

// Same as CompareExecuteNode but it consumes the data. Previously false node would get called with the same data as this.
template<class T>
struct CompareExecuteWaitNode : public Node<T>
{
    CompareExecuteWaitNode(BoolNodeFn<T> compare_callback, VoidNodeFn<T> execute_callback, FutureNode<T> true_node, FutureNode<T> false_node, const std::string& name);
    void execute(NodePtr<T>& next, T& data) final;

    BoolNodeFn<T> compare_callback;
    VoidNodeFn<T> execute_callback;
    FutureNodeRef<T> true_node;
    FutureNodeRef<T> false_node;
    std::string name;
};

// Switch Node.
template<class T>
struct SwitchWaitNode : public Node<T>
{
    SwitchWaitNode(BoolNodeFn<T> first_compare, BoolNodeFn<T> second_compare, FutureNode<T> first_node, FutureNode<T> second_node, FutureNode<T> none_node, const std::string& name);
    void execute(NodePtr<T>& next, T& data) final;

    BoolNodeFn<T> first_compare;
    BoolNodeFn<T> second_compare;
    FutureNodeRef<T> first_node;
    FutureNodeRef<T> second_node;
    FutureNodeRef<T> none_node;
    std::string name;
};


#include "parser/graph/node_graph.inl"