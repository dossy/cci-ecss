#ifndef included_ecss_file_util
#define included_ecss_file_util

#include <cstdio>
#include <string>

namespace ecss_core {

//
// Converts a possibly relative path to an absolute path, resolving any
// symbolic links. If the path does not exist or if an error occurs, returns
// an empty string.
//
std::string make_full_path(std::string path);

//
// Returns true if and only if the given path is relative.
//
bool is_relative_path(std::string path);

//
// Returns true if the specified file exists and is readable, false otherwise.
//
bool is_readable_file(std::string filename);

//
// Strips the parent directory from the given path and returns the result. If
// path is relative, returns the path unadulterated. If path is empty or
// consists or nothing but directory separator characters, return an empty
// string. Trailing directory separator characters are ignored.
//
std::string base_name(std::string path);

//
// Similar to base_name, except this function returns the part of the path
// that base_name removes. If path is relative, returns an empty string.
//
std::string dir_name(std::string path);

//
// Reads an entire file into memory and returns it as a string.
//
std::string read_file(const std::string& path);

//
// Writes the string s to the file at the specified path.
//
void write_file(const std::string& path, const std::string& s);

//
// Reads from stdin until end-of-file and returns the accumulated data as a
// single string.
//
std::string read_stdin();

} // namespace ecss_core

#endif
