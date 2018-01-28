#include "ecss/parser.h"
#include "ecss/ast.h"
#include "ecss/error.h"
#include "ecss/file_util.h"
#include "ecss/lexer.h"
#include "ecss/utility.h"
#include <cassert>
#include <cstdarg>
#include <map>
#include <memory>
#include <vector>

using namespace std;
using namespace ecss_core;

//#define ECSS_DEBUG_PARSER 1

namespace
{
    Selector::Combinator token_to_selector_combinator(Token token)
    {
        switch (token) {
        case TOKEN_S:
            return Selector::DESCENDANT;
        case TOKEN_GREATER:
            return Selector::CHILD;
        case TOKEN_PLUS:
            return Selector::ADJACENT_SIBLING;
        case TOKEN_TILDE:
            return Selector::GENERAL_SIBLING;
        default:
            ECSS_PANIC(("Invalid selector combinator token: %d.", token));
        }
        assert(0); // not reached
    }

    Attribute_selector::Operator token_to_attribute_selector_op(Token token)
    {
        switch (token) {
        case TOKEN_EQUALS:
            return Attribute_selector::EQUALS;
        case TOKEN_INCLUDES:
            return Attribute_selector::INCLUDES;
        case TOKEN_DASH_MATCH:
            return Attribute_selector::DASH_MATCH;
        case TOKEN_SUBSTRING_MATCH:
            return Attribute_selector::SUBSTRING_MATCH;
        case TOKEN_PREFIX_MATCH:
            return Attribute_selector::PREFIX_MATCH;
        case TOKEN_SUFFIX_MATCH:
            return Attribute_selector::SUFFIX_MATCH;
        default:
            ECSS_PANIC(("Invalid attribute selector op token: %d.", token));
        }
        assert(0); // not reached
    }

    Binary_expr::Operator token_to_binary_expr_op(Token token)
    {
        switch (token) {
        case TOKEN_COMMA:
            return Binary_expr::COMMA;
        case TOKEN_EQUALS:
            return Binary_expr::EQUALS;
        case TOKEN_PLUS:
            return Binary_expr::PLUS;
        case TOKEN_MINUS:
            return Binary_expr::MINUS;
        case TOKEN_STAR:
            return Binary_expr::STAR;
        case TOKEN_SLASH:
            return Binary_expr::SLASH;
        case TOKEN_CONCAT:
            return Binary_expr::CONCAT;
        default:
            ECSS_PANIC(("Invalid binary expr op token: %d.", token));
        }
        assert(0); // not reached
    }

    Unary_expr::Operator token_to_unary_expr_op(Token token)
    {
        switch (token) {
        case TOKEN_MINUS:
            return Unary_expr::MINUS;
        case TOKEN_PLUS:
            return Unary_expr::PLUS;
        default:
            ECSS_PANIC(("Invalid unary expr op token: %d.", token));
        }
        assert(0); // not reached
    }

    string format_curr_token_for_error_msg(Lexer& lex)
    {
        if (lex.token() == TOKEN_ERROR)
            return "end of file";

        string answer = "the symbol \"";
        answer += (lex.token()==TOKEN_S ? " " : lex.lexeme().c_str());
        answer += '"';
        return answer;
    }
}

// +------------------+
// | Debug_func_entry |
// +------------------+

class Debug_func_entry {
public:
    Debug_func_entry(const char* function_name, Lexer& lex)
    : m_function_name(function_name)
    {
        #ifdef ECSS_DEBUG_PARSER
        indent();
        fprintf(stderr, "--> %s (line %d)\n", m_function_name, lex.line());
        s_depth++;
        #endif
    }
    ~Debug_func_entry()
    {
        #ifdef ECSS_DEBUG_PARSER
        s_depth--;
        indent();
        fprintf(stderr, "<-- %s\n", m_function_name);
        #endif
    }
private:
    void indent()
    {
        for (int i = 0; i < s_depth; i++)
            fputs("  ", stderr);
    }
    const char* m_function_name;
    static int s_depth;
};

int Debug_func_entry::s_depth = 0;

// +--------+
// | macros |
// +--------+

#define PRODUCTION_START_LINE prod_start_line_
#define PRODUCTION_START_COLUMN prod_start_column_

#define BEGIN_PRODUCTION                                                \
    Debug_func_entry func_entry_debug_(__FUNCTION__, m_lexer);          \
    int PRODUCTION_START_LINE   __attribute__((unused)) = m_lexer.line(); \
    int PRODUCTION_START_COLUMN __attribute__((unused)) = m_lexer.column();

