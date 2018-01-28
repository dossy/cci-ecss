#include "ecss/utility.h"
#include <errno.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

using namespace std;
using namespace ecss_core;

static void _panic_printf(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

	if (fmt[0] != '\0' && fmt[strlen(fmt)-1] == ':') {
		fprintf(stderr, " %s", strerror(errno));
    }
    fputc('\n', stderr);

    abort();
}

void (*ecss_core::ecss_panic(const char* file,
                             int line,
                             const char* func))(const char*, ...)
{
    fprintf(stderr, "PANIC: %s:%d: %s: ", file, line, func);
    return _panic_printf;
}

string ecss_core::join(const char* sep, const vector<string>& v)
{
    string answer;
    for (int i = 0, n = v.size(); i < n; i++) {
        answer += v[i];
        if (i != n-1) {
            answer += sep;
        }
    }
    return answer;
}

string ecss_core::dequote(const string& s)
{
    int end = s.length();

    if (s[0] == '\'' && s[end - 1] == '\'') {       // single-quoted string
        string t;

        end--;
        for (int i = 1; i < end; i++) {
            if (s[i] == '\\') {
                if (++i == end) {
                    break;
                }
                if (s[i] == '\'') {
                    t += '\'';
                }
            }
            else {
                t += s[i];
            }
        }

        return t;
    }

    if (s[0] == '"' && s[end - 1] == '"') {         // double-quoted string
        string t;

        end--;
        for (int i = 1; i < end; i++) {
            if (s[i] == '\\') {
                if (++i == end) {
                    break;
                }
                switch (s[i]) {
                case '"':
                    t += '"';
                    break;
                case '\\':
                    t += '\\';
                    break;
                case '\0':
                    t += '\0';
                    break;
                case 'a':
                    t += '\a';
                    break;
                case 'b':
                    t += '\b';
                    break;
                case 't':
                    t += '\t';
                    break;
                case 'n':
                    t += '\n';
                    break;
                case 'v':
                    t += '\v';
                    break;
                case 'f':
                    t += '\f';
                    break;
                case 'r':
                    t += '\r';
                    break;
                default:
                    t += s[i];  // not a valid escape sequence
                    break;
                }
            }
            else {
                t += s[i];
            }
        }

        return t;
    }

    return s;                                       // not a quoted string
}

string ecss_core::format(const char* fmt, ...)
{
    va_list args;                       // ellided arguments
    int size = 200;                     // initial guess at size
    char* p = new char[size];

    for (;;) {
        va_start(args, fmt);
        int n = vsnprintf(p, size, fmt, args);
        va_end(args);

        if (n >= 0 && n < size) {
            break;
        }

        if (n >= 0) {
            size = n+1; // we know the exact size
        }
        else {
            size *= 2;  // obsolete vsnprintf, have to guess
        }

        delete [] p;
        p = new char[size];
    }

    string s(p);
    delete [] p;
    return s;
}

string ecss_core::erase_trailing_chars(const string& s, const char* char_set)
{
    const char* cs = char_set ? char_set : "\r\n";

    int i = s.length() - 1;
    for (; i >= 0; i--) {
        if (strchr(cs, s[i]) == 0) {
            break;  // s[i] is not in cs
        }
    }
    return s.substr(0, i + 1);
}
