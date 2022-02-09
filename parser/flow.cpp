#include "flow.h"
#include <cassert>
#include "parser.h"
#include <iostream>

bool flow::execute(const std::string& word)
{
    if (next == nullptr)
        return start->execute(next, word);
    else
        return next->execute(next, word);
}

bool token_flow::execute(char token)
{
    buffer.push_back(token);

    if (next == nullptr)
        return start->execute(next, std::string(1, token));
    else
        return next->execute(next, std::string(1, token));
}