#define CREATE_AST_NODE(var_name, type_name, ...) do {  \
        var_name.reset(new type_name(__VA_ARGS__));     \
        var_name->annotate(m_filename,                  \
                           PRODUCTION_START_LINE,       \
                           PRODUCTION_START_COLUMN);    \
    } while(0)

#define RETURN_AST_NODE(type_name, ...) do {                \
        type_name##_ptr rv_(new type_name(__VA_ARGS__));    \
        rv_->annotate(m_filename,                           \
                      PRODUCTION_START_LINE,                \
                      PRODUCTION_START_COLUMN);             \
        return rv_;                                         \
    } while (0)

// +-------------+
// | Preserve_ws |
// +-------------+

class Preserve_ws {
public:
    Preserve_ws(Lexer& lexer, bool b)
    : m_lexer(lexer), m_old_value(lexer.preserve_ws(b)) {}
    ~Preserve_ws() { m_lexer.preserve_ws(m_old_value); }
private:
    Lexer& m_lexer;
    const bool m_old_value;
};

#define PRESERVING_WS(code) do {                \
        Preserve_ws pws__(m_lexer, true);       \
        code;                                   \
    } while (0)

// Used to unwind the stack when we fail to parse a non-terminal.
struct Parse_failure_exception {};

// +-----------------+
// | Auto_inc_dec<T> |
// +-----------------+

template<typename T>
class Auto_inc_dec {
public:
    explicit Auto_inc_dec(T& val) : m_val(val) { ++m_val; }
    ~Auto_inc_dec() { --m_val; }

private:
    T& m_val;
};

// +--------------+
// | Parser::Impl |
// +--------------+

typedef std::map<std::string, Location> Include_map;
typedef boost::shared_ptr<Include_map> Include_map_ptr;

class Parser::Impl {
public:
    Impl(const string& input_buffer,
         const string& filename,
         Compiler_settings_ptr cs,
         Include_map_ptr include_map);
    Stylesheet_ptr parse();
    const vector<Compile_error>& compile_errors() const;

private:
    FILE* open_file();

    Stylesheet_ptr parse_stylesheet();
    void parse_pragma();
    void parse_include(Stylesheet_ptr);
    Import_ptr parse_import();
    string parse_url();
    void parse_medium_list(Has_media*);
    Variable_declaration_ptr parse_variable_declaration();
    Flavor_declaration_ptr parse_flavor_declaration();
    Media_block_ptr parse_media_block();
    void parse_page();
    Ruleset_ptr parse_ruleset();
    void parse_ruleset_block(Ruleset_ptr ruleset);
    Ruleset_declaration_ptr parse_ruleset_element();
    Selector_ptr parse_selector();
    Token parse_optional_combinator();
    Simple_selector_sequence_ptr parse_simple_selector_sequence();
    Simple_selector_ptr parse_simple_selector();
    Class_selector_ptr parse_class_selector();
    ID_selector_ptr parse_id_selector();
    Attribute_selector_ptr parse_attribute_selector();
    Pseudo_selector_ptr parse_pseudo_selector();
    Property_ptr parse_property();
    User_agent parse_property_user_agent();
    Expr_ptr parse_expr();
    Expr_ptr parse_binary_expr(Expr_ptr lhs);
    Expr_ptr parse_unary_expr();
    Function_term_ptr parse_function_term();

    string resolve_included_path(string path);
    bool try_any_expr_token();
    string match(Token);
    Token expect_any_of(Token, ...);
    bool try_any_of(Token, ...);
    bool try_any_of_impl(const vector<Token>&);
    void skip(Token);
    void skip_any_of(Token, ...);
    void consume(Token);
    void parse_failure(vector<Token> expected_tokens);
    void parse_failure(Token expected_token);
    void add_compile_error(string msg, int line, int column);
    void add_compile_errors(vector<Compile_error> errors);

private:
    const string m_filename;
    Compiler_settings_ptr m_compiler_settings;
    Lexer m_lexer;
    Include_map_ptr m_include_map;
    int m_ruleset_depth;
    vector<Compile_error> m_errors;
};

Parser::Impl::Impl(const string& input_buffer,
                   const string& filename,
                   Compiler_settings_ptr cs,
                   Include_map_ptr include_map)
: m_filename(filename)
, m_compiler_settings(cs)
, m_lexer(input_buffer)
, m_include_map(include_map)
, m_ruleset_depth(0)
{}

