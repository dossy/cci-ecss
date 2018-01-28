#include "ecss/file_util.h"
#include "ecss/error.h"
#include "ecss/utility.h"
#include <cerrno>
#include <vector>

// UNIX headers
#include <limits.h>         // for PATH_MAX
#include <sys/stat.h>       // for S_xxx file mode constants and umask(2)
#include <sys/types.h>      // basic system data types
#include <unistd.h>

using namespace std;

namespace
{
    // Character constant used to separate directories on this platform.
    char const DIR_SEP = '/';

    // Sometimes we need to use DIR_SEP as if it were a c-style string.
    char const DIR_SEP_CSTR[] = "/";
}

string ecss_core::make_full_path(string path)
{
    vector<char> absolute_path(PATH_MAX);
    if (!realpath(path.c_str(), &absolute_path[0])) {
        return "";
    }
    return string(&absolute_path[0]);
}

bool ecss_core::is_relative_path(string path)
{
    return path.length() == 0 || path[0] != DIR_SEP;
}

bool ecss_core::is_readable_file(string filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != 0) {
        return false; // stat(2) failed
    }
    if (!S_ISREG(buf.st_mode)) {
        return false; // not a regular file
    }
    if (FILE* fp = fopen(filename.c_str(), "r")) {
        fclose(fp);
        return true;
    }
    return false;     // couldn't open for reading
}

string ecss_core::base_name(string path)
{
    // path might end with one or more DIR_SEP chars; remove them.
    const string s = erase_trailing_chars(path, DIR_SEP_CSTR);

    // If path was either empty or nothing but DIR_SEP chars, return "".
    if (s.empty()) return "";

    // Find the position of the last DIR_SEP char.
    string::size_type i = s.find_last_of(DIR_SEP);

    // Was the path relative? If so, s is a filename, so simply return it.
    if (i == string::npos) return s;

    // Return the substring containing characters i+1 through the end.
    return s.substr(i+1);
}

string ecss_core::dir_name(string path)
{
    // path might end with one or more DIR_SEP chars; remove them.
    const string s = erase_trailing_chars(path, DIR_SEP_CSTR);

    // If path was either empty or nothing but DIR_SEP chars, return DIR_SEP.
    if (s.empty()) return DIR_SEP_CSTR;

    // Find the position of the last DIR_SEP char.
    string::size_type i = s.find_last_of(DIR_SEP);

    // Was the path relative? If so, we can only return the empty string.
    if (i == string::npos) return "";

    // Return the substring containing characters 0 through i.
    return s.substr(0, i+1);
}

string ecss_core::read_file(const string& path)
{
    FILE* fp = fopen(path.c_str(), "r");
    if (!fp) {
        throw IO_error("fopen", errno);
    }
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    vector<char> s(file_size);
    fread(&s[0], sizeof(char), file_size, fp);
    if (fclose(fp) != 0) {
        throw IO_error("fclose", errno);
    }
    return string(&s[0], file_size);
}

void ecss_core::write_file(const string& path, const string& s)
{
    FILE* fp = fopen(path.c_str(), "w");
    if (!fp) {
        throw IO_error("fopen", errno);
    }
    fputs(s.c_str(), fp);
    if (fclose(fp) != 0) {
        throw IO_error("fclose", errno);
    }
}

string ecss_core::read_stdin()
{
    string answer;
    char buf[1000];
    while (!feof(stdin)) {
        if (size_t num_read = fread(buf, 1, sizeof(buf), stdin)) {
            answer.append(buf, num_read);
        }
    }
    return answer;
}

