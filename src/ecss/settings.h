#ifndef included_ecss_compiler_settings
#define included_ecss_compiler_settings

#include "ecss/regex.h"
#include "ecss/user_agent.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

namespace ecss_core {

class Compiler_settings {
public:
    Compiler_settings();
    void set_include_path(std::string path);
    void add_include_dir(std::string dir);
    const std::vector<std::string>& include_dirs() const;
    void add_user_agent_regexp(const std::string& regexp, const User_agent&);
    void set_user_agent(const User_agent&);
    void set_user_agent_string(const std::string&);
    const User_agent& user_agent() const;
    void set_preserve_order(bool);
    bool preserve_order() const;

private:
    typedef std::pair<Regex, User_agent> Regexp_user_agent_pair;

    std::vector<std::string> m_include_dirs;
    std::vector<Regexp_user_agent_pair> m_user_agent_regexps;
    std::string m_user_agent_string;
    User_agent m_user_agent;
    bool m_preserve_order;
};

typedef boost::shared_ptr<Compiler_settings> Compiler_settings_ptr;

} // namespace ecss_core

#endif