Stylesheet_ptr Parser::Impl::parse()
{
    try {
        return parse_stylesheet();
    }
    catch (Parse_failure_exception) {
        return Stylesheet_ptr();
    }
}

const vector<Compile_error>& Parser::Impl::compile_errors() const
{
    return m_errors;
}

Stylesheet_ptr Parser::Impl::parse_stylesheet()
{
    BEGIN_PRODUCTION;

    Stylesheet_ptr stylesheet;
    CREATE_AST_NODE(stylesheet, Stylesheet);

    if (m_lexer.token() == TOKEN_CHARSET_SYM) {
        consume(TOKEN_CHARSET_SYM);
        stylesheet->set_charset(match(TOKEN_STRING));
        match(TOKEN_SEMICOLON);
    }
    skip_any_of(TOKEN_S, TOKEN_CDO, TOKEN_CDC, 0);
    while (m_lexer.token() == TOKEN_IMPORT_SYM) {
        stylesheet->add_import(parse_import());
        skip_any_of(TOKEN_S, TOKEN_CDO, TOKEN_CDC, 0);
    }
    while (!m_lexer.eof()) {
        if (m_lexer.token() == TOKEN_PRAGMA_SYM)
            parse_pragma();
        else if (m_lexer.token() == TOKEN_INCLUDE_SYM)
            parse_include(stylesheet);
        else if (m_lexer.token() == TOKEN_MEDIA_SYM)
            stylesheet->add_media_block(parse_media_block());
        else if (m_lexer.token() == TOKEN_VARIABLE)
            stylesheet->add_variable_declaration(parse_variable_declaration());
        else if (m_lexer.token() == TOKEN_FLAVOR_SYM)
            stylesheet->add_flavor_declaration(parse_flavor_declaration());
        else
            stylesheet->add_ruleset(parse_ruleset());
        skip_any_of(TOKEN_S, TOKEN_CDO, TOKEN_CDC, 0);
    }
    return stylesheet;
}

void Parser::Impl::parse_pragma()
{
    BEGIN_PRODUCTION;

    consume(TOKEN_PRAGMA_SYM);
    const string pragma = match(TOKEN_IDENT);
    match(TOKEN_LPAREN);
    if (pragma == "set_user_agent") {
        m_compiler_settings->set_user_agent(parse_property_user_agent());
    }
    else if (pragma == "set_include_path") {
        m_compiler_settings->set_include_path(dequote(match(TOKEN_STRING)));
    }
    else if (pragma == "preserve_order") {
        m_compiler_settings->set_preserve_order(true);
    }
    else {
        add_compile_error(format("Invalid pragma \"%s\".", pragma.c_str()),
                          PRODUCTION_START_LINE,
                          PRODUCTION_START_COLUMN);
    }
    match(TOKEN_RPAREN);
    match(TOKEN_SEMICOLON);
}

void Parser::Impl::parse_include(Stylesheet_ptr parent)
{
    BEGIN_PRODUCTION;

    consume(TOKEN_INCLUDE_SYM);
    const string included_path =
        expect_any_of(TOKEN_STRING, TOKEN_URL_SYM, 0) == TOKEN_URL_SYM
            ? parse_url()
            : dequote(match(TOKEN_STRING));
    const string filename = resolve_included_path(included_path);
    match(TOKEN_SEMICOLON);

    // If resolve_included_path returned an empty string, we were unable to
    // either find the included file or open it for reading.
    if (filename.empty()) {
        add_compile_error(
            format("Unable to read included stylesheet \"%s\".",
                   included_path.c_str()),
            PRODUCTION_START_LINE,
            PRODUCTION_START_COLUMN);
        return;
    }

    // If this file has already been included, don't include it again.
    if (m_include_map->find(filename) != m_include_map->end()) {
        return;
    }
    (*m_include_map)[filename] =
        Location(m_filename, PRODUCTION_START_LINE, PRODUCTION_START_COLUMN);

    // Try to read the entire file into memory, bailing out with a compile
    // error if we encounter an i/o error.
    string input_buffer;
    try {
        input_buffer = read_file(filename);
    }
    catch (IO_error& e) {
        add_compile_error(
            format("I/O error reading from included stylesheet \"%s\": %s.",
                   included_path.c_str(),
                   e.to_string().c_str()),
            PRODUCTION_START_LINE,
            PRODUCTION_START_COLUMN);
        return;
    }

    // Finally, we can parse the included stylesheet, giving us another
    // stylesheet. Add the new parser's errors (if any) to our own list.
    Parser::Impl parser(input_buffer,
                        filename,
                        m_compiler_settings,
                        m_include_map);
    Stylesheet_ptr child = parser.parse();
    add_compile_errors(parser.compile_errors());
    if (!child)
        return;

    // A stylesheet may only have a single @charset; if the child stylesheet
    // has the same charset as our own, we'll ignore it; otherwise, error.
    if (child->charset() != parent->charset()) {
        add_compile_error(
            format("Included stylesheet \"%s\" has wrong charset (%s); "
                   "must agree with parent stylesheet charset (%s).",
                   filename.c_str(),
                   child->charset().c_str(),
                   parent->charset().c_str()),
            PRODUCTION_START_LINE,
            PRODUCTION_START_COLUMN);
    }

    // Because @import statements must all be at the beginning of a css
    // stylesheet, it is a semantic error to include stylesheets with their
    // own @import statements.
    if (child->num_imports() != 0) {
        add_compile_error(
            format("Included stylesheet \"%s\" contains @import statements.",
                   filename.c_str()),
            PRODUCTION_START_LINE,
            PRODUCTION_START_COLUMN);
    }

    // Stylesheets know how to merge themselves with another one.
    parent->merge_stylesheet(child);
}

