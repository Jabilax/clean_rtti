#pragma once
#include <string>
#include <functional>

struct node
{
    virtual bool execute(node*& next, const std::string& word) = 0;
};

struct flow
{
    // returns if the flow is exectuting
    bool execute(const std::string& word);

    node* start{nullptr};
    node* next{nullptr};
};

struct token_flow
{
    // returns if the flow is exectuting
    bool execute(char token);

    std::string buffer;
    bool send;

    node* start{ nullptr };
    node* next{ nullptr };
};

