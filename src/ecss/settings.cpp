#include "ecss/settings.h"
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace ecss_core;

// ^mozilla.*msie 5[.]0;
// msie 5.0
//
// ^mozilla.*msie 5[.]5;
// msie 5.5
//
// ^mozilla.*msie 6[.]0;
// msie 6.0
//
// ^mozilla.*msie 7[.]0;
// msie 7.0
//
// ^mozilla.*msie
// msie
//
// ^mozilla.*gecko
// mozilla
//
// ^mozilla.*gecko.*firefox
// firefox
//
// ^mozilla.*applewebkit.*safari
// safari
//
// ^lynx
// lynx
//
// opera
// opera

Compiler_settings::Compiler_settings()
: m_preserve_order(false)
{
    add_user_agent_regexp("^mozilla.*msie 5[.]0", User_agent("msie", "5.0"));
    add_user_agent_regexp("^mozilla.*msie 5[.]5", User_agent("msie", "5.5"));
    add_user_agent_regexp("^mozilla.*msie 6[.]0", User_agent("msie", "6.0"));
    add_user_agent_regexp("^mozilla.*msie 7[.]0", User_agent("msie", "7.0"));
    add_user_agent_regexp("^mozilla.*msie", User_agent("msie"));
    add_user_agent_regexp("^mozilla.*gecko", User_agent("mozilla"));
    add_user_agent_regexp("^mozilla.*apple.*safari", User_agent("safari"));
    add_user_agent_regexp("^lynx", User_agent("lynx"));
    add_user_agent_regexp("opera", User_agent("opera"));
}

void Compiler_settings::set_include_path(std::string path)
{
    m_include_dirs.clear();
    vector<string> parts;
    boost::split(parts, path, boost::is_any_of(":"));
    for (int i = 0, n = parts.size(); i < n; i++) {
        if (!parts[i].empty()) {
            add_include_dir(parts[i]);
        }
    }
}

void Compiler_settings::add_include_dir(string dir)
{
    m_include_dirs.push_back(dir);
}

const vector<string>& Compiler_settings::include_dirs() const
{
    return m_include_dirs;
}

void Compiler_settings::add_user_agent_regexp(const string& regexp,
                                              const User_agent& user_agent)
{
    m_user_agent_regexps.push_back(make_pair(Regex(regexp), user_agent));
}

void Compiler_settings::set_user_agent_string(const string& s)
{
    m_user_agent_string = s;
    m_user_agent = User_agent();
    for (int i = 0; i < int(m_user_agent_regexps.size()); i++) {
        if (m_user_agent_regexps[i].first.match(s)) {
            m_user_agent = m_user_agent_regexps[i].second;
            return;
        }
    }
}

void Compiler_settings::set_user_agent(const User_agent& ua)
{
    m_user_agent = ua;
}

const User_agent& Compiler_settings::user_agent() const
{
    return m_user_agent;
}

void Compiler_settings::set_preserve_order(bool b)
{
    m_preserve_order = b;
}

bool Compiler_settings::preserve_order() const
{
    return m_preserve_order;
}