Import_ptr Parser::Impl::parse_import()
{
    BEGIN_PRODUCTION;
    consume(TOKEN_IMPORT_SYM);
    Import_ptr import;
    if (expect_any_of(TOKEN_STRING, TOKEN_URL_SYM, 0) == TOKEN_URL_SYM)
        CREATE_AST_NODE(import, URI_import, parse_url());
    else
        CREATE_AST_NODE(import, Path_import, match(TOKEN_STRING));
    if (m_lexer.token() != TOKEN_SEMICOLON)
        parse_medium_list(import.get());
    match(TOKEN_SEMICOLON);
    return import;
}

string Parser::Impl::parse_url()
{
    BEGIN_PRODUCTION;
    consume(TOKEN_URL_SYM);
    match(TOKEN_LPAREN);
    const string url =
        expect_any_of(TOKEN_STRING, TOKEN_URI, 0) == TOKEN_URI
            ? match(TOKEN_URI)
            : dequote(match(TOKEN_STRING));
    match(TOKEN_RPAREN);
    return url;
}

void Parser::Impl::parse_medium_list(Has_media* container)
{
    BEGIN_PRODUCTION;
    container->add_medium(match(TOKEN_IDENT));
    while (m_lexer.token() == TOKEN_COMMA) {
        consume(TOKEN_COMMA);
        container->add_medium(match(TOKEN_IDENT));
    }
}

Variable_declaration_ptr Parser::Impl::parse_variable_declaration()
{
    BEGIN_PRODUCTION;
    const string name = match(TOKEN_VARIABLE);
    bool readonly = false;
    if (expect_any_of(TOKEN_READONLY, TOKEN_COLON, 0) == TOKEN_READONLY) {
        consume(TOKEN_READONLY);
        readonly = true;
    }
    match(TOKEN_COLON);
    Expr_ptr expr = parse_expr();
    match(TOKEN_SEMICOLON);
    RETURN_AST_NODE(Variable_declaration, name, expr, readonly);
}

Flavor_declaration_ptr Parser::Impl::parse_flavor_declaration()
{
    BEGIN_PRODUCTION;
    consume(TOKEN_FLAVOR_SYM);
    const string name = match(TOKEN_IDENT);
    Ruleset_ptr ruleset;
    CREATE_AST_NODE(ruleset, Ruleset);
    parse_ruleset_block(ruleset);
    RETURN_AST_NODE(Flavor_declaration, name, ruleset);
}

Media_block_ptr Parser::Impl::parse_media_block()
{
    BEGIN_PRODUCTION;
    consume(TOKEN_MEDIA_SYM);
    Media_block_ptr answer;
    CREATE_AST_NODE(answer, Media_block);
    parse_medium_list(answer.get());
    match(TOKEN_LBRACE);
    while (m_lexer.token() != TOKEN_RBRACE)
        answer->add_ruleset(parse_ruleset());
    consume(TOKEN_RBRACE);
    return answer;
}

void Parser::Impl::parse_page()
{
    assert(0);
}

