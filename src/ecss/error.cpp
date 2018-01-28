#include "ecss/error.h"
#include <cstring>

using namespace std;
using namespace ecss_core;

Error::~Error() {}

System_error::System_error(const char* syscall, int errnum)
: m_syscall(syscall)
, m_errnum(errnum)
{}

string System_error::to_string() const
{
    string answer("syscall \"");
    answer += m_syscall;
    answer += "\" failed [";
    answer += boost::lexical_cast<string>(m_errnum);
    answer += "]: ";
    answer += strerror(m_errnum);
    return answer;
}

Compile_error::Compile_error(const Location& location, string what)
: m_location(location)
, m_what(what)
{}

string Compile_error::to_string() const
{
    return m_location.to_string() + ": " + m_what;
}

Regex_error::Regex_error(string pattern, string error)
: m_pattern(pattern)
, m_error(error)
{}

string Regex_error::to_string() const
{
    return "error in regex \"" + m_pattern + "\": " + m_error;
}
