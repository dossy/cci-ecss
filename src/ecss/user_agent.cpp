#include "ecss/user_agent.h"

using ecss_core::User_agent;

User_agent::User_agent(std::string name)
: m_name(name)
{}

User_agent::User_agent(std::string name, std::string version)
: m_name(name)
, m_version(version)
{}

bool User_agent::match(const User_agent& ua) const
{
    // If either name is empty, we automatically match.
    if (name().empty() || ua.name().empty())
        return true;

    // Since neither name is blank, the names must match exactly.
    if (name() != ua.name())
        return false;

    // If either version is empty, we automatically match.
    if (version().empty() || ua.version().empty())
        return true;

    // Since neither version is blank, the versions must match exactly.
    if (version() != ua.version())
        return false;

    // Both name and version are non-blank, and both match exactly.
    return true;
}
