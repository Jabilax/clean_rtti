#pragma once
#include "flow.h"
#include <vector>

// The parser can act on any data.
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