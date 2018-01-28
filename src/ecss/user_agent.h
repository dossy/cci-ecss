#ifndef included_ecss_user_agent
#define included_ecss_user_agent

#include <string>

namespace ecss_core {

class User_agent {
public:
    User_agent() {}
    explicit User_agent(std::string name);
    User_agent(std::string name, std::string version);
    const std::string& name() const { return m_name; }
    const std::string& version() const { return m_version; }
    bool match(const User_agent&) const;

private:
    std::string m_name;
    std::string m_version;
};

} // namespace ecss_core

#endif
