#include "ecss/lexer.h"
#include "ecss/file_util.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace ecss_core;

int main(int argc, char** argv)
{
    Lexer lex(read_stdin());
    lex.preserve_ws(true);
    while (!lex.eof()) {
        printf("%3d %2d: %-15s (%s)\n",
               lex.line(),
               lex.column(),
               token_to_string(lex.token()),
               lex.token()==TOKEN_S ? " " : lex.lexeme().c_str());
        lex.advance();
    }
    return 0;
}
