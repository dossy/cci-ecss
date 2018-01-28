#ifndef included_ecss_regex
#define included_ecss_regex

#include "ecss/utility.h"
#include <boost/shared_ptr.hpp>
#include <string>

namespace ecss_core {

// Simple wrapper class for the POSIX regular expression library.
class Regex {
public:
    explicit Regex(std::string pattern);
    Regex(std::string pattern, bool ignore_case);
    bool match(const std::string& s);

private:
    class Impl;
    boost::shared_ptr<Impl> m_impl;
};

} // namespace ecss_core

#endif
