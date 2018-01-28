#ifndef included_ecss_scanner_aux
#define included_ecss_scanner_aux

#define YY_EXTRA_TYPE Lexer_position

//
// Used by the lexical analyzer (see scanner.l) to keep track of the current
// position in the input being scanned. Both lines and columns are 0-based.
// NOTE: This class returns the line and column of the position immediately
// following the most recently scanned token; thus, to determine the starting
// column of the current token you must first subtract the token's length.
//
class Lexer_position {
public:
    Lexer_position()
    : m_line(-1)
    , m_column(0)
    , m_bol(true)
    {}

    void advance(int n)
    {
        if (m_bol) {
            m_bol = false;
            m_line++;
            m_column = n;
        }
        else {
            m_column += n;
        }
    }

    void newline()
    {
        advance(0);
        m_bol = true;
        m_column += 1;
    }

    int line() const
    {
        return m_line;
    }

    int column() const
    {
        return m_column;
    }

private:
    int m_line;
    int m_column;
    int m_bol;
};

#endif
