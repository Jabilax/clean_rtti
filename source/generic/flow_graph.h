#pragma once
#include "generic/node_graph.h"
#include <vector>

// A FlowGraph is a collection of node graphs that act on the same data.
// You keep pushing the data and the node graphs flow through their nodes.
template<class T>
struct FlowGraph
{
public:
    void add_node_graph(FutureNode<T> start);
    void push(T&& data);

private:
    std::vector<NodeGraph<T>> flows;
    std::vector<NodeGraph<T>> active_flows;
};

#include "generic/flow_graph.inl"