#include "ecss/lexer.h"
#include "ecss/scanner_aux.h"  // (must precede "ecss/scanner.l.h")
#include "ecss/scanner.l.h"
#include "ecss/utility.h"
#include <cassert>
#include <deque>
#include <string>

using namespace std;
using ecss_core::Lexer;
using ecss_core::Location;

// +------------+
// | Token_info |
// +------------+

class Token_info {
public:
    Token_info(Token token, const char* lexeme, int line, int column)
    : m_token(token)
    , m_lexeme(lexeme)
    , m_line(line)
    , m_column(column)
    {}

    Token token() const { return m_token; }
    const string& lexeme() const { return m_lexeme; }
    int line() const { return m_line; }
    int column() const { return m_column; }

private:
    Token m_token;      // token identifier
    string m_lexeme;    // corresponding symbol
    int m_line;         // line number
    int m_column;       // column number
};

// +---------------------+
// | Lexer::Token_stream |
// +---------------------+

class Lexer::Token_stream {
public:
    Token_stream(const string& input)
    : m_eof(false)
    , m_error_token(TOKEN_ERROR, "", -1, -1)
    {
        yylex_init(&m_scanner);
        yyset_extra(Lexer_position(), m_scanner);
        m_buffer_state = yy_scan_string(input.c_str(), m_scanner);
    }

    ~Token_stream()
    {
        yy_delete_buffer(m_buffer_state, m_scanner);
        yylex_destroy(m_scanner);
    }

    void insert(Token token)
    {
        m_tokens.push_front(Token_info(token, "", -1, -1));
    }

    void discard()
    {
        if (m_tokens.empty()) {
            nth(0);
            if (m_tokens.empty()) {
                return;
            }
        }
        m_tokens.pop_front();
    }

    Token token(int n)
    {
        return nth(n).token();
    }

    const string& lexeme(int n)
    {
        return nth(n).lexeme();
    }

    int line(int n)
    {
        return nth(n).line();
    }

    int column(int n)
    {
        return nth(n).column();
    }

    bool eof()
    {
        return m_eof && nth(0).token()==TOKEN_ERROR;
    }

private:
    const Token_info& nth(int n);

private:
    yyscan_t m_scanner;             // opaque Flex reentrant scanner
    YY_BUFFER_STATE m_buffer_state; // Flex buffer handle
    deque<Token_info> m_tokens;     // list of scanned tokens
    bool m_eof;                     // true when input has been exhausted
    const Token_info m_error_token; // represents a token after end of stream
};

const Token_info& Lexer::Token_stream::nth(int n)
{
    assert(n >= 0);

    if (n < int(m_tokens.size())) {
        return m_tokens[n];
    }

    if (m_eof) {
        return m_error_token;
    }

    while (n >= int(m_tokens.size())) {
        if (int tok = yylex(m_scanner)) {
            int column = yyget_extra(m_scanner).column()-yyget_leng(m_scanner);
            m_tokens.push_back(Token_info(static_cast<Token>(tok),
                                          yyget_text(m_scanner),
                                          yyget_extra(m_scanner).line(),
                                          column));
        }
        else {
            m_eof = true;
            return m_error_token;
        }
    }
    return m_tokens[n];
}

// +-------+
// | Lexer |
// +-------+

Lexer::Lexer(const string& input)
: m_stream(new Token_stream(input))
, m_preserve_ws(false)
{}

Lexer::~Lexer()
{
    delete m_stream;
}

bool Lexer::preserve_ws(bool b)
{
    const bool old = m_preserve_ws;
    m_preserve_ws = b;
    return old;
}

bool Lexer::advance()
{
    m_stream->discard();
    discard_ws_unless_preserved();
    return token() != TOKEN_ERROR;
}

void Lexer::insert(Token token)
{
    m_stream->insert(token);
}

Token Lexer::token()
{
    discard_ws_unless_preserved();
    return m_stream->token(0);
}

Token Lexer::lookahead(int n)
{
    if (m_preserve_ws) {
        return m_stream->token(n);
    }

    Token token = TOKEN_ERROR;
    for (int i = 0; n >= 0; i++) {
        if ((token = m_stream->token(i)) != TOKEN_S) {
            --n;
        }
    }
    return token;
}

const string& Lexer::lexeme()
{
    discard_ws_unless_preserved();
    return m_stream->lexeme(0);
}

int Lexer::line()
{
    discard_ws_unless_preserved();
    return m_stream->line(0);
}

int Lexer::column()
{
    discard_ws_unless_preserved();
    return m_stream->column(0);
}

bool Lexer::eof()
{
    return m_stream->eof();
}

void Lexer::discard_ws_unless_preserved()
{
    if (!m_preserve_ws) {
        while (m_stream->token(0) == TOKEN_S) {
            m_stream->discard();
        }
    }
}

// +----------+
// | Location |
// +----------+

string Location::to_string() const
{
    return format("%s:%d:%d",
                  filename().empty() ? "<>" : filename().c_str(),
                  line() + 1,
                  column());
}
