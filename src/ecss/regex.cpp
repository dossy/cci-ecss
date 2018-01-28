#include "ecss/regex.h"
#include "ecss/error.h"
#include <regex.h>
#include <vector>

using namespace std;
using ecss_core::Regex;

//
// +-------------+
// | Regex::Impl |
// +-------------+
//

class Regex::Impl {
public:
    Impl(string pattern, bool ignore_case)
    {
        int flags = REG_EXTENDED | REG_NOSUB | (ignore_case ? REG_ICASE : 0);
        int errcode;
        if ((errcode = regcomp(&m_regex, pattern.c_str(), flags)) != 0) {
            vector<char> buf(regerror(errcode, &m_regex, 0, 0));
            regerror(errcode, &m_regex, &buf[0], buf.size());
            throw Regex_error(pattern, string(&buf[0]));
        }
    }

    ~Impl()
    {
        regfree(&m_regex);
    }

    bool match(const string& s)
    {
        return regexec(&m_regex, s.c_str(), 0, 0, 0) == 0;
    }

private:
    regex_t m_regex;
};

//
// +-------+
// | Regex |
// +-------+
//

Regex::Regex(string pattern)
: m_impl(new Impl(pattern, true))
{}

Regex::Regex(string pattern, bool ignore_case)
: m_impl(new Impl(pattern, ignore_case))
{}

bool Regex::match(const string& s)
{
    return m_impl->match(s);
}