Ruleset_ptr Parser::Impl::parse_ruleset()
{
    BEGIN_PRODUCTION;
    Ruleset_ptr ruleset;
    CREATE_AST_NODE(ruleset, Ruleset);
    ruleset->add_selector(parse_selector());
    while (m_lexer.token() == TOKEN_COMMA) {
        consume(TOKEN_COMMA);
        ruleset->add_selector(parse_selector());
    }
    Auto_inc_dec<int> ruleset_depth(m_ruleset_depth);
    parse_ruleset_block(ruleset);
    return ruleset;
}

void Parser::Impl::parse_ruleset_block(Ruleset_ptr ruleset)
{
    BEGIN_PRODUCTION;
    match(TOKEN_LBRACE);
    if (Ruleset_declaration_ptr p = parse_ruleset_element())
        ruleset->add_child(p);
    while (m_lexer.token() == TOKEN_SEMICOLON) {
        consume(TOKEN_SEMICOLON);
        if (Ruleset_declaration_ptr p = parse_ruleset_element())
            ruleset->add_child(p);
    }
    match(TOKEN_RBRACE);
}

Ruleset_declaration_ptr Parser::Impl::parse_ruleset_element()
{
    BEGIN_PRODUCTION;

    if (m_lexer.token() == TOKEN_ADD_FLAVOR_SYM) {
        consume(TOKEN_ADD_FLAVOR_SYM);
        RETURN_AST_NODE(Flavor, match(TOKEN_IDENT));
    }

    // Predictive parsing is not possible here; we can't tell whether we're
    // looking at a ruleset or a property until we've tried one or the other.
    // Rather than employ backtracking, we use a simple heuristic: we
    // lookahead until we encounter a '{', ';', or '}'. If we encounter the
    // first, we parse a ruleset; otherwise, a property.

    for (int i = 0; ; i++) {
        Token lookahead = m_lexer.lookahead(i);
        if (lookahead == TOKEN_SEMICOLON || lookahead == TOKEN_RBRACE)
            return parse_property();
        if (lookahead == TOKEN_LBRACE) {
            Ruleset_ptr ruleset = parse_ruleset();
            if (m_lexer.token() != TOKEN_SEMICOLON)
                m_lexer.insert(TOKEN_SEMICOLON);
            return ruleset;
        }
        if (lookahead == TOKEN_ERROR)
            return parse_ruleset(); // guaranteed parse error
    }
    assert(0); // should not get here
}

Selector_ptr Parser::Impl::parse_selector()
{
    BEGIN_PRODUCTION;

    Preserve_ws pws(m_lexer, true); // selectors are whitespace-sensitive
    skip(TOKEN_S);

    // If we are inside of a ruleset (i.e. we're parsing a nested ruleset), it
    // is legal for a selector to begin with a combinator.
    Selector::Combinator unary_combinator(Selector::DESCENDANT);
    if (m_ruleset_depth > 0) {
        if (Token token = parse_optional_combinator()) {
            unary_combinator = token_to_selector_combinator(token);
            skip(TOKEN_S);
        }
    }

    Simple_selector_sequence_ptr lhs = parse_simple_selector_sequence();
    if (Token combinator_token = parse_optional_combinator()) {
        RETURN_AST_NODE(Selector,
                        unary_combinator,
                        lhs,
                        token_to_selector_combinator(combinator_token),
                        parse_selector());
    }
    RETURN_AST_NODE(Selector, unary_combinator, lhs);
}

Token Parser::Impl::parse_optional_combinator()
{
    if (!try_any_of(TOKEN_PLUS, TOKEN_GREATER, TOKEN_TILDE, TOKEN_S, 0))
        return TOKEN_ERROR;

    Token token = m_lexer.token();
    consume(token);
    if (token == TOKEN_S) {
        skip(TOKEN_S);
        if (try_any_of(TOKEN_PLUS, TOKEN_GREATER, TOKEN_TILDE, 0)) {
            consume(token = m_lexer.token());
            skip(TOKEN_S);
            return token;
        }
        if (try_any_of(TOKEN_LBRACE, TOKEN_COMMA, 0))
            return TOKEN_ERROR;
    }
    return token;
}

