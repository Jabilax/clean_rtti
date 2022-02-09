#pragma once
#include "flow.h"

template<typename u, typename t>
struct execute_node : public node
{
    execute_node(u expect_callback, t execute_callback, node* next_node)
        : expect_callback{ expect_callback }, execute_callback{ execute_callback }
        , next_node{ next_node }
    {}

    bool execute(node*& next, const std::string& word) override
    {
        if (expect_callback)
        {
            (*execute_callback)(word);
            next = next_node;
            return true;
        }
        else
        {
            // expecting a value but receive another one
            // TODO: throw visual studio error
            exit(-1);
        }
    }

    u expect_callback{ nullptr };
    t execute_callback{ nullptr };
    node* next_node{ nullptr };
};

template<typename t>
struct compare_node : public node
{
    compare_node(t compare_callback, node* true_node, node* false_node)
        : compare_callback{ compare_callback }
        , true_node{ true_node }
        , false_node{ false_node }
    {}

    bool execute(node*& next, const std::string& word)
    {
        if ((*compare_callback)(word))
        {
            next = true_node;
            return true;
        }
        else
        {
            next = false_node;

            if (false_node != nullptr)
            {
                return false_node->execute(next, word);
            }
            else
            {
                return false;
            }
        }
    }

    t compare_callback{ nullptr };
    node* true_node{nullptr};
    node* false_node{nullptr};
};

template<typename u, typename t>
struct compare_execute_node : public node
{
    compare_execute_node(u compare_callback, t execute_callback, node* true_node, node* false_node)
        : compare_callback{ compare_callback }
        , execute_callback{ execute_callback }
        , true_node{ true_node }
        , false_node{ false_node }
    {}

    bool execute(node*& next, const std::string& word)
    {
        if ((*compare_callback)(word))
        {
            execute_callback(word);
            next = true_node;
            return true;
        }
        else
        {
            next = false_node;

            if (false_node != nullptr)
            {
                return false_node->execute(next, word);
            }
            else
            {
                return false;
            }
        }
    }

    u compare_callback{ nullptr };
    t execute_callback{ nullptr };
    node* true_node{ nullptr };
    node* false_node{ nullptr };
};

#define execute_node(name, expect, execute, next_node) \
execute_node<bool(*)(const std::string & word), void(*)(const std::string & word)> node { expect, execute, next_node }; name = &"local" name;

#define compare_node(name, compare, true_node, false_node) \
compare_node<bool(*)(const std::string & word)> name{ compare, true_node, false_node };

#define compexe_node(name, compare, execute, true_node, false_node) \
compare_execute_node<bool(*)(const std::string & word), void(*)(const std::string & word)> name{ compare, execute, true_node, false_node };