
template<class T>
void Parser<T>::add_flow(FutureFlowNode<T> start_node)
{
    flows.emplace_back(start_node);
}

template<class T>
void Parser<T>::push(T&& data)
{
    // Continue execution on previous active flows.
    for (auto it = active_flows.begin(); it != active_flows.end();)
    {
        if (auto& flow = *it; flow.next(data))
        {
            // If the flow has ended remove it.
            it = active_flows.erase(it);
            continue;
        }
        it++;
    }

    // Check if any new flow starts.
    for (auto& flow : flows)
    {
        if (flow.start(data))
        {
            // Move the flow to the active ones.
            active_flows.push_back(flow);
        }
    }
}