Simple_selector_sequence_ptr Parser::Impl::parse_simple_selector_sequence()
{
    BEGIN_PRODUCTION;

    Token token =
        expect_any_of(TOKEN_VARIABLE,
                      TOKEN_IDENT,
                      TOKEN_STAR,
                      TOKEN_CLASS,
                      TOKEN_HASH,
                      TOKEN_LBRACKET,
                      TOKEN_COLON,
                      0);

    Type_selector_ptr type_selector;
    if (token==TOKEN_IDENT || token==TOKEN_STAR)
        CREATE_AST_NODE(type_selector, Type_selector, match(token));
    else
        CREATE_AST_NODE(type_selector, Type_selector, "*");

    Simple_selector_sequence_ptr simple_selector_sequence;
    CREATE_AST_NODE(simple_selector_sequence,
                    Simple_selector_sequence,
                    type_selector);

    while (try_any_of(TOKEN_CLASS, TOKEN_HASH, TOKEN_LBRACKET, TOKEN_COLON, 0))
        simple_selector_sequence->add_simple_selector(parse_simple_selector());

    return simple_selector_sequence;
}

Simple_selector_ptr Parser::Impl::parse_simple_selector()
{
    BEGIN_PRODUCTION;

    switch (m_lexer.token()) {
    case TOKEN_CLASS:
        return parse_class_selector();
    case TOKEN_HASH:
        return parse_id_selector();
    case TOKEN_LBRACKET:
        return parse_attribute_selector();
    case TOKEN_COLON:
        return parse_pseudo_selector();
    default:
        ECSS_PANIC(("Unexpected token %d.", m_lexer.token()));
    }
    assert(0); // not reached
}

Class_selector_ptr Parser::Impl::parse_class_selector()
{
    BEGIN_PRODUCTION;
    RETURN_AST_NODE(Class_selector, match(TOKEN_CLASS));
}

ID_selector_ptr Parser::Impl::parse_id_selector()
{
    BEGIN_PRODUCTION;
    RETURN_AST_NODE(ID_selector, match(TOKEN_HASH));
}

Attribute_selector_ptr Parser::Impl::parse_attribute_selector()
{
    BEGIN_PRODUCTION;

    Preserve_ws pws(m_lexer, false);
    consume(TOKEN_LBRACKET);
    const string name = match(TOKEN_IDENT);
    const Token op_tok = expect_any_of(TOKEN_EQUALS,
                                       TOKEN_INCLUDES,
                                       TOKEN_DASH_MATCH,
                                       TOKEN_SUBSTRING_MATCH,
                                       TOKEN_PREFIX_MATCH,
                                       TOKEN_SUFFIX_MATCH,
                                       TOKEN_RBRACKET,
                                       0);
    if (op_tok == TOKEN_RBRACKET) {
        PRESERVING_WS(consume(TOKEN_RBRACKET));
        RETURN_AST_NODE(Attribute_selector, name);
    }

    consume(op_tok);

    Attribute_selector_ptr answer;
    CREATE_AST_NODE(
        answer,
        Attribute_selector,
        name,
        token_to_attribute_selector_op(op_tok),
        match(expect_any_of(TOKEN_IDENT, TOKEN_STRING, 0)));

    PRESERVING_WS(match(TOKEN_RBRACKET));

    return answer;
}

Pseudo_selector_ptr Parser::Impl::parse_pseudo_selector()
{
    BEGIN_PRODUCTION;
    consume(TOKEN_COLON);
    string pseudo = match(TOKEN_IDENT);
    if (m_lexer.token() == TOKEN_LPAREN) {
        pseudo += match(TOKEN_LPAREN);
        if (expect_any_of(TOKEN_IDENT, TOKEN_RPAREN, 0) == TOKEN_IDENT)
            pseudo += match(TOKEN_IDENT);
        pseudo += match(TOKEN_RPAREN);
    }
    RETURN_AST_NODE(Pseudo_selector, pseudo);
}

Property_ptr Parser::Impl::parse_property()
{
    BEGIN_PRODUCTION;
    if (m_lexer.token() != TOKEN_IDENT)
        return Property_ptr(); // empty property
    string property = match(TOKEN_IDENT);
    User_agent user_agent;
    if (expect_any_of(TOKEN_COLON, TOKEN_LPAREN, 0) == TOKEN_LPAREN) {
        match(TOKEN_LPAREN);
        user_agent = parse_property_user_agent();
        match(TOKEN_RPAREN);
    }
    match(TOKEN_COLON);
    Expr_ptr expr = parse_expr();
    bool important = false;
    if (m_lexer.token() == TOKEN_IMPORTANT_SYM) {
        consume(TOKEN_IMPORTANT_SYM);
        important = true;
    }
    RETURN_AST_NODE(Property, property, user_agent, expr, important);
}

User_agent Parser::Impl::parse_property_user_agent()
{
    string name = match(TOKEN_IDENT);
    string version;
    if (m_lexer.token() == TOKEN_NUMBER)
        version = match(TOKEN_NUMBER);
    return User_agent(name, version);
}

