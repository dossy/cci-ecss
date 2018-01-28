#ifndef included_ecss_lexer
#define included_ecss_lexer

#include "ecss/tokens.h"
#include "ecss/utility.h"
#include <cstdio>
#include <string>

namespace ecss_core {

class Lexer : boost::noncopyable {
public:
    Lexer(const std::string& input);
    ~Lexer();
    bool preserve_ws(bool b);
    bool advance();
    void insert(Token token);
    Token token();
    Token lookahead(int n);
    const std::string& lexeme();
    int line();
    int column();
    bool eof();

private:
    void discard_ws_unless_preserved();

private:
    struct Token_stream;
    Token_stream* m_stream;
    bool m_preserve_ws;
};

// "Location" is insufficiently descriptive; possibly rename Source_location?
class Location {
public:
    Location()
    : m_line(-1)
    , m_column(-1)
    {}

    Location(const std::string& filename, int line, int column)
    : m_filename(filename)
    , m_line(line)
    , m_column(column)
    {}

    void set(const std::string& filename, int line, int column)
    {
        m_filename = filename;
        m_line = line;
        m_column = column;
    }

    const std::string& filename() const { return m_filename; }
    int line() const { return m_line; }
    int column() const { return m_column; }
    std::string to_string() const;

private:
    std::string m_filename;
    int m_line;
    int m_column;
};

} // namespace ecss_core

#endif
