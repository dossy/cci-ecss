#ifndef included_ecss_error
#define included_ecss_error

#include "ecss/lexer.h"
#include <string>

namespace ecss_core {

struct Error {
    virtual ~Error();
    virtual std::string to_string() const = 0;
};

// A system call failed.
struct System_error : public Error {
    System_error(const char* syscall, int errnum);
    std::string to_string() const;
private:
    std::string m_syscall;
    int m_errnum;
};

// An I/O syscall failed.
struct IO_error : public System_error {
    IO_error(const char* syscall, int errnum)
    : System_error(syscall, errnum) {}
};

// Generic compile-time error.
class Compile_error : public Error {
public:
    Compile_error(const Location& location, std::string what);
    const Location& location() const { return m_location; }
    const std::string& what() const { return m_what; }
    std::string to_string() const;
private:
    Location m_location;
    std::string m_what;
};

// An error raised by the regular expression engine.
struct Regex_error : public Error {
    Regex_error(std::string pattern, std::string error);
    std::string to_string() const;
private:
    std::string m_pattern;
    std::string m_error;
};

} // namespace ecss_core

#endif
