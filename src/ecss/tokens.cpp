#include "ecss/tokens.h"
#include "ecss/utility.h"

using namespace ecss_core;

const char* token_to_string(Token tok)
{
    switch (tok)
    {
    case TOKEN_ERROR:           return "ERROR";
    case TOKEN_ADD_FLAVOR_SYM:  return "@theme";
    case TOKEN_AT:              return "@";
    case TOKEN_CDC:             return "-->";
    case TOKEN_CDO:             return "<!--";
    case TOKEN_CHARACTER:       return "CHARACTER";
    case TOKEN_CHARSET_SYM:     return "@charset";
    case TOKEN_CLASS:           return "CLASS";
    case TOKEN_COLON:           return "COLON";
    case TOKEN_COMMA:           return "COMMA";
    case TOKEN_CONCAT:          return "CONCAT";
    case TOKEN_DASH_MATCH:      return "|=";
    case TOKEN_DIMENSION:       return "DIMENSION";
    case TOKEN_DOUBLE_COLON:    return "::";
    case TOKEN_EQUALS:          return "EQUALS";
    case TOKEN_FLAVOR_SYM:      return "@define-theme";
    case TOKEN_FUNCTION:        return "FUNCTION";
    case TOKEN_GREATER:         return "GREATER";
    case TOKEN_HASH:            return "HASH";
    case TOKEN_IDENT:           return "IDENT";
    case TOKEN_IMPORTANT_SYM:   return "IMPORTANT_SYM";
    case TOKEN_IMPORT_SYM:      return "@import";
    case TOKEN_INCLUDES:        return "~=";
    case TOKEN_INCLUDE_SYM:     return "@include";
    case TOKEN_LBRACE:          return "LBRACE";
    case TOKEN_LBRACKET:        return "LBRACKET";
    case TOKEN_LESS:            return "LESS";
    case TOKEN_LPAREN:          return "LPAREN";
    case TOKEN_MEDIA_SYM:       return "@media";
    case TOKEN_MINUS:           return "MINUS";
    case TOKEN_NUMBER:          return "NUMBER";
    case TOKEN_PAGE_SYM:        return "@page";
    case TOKEN_PERCENTAGE:      return "PERCENTAGE";
    case TOKEN_PLUS:            return "PLUS";
    case TOKEN_PRAGMA_SYM:      return "@pragma";
    case TOKEN_PREFIX_MATCH:    return "^=";
    case TOKEN_RBRACE:          return "RBRACE";
    case TOKEN_RBRACKET:        return "RBRACKET";
    case TOKEN_READONLY:        return "readonly";
    case TOKEN_RPAREN:          return "RPAREN";
    case TOKEN_S:               return "WHITESPACE";
    case TOKEN_SEMICOLON:       return "SEMICOLON";
    case TOKEN_SLASH:           return "SLASH";
    case TOKEN_STAR:            return "STAR";
    case TOKEN_STRING:          return "STRING";
    case TOKEN_SUBSTRING_MATCH: return "*=";
    case TOKEN_SUFFIX_MATCH:    return "$=";
    case TOKEN_TILDE:           return "TILDE";
    case TOKEN_UNCLOSED_STRING: return "UNCLOSED_STRING";
    case TOKEN_URI:             return "URI";
    case TOKEN_URL_SYM:         return "url";
    case TOKEN_VARIABLE:        return "VARIABLE";
    }
    ECSS_PANIC(("Invalid token: %d", tok));
    return NULL; // not reached
}
