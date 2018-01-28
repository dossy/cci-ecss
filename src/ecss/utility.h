#ifndef included_ecss_utility
#define included_ecss_utility

#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>
#include <cstdarg>
#include <cstdlib>
#include <string>
#include <sys/types.h>  // for u_int32_t
#include <vector>

namespace ecss_core {

//
// This macro provides a simple printf-and-abort facility. It's similar to the
// standard assert macro, except this one takes printf-style arguments instead
// of a boolean expression. Note that the macro arguments must be doubly
// parenthesized, e.g. ECSS_PANIC(("Hello, %s!", "world"));
//
#define ECSS_PANIC(args)                                    \
    ecss_panic(__FILE__, __LINE__, __PRETTY_FUNCTION__) args

// (Used internally by the ECSS_PANIC macro; never call it directly.)
void (*ecss_panic(const char*, int, const char*))(const char*, ...);

//
// Concatenates the strings in the vector v using the given separator (sep)
// and returns the result.
//
std::string join(const char* sep, const std::vector<std::string>& v);

//
// If a given string begins and ends with either single or double quotes,
// converts the escape sequences in the string to ASCII characters, removes
// the surrounding quote marks, and returns the new string. If the string
// doesn't begin and end with the same quote marks, returns the original. If
// the first character is a single quote, the string is considered
// single-quoted, and only the escaped single quote (\') is unescaped.
// Otherwise, the string is considered double-quoted, and the following
// conversions take place:
//
//      \"   ->  "
//      \\\\ ->  \\ (ASCII 92)
//      \0   ->  null (ASCII 0)
//      \a   ->  bell (ASCII 7)
//      \b   ->  backspace (ASCII 8)
//      \t   ->  tab (ASCII 9)
//      \n   ->  newline (ASCII 10)
//      \v   ->  vertical tab (ASCII 11)
//      \f   ->  form feed (ASCII 12)
//      \r   ->  carriage return (ASCII 13)
//
std::string dequote(const std::string& s);

//
// Similar to printf, this function returns a std::string object instead of a
// character pointer. This is basically a better snprintf in many situations.
//
std::string format(const char* fmt, ...);

//
// Removes from the end of s any character in appearing in char_set. If
// char_set is not given, it defaults to "\r\n".
//
std::string erase_trailing_chars(const std::string& s, const char* char_set=0);

} // namespace ecss_core

#endif
