
template<class T>
void FlowGraph<T>::add_node_graph(FutureNode<T> start_node)
{
    flows.emplace_back(start_node);
}

template<class T>
void FlowGraph<T>::push(T&& data)
{
    // Continue execution on previous active flows.
    for (auto it = active_flows.begin(); it != active_flows.end();)
    {
        auto& flow = *it;

        if (flow.is_finished())
        {
            // If the flow has ended remove it.
            it = active_flows.erase(it);
        }
        else
        {
            // Continue with the flow.
            flow.next(data); it++;
        }
    }

    // Check if any new flow starts.
    for (auto& flow : flows)
    {
        flow.start(data);

        if (!flow.is_finished())
        {
            // Move the flow to the active ones.
            active_flows.push_back(flow);
        }
    }



}
