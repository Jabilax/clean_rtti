#pragma once
#include <string>
#include <functional>
#include <memory>

struct node
{
    virtual bool execute(node* next, const std::string& word) = 0;
};
using node_ptr = std::shared_ptr<node>;

struct flow
{
    flow(node_ptr& start);

    // returns if the flow is exectuting
    bool execute(const std::string& word);

    node_ptr start;
    node* next{nullptr};
};

//struct token_flow
//{
//    // returns if the flow is exectuting
//    bool execute(char token);
//
//    std::string buffer;
//    bool send;
//
//    node_ptr start{ nullptr };
//    node_ptr next{ nullptr };
//};


