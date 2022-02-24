#pragma once
#include "flow.h"
#include <vector>

// The parser can act on any data.
// A flow is a graph of nodes that read the data.
// You can have multiple flows at the same time.
// You can also have mutiple parsers working at the same time on the data,
// maybe one at the token level and another one at the word level.
template<class T>
struct Parser
{
public:
    void add_flow(FutureFlowNode<T> start);
    void push(T&& data);

private:
    std::vector<Flow<T>> flows;
    std::vector<Flow<T>> active_flows;
};

#include "generic_parser/parser.inl"