Expr_ptr Parser::Impl::parse_expr()
{
    BEGIN_PRODUCTION;
    Expr_ptr expr = parse_unary_expr();
    while (try_any_expr_token())
        expr = parse_binary_expr(expr);
    return expr;
}

Expr_ptr Parser::Impl::parse_binary_expr(Expr_ptr lhs)
{
    BEGIN_PRODUCTION;
    assert(try_any_expr_token());
    Binary_expr::Operator op = Binary_expr::WHITESPACE;
    if (try_any_of(TOKEN_COMMA,
                   TOKEN_EQUALS,
                   TOKEN_PLUS,
                   TOKEN_MINUS,
                   TOKEN_STAR,
                   TOKEN_SLASH,
                   TOKEN_CONCAT,
                   0))
    {
        op = token_to_binary_expr_op(m_lexer.token());
        consume(m_lexer.token());
    }
    RETURN_AST_NODE(Binary_expr, lhs, op, parse_unary_expr());
}

Expr_ptr Parser::Impl::parse_unary_expr()
{
    BEGIN_PRODUCTION;
    if (try_any_of(TOKEN_MINUS, TOKEN_PLUS, 0)) {
        Unary_expr::Operator op = token_to_unary_expr_op(m_lexer.token());
        consume(m_lexer.token());
        const Token token =
            expect_any_of(TOKEN_NUMBER,
                          TOKEN_PERCENTAGE,
                          TOKEN_DIMENSION,
                          TOKEN_VARIABLE,
                          0);
        Term_ptr term;
        if (token == TOKEN_NUMBER)
            CREATE_AST_NODE(term, Number_term, match(token));
        if (token == TOKEN_PERCENTAGE)
            CREATE_AST_NODE(term, Percentage_term, match(token));
        if (token == TOKEN_DIMENSION)
            CREATE_AST_NODE(term, Dimension_term, match(token));
        if (token == TOKEN_VARIABLE)
            CREATE_AST_NODE(term, Variable_term, match(token));
        assert(term != 0);
        RETURN_AST_NODE(Unary_expr, term, op);
    }

    const Token token =
        expect_any_of(TOKEN_NUMBER,
                      TOKEN_PERCENTAGE,
                      TOKEN_DIMENSION,
                      TOKEN_STRING,
                      TOKEN_IDENT,
                      TOKEN_URL_SYM,
                      TOKEN_HASH,
                      TOKEN_VARIABLE,
                      TOKEN_LPAREN,
                      0);

    if (token == TOKEN_NUMBER)
        RETURN_AST_NODE(Number_term, match(token));
    if (token == TOKEN_PERCENTAGE)
        RETURN_AST_NODE(Percentage_term, match(token));
    if (token == TOKEN_DIMENSION)
        RETURN_AST_NODE(Dimension_term, match(token));
    if (token == TOKEN_STRING)
        RETURN_AST_NODE(String_term, match(token));
    if (token == TOKEN_URL_SYM)
        RETURN_AST_NODE(URL_term, parse_url());
    if (token == TOKEN_HASH)
        RETURN_AST_NODE(Hash_term, match(token));
    if (token == TOKEN_VARIABLE)
        RETURN_AST_NODE(Variable_term, match(token));
    if (token == TOKEN_IDENT) {
        if (m_lexer.lookahead(1) == TOKEN_LPAREN)
            return parse_function_term();
        RETURN_AST_NODE(Identifier_term, match(token));
    }
    if (token == TOKEN_LPAREN) {
        match(TOKEN_LPAREN);
        Expr_ptr expr = parse_expr();
        match(TOKEN_RPAREN);
        return expr;
    }

    ECSS_PANIC(("Internal error."));
    assert(0);
}

Function_term_ptr Parser::Impl::parse_function_term()
{
    BEGIN_PRODUCTION;
    const string function_name = match(TOKEN_IDENT);
    match(TOKEN_LPAREN);
    Expr_ptr expr = parse_expr();
    match(TOKEN_RPAREN);
    RETURN_AST_NODE(Function_term, function_name, expr);
}

