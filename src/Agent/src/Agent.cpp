#include <uxr/agent/AgentInstance.hpp>

int main(int argc, char** argv)
{
    eprosima::uxr::AgentInstance& agent_instance = agent_instance.getInstance();

    if (!agent_instance.create(argc, argv))
    {
        return 1;
    }
    agent_instance.run();

    return 0;
}
