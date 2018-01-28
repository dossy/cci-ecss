#ifndef included_ecss_ast
#define included_ecss_ast

#include "ecss/lexer.h"
#include "ecss/user_agent.h"
#include "ecss/utility.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

namespace ecss_core {

class AST_visitor;

#define declare_ast_node(class_name)                            \
    class class_name;                                           \
    typedef boost::shared_ptr<class_name> class_name##_ptr

declare_ast_node(AST_node);
declare_ast_node(Attribute_selector);
declare_ast_node(Binary_expr);
declare_ast_node(Class_selector);
declare_ast_node(Declaration);
declare_ast_node(Dimension_term);
declare_ast_node(Expr);
declare_ast_node(Flavor);
declare_ast_node(Flavor_declaration);
declare_ast_node(Function_term);
declare_ast_node(Hash_term);
declare_ast_node(ID_selector);
declare_ast_node(Identifier_term);
declare_ast_node(Import);
declare_ast_node(Media_block);
declare_ast_node(Number_term);
declare_ast_node(Path_import);
declare_ast_node(Percentage_term);
declare_ast_node(Property);
declare_ast_node(Pseudo_selector);
declare_ast_node(Ruleset);
declare_ast_node(Ruleset_declaration);
declare_ast_node(Selector);
declare_ast_node(Simple_selector);
declare_ast_node(Simple_selector_sequence);
declare_ast_node(String_term);
declare_ast_node(Stylesheet);
declare_ast_node(Term);
declare_ast_node(Type_selector);
declare_ast_node(URI_import);
declare_ast_node(URL_term);
declare_ast_node(Unary_expr);
declare_ast_node(Value_term);
declare_ast_node(Variable_declaration);
declare_ast_node(Variable_term);

#undef declare_ast_node

class AST_node {
public:
    virtual ~AST_node();
    void annotate(const std::string& filename, int line, int column);
    const Location& location() const { return m_location; }
    virtual void accept(AST_visitor&) = 0;
private:
    Location m_location;
};

class Has_media {
public:
    virtual ~Has_media() = 0;
    void add_medium(const std::string& medium) { m_media.push_back(medium); }
    int num_media() const { return m_media.size(); }
    const std::string& medium(int n) const { return m_media[n]; }
private:
    std::vector<std::string> m_media;
};

class Simple_selector : public AST_node {};

class Type_selector : public Simple_selector {
public:
    Type_selector(const std::string& type) : m_type(type) {}
    virtual ~Type_selector() {}
    const std::string type() const { return m_type; }
    void accept(AST_visitor& v);
private:
    const std::string m_type;
};

class Attribute_selector : public Simple_selector {
public:
    enum Operator {
        NONE, EQUALS, INCLUDES, DASH_MATCH,
        SUBSTRING_MATCH, PREFIX_MATCH, SUFFIX_MATCH
    };
    Attribute_selector(const std::string& name);
    Attribute_selector(const std::string& name,
                       Operator op,
                       const std::string& value);
    virtual ~Attribute_selector() {}
    const std::string& name() const { return m_name; }
    Operator op() const { return m_op; }
    const std::string& value() const { return m_value; }
    void accept(AST_visitor& v);
private:
    const std::string m_name;
    const Operator m_op;
    const std::string m_value;
};

class Class_selector : public Attribute_selector {
public:
    Class_selector(const std::string& class_name)
    : Attribute_selector("class", Attribute_selector::INCLUDES, class_name) {}
    void accept(AST_visitor& v);
};

class ID_selector : public Simple_selector {
public:
    ID_selector(const std::string& id) : m_id(id) {}
    const std::string& id() const { return m_id; }
    void accept(AST_visitor& v);
private:
    const std::string m_id;
};

// FIXME: pseudo selectors need a lot of work
class Pseudo_selector : public Simple_selector {
public:
    Pseudo_selector(const std::string s) : m_s(s) {}
    const std::string& s() const { return m_s; }
    void accept(AST_visitor& v);
private:
    const std::string m_s;
};

class Simple_selector_sequence : public AST_node {
public:
    Simple_selector_sequence(Type_selector_ptr s);
    void add_simple_selector(Simple_selector_ptr s);
    int num_simple_selectors() const;
    Simple_selector_ptr simple_selector(int n) const;
    void accept(AST_visitor& v);
private:
    std::vector<Simple_selector_ptr> m_simple_selectors;
};

class Selector : public AST_node {
public:
    enum Combinator { DESCENDANT, CHILD, ADJACENT_SIBLING, GENERAL_SIBLING };
    Selector(Combinator uc, Simple_selector_sequence_ptr lhs);
    Selector(Combinator uc, Simple_selector_sequence_ptr lhs,
             Combinator bc, Selector_ptr rhs);
    Combinator unary_combinator() const { return m_unary_combinator; }
    Simple_selector_sequence_ptr lhs() const { return m_lhs; }
    Combinator binary_combinator() const { return m_binary_combinator; }
    Selector_ptr rhs() const { return m_rhs; }
    void accept(AST_visitor& v);
private:
    Combinator m_unary_combinator;
    Simple_selector_sequence_ptr m_lhs;
    Combinator m_binary_combinator;
    Selector_ptr m_rhs;
};

class Expr : public AST_node {};

class Term : public Expr {};

class Value_term : public Term {
public:
    Value_term(std::string value) : m_value(value) {}
    const std::string value() const { return m_value; }
private:
    std::string m_value;
};

class Dimension_term : public Value_term {
public:
    Dimension_term(std::string value) : Value_term(value) {}
    void accept(AST_visitor& v);
};

class Function_term : public Term {
public:
    Function_term(std::string function_name, Expr_ptr expr);
    const std::string& function_name() const { return m_function_name; }
    Expr_ptr expr() const { return m_expr; }
    void accept(AST_visitor& v);
private:
    const std::string m_function_name;
    Expr_ptr m_expr;
};

class Hash_term : public Value_term {
public:
    Hash_term(std::string value) : Value_term(value) {}
    void accept(AST_visitor& v);
};

class Identifier_term : public Value_term {
public:
    Identifier_term(std::string value) : Value_term(value) {}
    void accept(AST_visitor& v);
};

class Number_term : public Value_term {
public:
    Number_term(std::string value) : Value_term(value) {}
    void accept(AST_visitor& v);
};

class Percentage_term : public Value_term {
public:
    Percentage_term(std::string value) : Value_term(value) {}
    void accept(AST_visitor& v);
};

class String_term : public Value_term {
public:
    String_term(std::string value) : Value_term(value) {}
    void accept(AST_visitor& v);
};

class URL_term : public Value_term {
public:
    URL_term(std::string value) : Value_term(value) {}
    void accept(AST_visitor& v);
};

class Variable_term : public Term {
public:
    Variable_term(std::string name) : m_name(name) {}
    const std::string& name() const { return m_name; }
    void accept(AST_visitor& v);
private:
    const std::string m_name;
};

class Unary_expr : public Expr {
public:
    enum Operator { MINUS, PLUS };
    Unary_expr(Term_ptr term, Operator op);
    Term_ptr term() const { return m_term; }
    Operator op() const { return m_op; }
    void accept(AST_visitor& v);
private:
    Term_ptr m_term;
    Operator m_op;
};

class Binary_expr : public Expr {
public:
    enum Operator {
        WHITESPACE,
        COMMA,
        EQUALS,
        PLUS,
        MINUS,
        STAR,
        SLASH,
        CONCAT,
    };
    Binary_expr(Expr_ptr lhs, Operator op, Expr_ptr rhs);
    Expr_ptr lhs() const { return m_lhs; }
    Operator op() const { return m_op; }
    Expr_ptr rhs() const { return m_rhs; }
    void accept(AST_visitor& v);
private:
    Expr_ptr m_lhs;
    Operator m_op;
    Expr_ptr m_rhs;
};

class Declaration : public AST_node {};

class Ruleset_declaration : public Declaration {};

class Property : public Ruleset_declaration {
public:
    Property(std::string name,
             User_agent user_agent,
             Expr_ptr expr,
             bool important);
    const std::string& name() const { return m_name; }
    const User_agent& user_agent() const { return m_user_agent; }
    Expr_ptr expr() const { return m_expr; }
    bool important() const { return m_important; }
    void accept(AST_visitor& v);
private:
    std::string m_name;
    User_agent m_user_agent;
    Expr_ptr m_expr;
    bool m_important;
};

class Ruleset : public Ruleset_declaration {
public:
    void add_selector(Selector_ptr s);
    void add_child(Ruleset_declaration_ptr p);
    int num_selectors() const;
    Selector_ptr selector(int n) const;
    int num_children() const;
    Ruleset_declaration_ptr child(int n) const;
    void accept(AST_visitor& v);
private:
    std::vector<Selector_ptr> m_selectors;
    std::vector<Ruleset_declaration_ptr> m_children;
};

class Variable_declaration : public Declaration {
public:
    Variable_declaration(const std::string& name, Expr_ptr expr, bool readonly);
    const std::string& name() const { return m_name; }
    Expr_ptr expr() const { return m_expr; }
    bool readonly() const { return m_readonly; }
    void accept(AST_visitor& v);
private:
    const std::string m_name;
    Expr_ptr m_expr;
    const bool m_readonly;
};

class Flavor_declaration : public Declaration {
public:
    Flavor_declaration(const std::string& name, Ruleset_ptr ruleset);
    const std::string& name() const { return m_name; }
    Ruleset_ptr ruleset() { return m_ruleset; }
    void accept(AST_visitor& v);
private:
    const std::string m_name;
    Ruleset_ptr m_ruleset;
};

class Flavor : public Ruleset_declaration {
public:
    Flavor(const std::string& name) : m_name(name) {}
    const std::string& name() const { return m_name; }
    void accept(AST_visitor& v);
private:
    const std::string m_name;
};

class Media_block : public Declaration, public Has_media {
public:
    void add_ruleset(Ruleset_ptr r);
    int num_rulesets() const { return m_rulesets.size(); }
    Ruleset_ptr ruleset(int n) const { return m_rulesets[n]; }
    void accept(AST_visitor& v);
private:
    std::vector<Ruleset_ptr> m_rulesets;
};

class Import : public AST_node, public Has_media {
public:
    Import(std::string target) : m_target(target) {}
    const std::string& target() const { return m_target; }
private:
    std::string m_target;
};

class Path_import : public Import {
public:
    Path_import(std::string target) : Import(target) {}
    void accept(AST_visitor& v);
};

class URI_import : public Import {
public:
    URI_import(std::string target) : Import(target) {}
    void accept(AST_visitor& v);
};

class Stylesheet : public AST_node {
public:
    void merge_stylesheet(Stylesheet_ptr);
    void set_charset(const std::string& s);
    void add_import(Import_ptr);
    void add_variable_declaration(Variable_declaration_ptr);
    void add_flavor_declaration(Flavor_declaration_ptr);
    void add_media_block(Media_block_ptr);
    void add_ruleset(Ruleset_ptr);
    const std::string& charset() const;
    int num_imports() const;
    Import_ptr import(int n) const;
    int num_declarations() const;
    Declaration_ptr declaration(int n) const;
    void accept(AST_visitor& v);

private:
    void add_declaration(Declaration_ptr d);
    std::string m_charset;
    std::vector<Import_ptr> m_imports;
    std::vector<Declaration_ptr> m_declarations;
};

class AST_visitor {
public:
    virtual ~AST_visitor() {}
    virtual void visit(Stylesheet*) = 0;
    virtual void visit(Path_import*) = 0;
    virtual void visit(URI_import*) = 0;
    virtual void visit(Variable_declaration*) = 0;
    virtual void visit(Flavor*) = 0;
    virtual void visit(Flavor_declaration*) = 0;
    virtual void visit(Media_block*) = 0;
    virtual void visit(Ruleset*) = 0;
    virtual void visit(Selector*) = 0;
    virtual void visit(Simple_selector_sequence*) = 0;
    virtual void visit(Type_selector*) = 0;
    virtual void visit(Attribute_selector*) = 0;
    virtual void visit(Class_selector*) = 0;
    virtual void visit(ID_selector*) = 0;
    virtual void visit(Pseudo_selector*) = 0;
    virtual void visit(Property*) = 0;
    virtual void visit(Unary_expr*) = 0;
    virtual void visit(Binary_expr*) = 0;
    virtual void visit(Dimension_term*) = 0;
    virtual void visit(Function_term*) = 0;
    virtual void visit(Hash_term*) = 0;
    virtual void visit(Identifier_term*) = 0;
    virtual void visit(Number_term*) = 0;
    virtual void visit(Percentage_term*) = 0;
    virtual void visit(String_term*) = 0;
    virtual void visit(URL_term*) = 0;
    virtual void visit(Variable_term*) = 0;
};

} // namespace ecss_core

#endif