string Parser::Impl::resolve_included_path(string original_path)
{
    if (is_relative_path(original_path)) {
        // Try to find the original_path in each include_dir.
        const vector<string>& include_dirs =
            m_compiler_settings->include_dirs();
        for (int i = 0, n = include_dirs.size(); i < n; i++) {
            const string path =
                make_full_path(include_dirs[i] + '/' + original_path);
            if (!path.empty() && is_readable_file(path))
                return path;
        }

        // None of the include dirs matched; try the including file's dir.
        const string path =
            make_full_path(dir_name(m_filename) + '/' + original_path);
        if (!path.empty() && is_readable_file(path))
            return path;

        return "";  // failure
    }
    return make_full_path(original_path);
}

bool Parser::Impl::try_any_expr_token()
{
    return try_any_of(TOKEN_COMMA,
                      TOKEN_EQUALS,
                      TOKEN_PLUS,
                      TOKEN_MINUS,
                      TOKEN_STAR,
                      TOKEN_SLASH,
                      TOKEN_CONCAT,
                      TOKEN_NUMBER,
                      TOKEN_PERCENTAGE,
                      TOKEN_DIMENSION,
                      TOKEN_STRING,
                      TOKEN_IDENT,
                      TOKEN_URL_SYM,
                      TOKEN_HASH,
                      TOKEN_VARIABLE,
                      TOKEN_LPAREN,
                      0);
}

string Parser::Impl::match(Token expected)
{
    if (m_lexer.token() != expected)
        parse_failure(expected);
    string answer = m_lexer.lexeme();
    m_lexer.advance();
    return answer;
}

#define BUILD_TOKEN_VECTOR(ts)                                  \
    vector<Token> ts;                                           \
    ts.push_back(first);                                        \
    do {                                                        \
        va_list ap;                                             \
        va_start(ap, first);                                    \
        while (Token t = static_cast<Token>(va_arg(ap, int)))   \
            ts.push_back(t);                                    \
        va_end(ap);                                             \
    } while(0)

Token Parser::Impl::expect_any_of(Token first, ...)
{
    BUILD_TOKEN_VECTOR(tokens);
    if (!try_any_of_impl(tokens))
        parse_failure(tokens);
    return m_lexer.token();
}

bool Parser::Impl::try_any_of(Token first, ...)
{
    BUILD_TOKEN_VECTOR(tokens);
    return try_any_of_impl(tokens);
}

bool Parser::Impl::try_any_of_impl(const vector<Token>& ts)
{
    for (int i = 0; i < int(ts.size()); i++)
        if (find(ts.begin(), ts.end(), m_lexer.token()) != ts.end())
            return true;
    return false;
}

void Parser::Impl::skip(Token expected)
{
    while (m_lexer.token() == expected)
        m_lexer.advance();
}

void Parser::Impl::skip_any_of(Token first, ...)
{
    BUILD_TOKEN_VECTOR(ts);
    while (find(ts.begin(), ts.end(), m_lexer.token()) != ts.end())
        m_lexer.advance();
}

void Parser::Impl::consume(Token expected)
{
    if (m_lexer.token() != expected) {
        ECSS_PANIC(("Expected token type \"%s\"; got \"%s\".",
                    token_to_string(expected),
                    token_to_string(m_lexer.token())));
    }
    m_lexer.advance();
}

void Parser::Impl::parse_failure(vector<Token> expected_tokens)
{
    string msg = "Encountered " + format_curr_token_for_error_msg(m_lexer)
        + " when expecting one of the following:";

    for (int i = 0, n = expected_tokens.size(); i < n; ++i) {
        msg += ' ';
        msg += token_to_string(expected_tokens[i]);
    }
    add_compile_error(msg, m_lexer.line(), m_lexer.column());
    throw Parse_failure_exception();
}

void Parser::Impl::parse_failure(Token expected_token)
{
    vector<Token> expected_tokens;
    expected_tokens.push_back(expected_token);
    parse_failure(expected_tokens);
}

void Parser::Impl::add_compile_error(string msg, int line, int column)
{
    m_errors.push_back(Compile_error(Location(m_filename, line, column), msg));
}

void Parser::Impl::add_compile_errors(vector<Compile_error> errors)
{
    m_errors.insert(m_errors.end(), errors.begin(), errors.end());
}

// +--------+
// | Parser |
// +--------+

Parser::Parser(const string& input_buffer,
               const string& filename,
               Compiler_settings_ptr cs)
: m_impl(new Impl(input_buffer,
                  filename,
                  cs,
                  Include_map_ptr(new Include_map)))
{}

Parser::~Parser()
{
    delete m_impl;
}

Stylesheet_ptr Parser::parse()
{
    return m_impl->parse();
}

const vector<Compile_error>& Parser::compile_errors() const
{
    return m_impl->compile_errors();
}
