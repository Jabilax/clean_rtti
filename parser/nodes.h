#pragma once
#include "flow.h"
#include <functional>

using void_word_fn = std::function<void(const std::string& word)>;
using bool_word_fn = std::function<bool(const std::string& word)>;

struct ExecuteNode : public node
{
    ExecuteNode(bool_word_fn expect_callback, void_word_fn execute_callback, node_ptr& next_node)
        : expect_callback{ expect_callback }, execute_callback{ execute_callback }
        , next_node{ next_node }
    {}

    bool execute(node* next, const std::string& word) override
    {
        if (expect_callback)
        {
            execute_callback(word);
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

    bool_word_fn expect_callback;
    void_word_fn execute_callback;
    node_ptr next_node;
};

struct CompareNode : public node
{
    CompareNode(bool_word_fn compare_callback, node_ptr& true_node, node_ptr& false_node)
        : compare_callback{ compare_callback }
        , true_node{ std::make_shared<node>(true_node) }
        , false_node{ std::make_shared<node>(false_node) }
    {}

    bool execute(node* next, const std::string& word)
    {
        if (compare_callback(word))
        {
            next = true_node.get();
            return true;
        }
        else
        {
            next = false_node.get();

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

    bool_word_fn compare_callback;
    node_ptr true_node;
    node_ptr false_node;
};

struct CompareExecute : public node
{
    CompareExecute(bool_word_fn compare_callback, void_word_fn execute_callback, node_ptr& true_node, node_ptr& false_node)
        : compare_callback{ compare_callback }
        , execute_callback{ execute_callback }
        , true_node{ true_node }
        , false_node{ false_node }
    {}

    bool execute(node* next, const std::string& word)
    {
        if (compare_callback(word))
        {
            execute_callback(word);
            next = true_node.get();
            return true;
        }
        else
        {
            next = false_node.get();

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

    bool_word_fn compare_callback;
    void_word_fn execute_callback;
    node_ptr true_node;
    node_ptr false_node;
};

auto execute_node = [](node_ptr& var, bool_word_fn expect, void_word_fn execute, node_ptr& next_node)
{
    var = std::make_shared<ExecuteNode>(expect, execute, next_node);
};

auto compare_node = [](node_ptr& var, bool_word_fn compare, node_ptr& true_node, node_ptr& false_node)
{
    var = std::make_shared<CompareNode>(compare, true_node, false_node);
};

auto compexe_node = [](node_ptr& var, bool_word_fn compare, void_word_fn execute, node_ptr& true_node, node_ptr& false_node)
{
    var = std::make_shared<CompareExecute>(compare, execute, true_node, false_node);
};