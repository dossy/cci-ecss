#include "ecss/ast.h"

using namespace std;
using namespace ecss_core;

//
// AST_node
//

AST_node::~AST_node() {}

void AST_node::annotate(const string& filename, int line, int column)
{
    m_location.set(filename, line, column);
}

//
// Has_media
//

Has_media::~Has_media() {}

//
// Attribute_selector
//

Attribute_selector::Attribute_selector(const string& name)
: m_name(name)
, m_op(NONE)
{}

Attribute_selector::Attribute_selector(const string& name,
                                       Operator op,
                                       const string& value)
: m_name(name)
, m_op(op)
, m_value(value)
{}

//
// Simple_selector_sequence
//

Simple_selector_sequence::Simple_selector_sequence(Type_selector_ptr s)
{
    assert(s);
    add_simple_selector(s);
}

void Simple_selector_sequence::add_simple_selector(Simple_selector_ptr s)
{
    assert(s);
    m_simple_selectors.push_back(s);
}

int Simple_selector_sequence::num_simple_selectors() const
{
    return m_simple_selectors.size();
}

Simple_selector_ptr Simple_selector_sequence::simple_selector(int n) const
{
    return m_simple_selectors.at(n);
}

//
// Selector
//

Selector::Selector(Combinator uc, Simple_selector_sequence_ptr lhs)
: m_unary_combinator(uc)
, m_lhs(lhs)
, m_binary_combinator(DESCENDANT)
{
    assert(m_lhs);
}

Selector::Selector(Combinator uc,
                   Simple_selector_sequence_ptr lhs,
                   Combinator bc,
                   Selector_ptr rhs)
: m_unary_combinator(uc)
, m_lhs(lhs)
, m_binary_combinator(bc)
, m_rhs(rhs)
{
    assert(m_lhs);
    assert(m_rhs);
}

//
// Function_term
//

Function_term::Function_term(string function_name, Expr_ptr expr)
: m_function_name(function_name)
, m_expr(expr)
{}

//
// Unary_expr
//

Unary_expr::Unary_expr(Term_ptr term, Operator op)
: m_term(term)
, m_op(op)
{
    assert(m_term);
}

//
// Binary_expr
//

Binary_expr::Binary_expr(Expr_ptr lhs, Operator op, Expr_ptr rhs)
: m_lhs(lhs)
, m_op(op)
, m_rhs(rhs)
{
    assert(lhs);
    assert(rhs);
}

//
// Property
//

Property::Property(string name,
                   User_agent user_agent,
                   Expr_ptr expr,
                   bool important)
: m_name(name)
, m_user_agent(user_agent)
, m_expr(expr)
, m_important(important)
{
    assert(expr);
}

//
// Ruleset
//

void Ruleset::add_selector(Selector_ptr s)
{
    assert(s);
    m_selectors.push_back(s);
}

void Ruleset::add_child(Ruleset_declaration_ptr p)
{
    assert(p);
    m_children.push_back(p);
}

int Ruleset::num_selectors() const
{
    return m_selectors.size();
}

Selector_ptr Ruleset::selector(int n) const
{
    return m_selectors.at(n);
}

int Ruleset::num_children() const
{
    return m_children.size();
}

Ruleset_declaration_ptr Ruleset::child(int n) const
{
    return m_children.at(n);
}

//
// Variable_declaration
//

Variable_declaration::Variable_declaration(const string& name,
                                           Expr_ptr expr,
                                           bool readonly)
: m_name(name)
, m_expr(expr)
, m_readonly(readonly)
{
    assert(m_expr);
}

//
// Flavor_declaration
//

Flavor_declaration::Flavor_declaration(const string& name, Ruleset_ptr ruleset)
: m_name(name)
, m_ruleset(ruleset)
{}

//
// Media_block
//

void Media_block::add_ruleset(Ruleset_ptr r)
{
    assert(r);
    m_rulesets.push_back(r);
}

//
// Stylesheet
//

void Stylesheet::merge_stylesheet(Stylesheet_ptr s)
{
    for (int i = 0; i < s->num_imports(); i++) {
        add_import(s->import(i));
    }
    for (int i = 0; i < s->num_declarations(); i++) {
        add_declaration(s->declaration(i));
    }
}

void Stylesheet::set_charset(const string& s)
{
    m_charset = s;
}

void Stylesheet::add_import(Import_ptr import)
{
    assert(import);
    m_imports.push_back(import);
}

void Stylesheet::add_variable_declaration(Variable_declaration_ptr d)
{
    add_declaration(d);
}

void Stylesheet::add_flavor_declaration(Flavor_declaration_ptr d)
{
    add_declaration(d);
}

void Stylesheet::add_media_block(Media_block_ptr d)
{
    add_declaration(d);
}

void Stylesheet::add_ruleset(Ruleset_ptr d)
{
    add_declaration(d);
}

void Stylesheet::add_declaration(Declaration_ptr d)
{
    assert(d);
    m_declarations.push_back(d);
}

const string& Stylesheet::charset() const
{
    return m_charset;
}

int Stylesheet::num_imports() const
{
    return m_imports.size();
}

Import_ptr Stylesheet::import(int n) const
{
    return m_imports.at(n);
}

int Stylesheet::num_declarations() const
{
    return m_declarations.size();
}

Declaration_ptr Stylesheet::declaration(int n) const
{
    return m_declarations.at(n);
}

//
// AST_node::accept member function definitions (keep these alphabetized)
//

void Attribute_selector::accept       (AST_visitor& v) { v.visit(this); }
void Binary_expr::accept              (AST_visitor& v) { v.visit(this); }
void Class_selector::accept           (AST_visitor& v) { v.visit(this); }
void Dimension_term::accept           (AST_visitor& v) { v.visit(this); }
void Flavor::accept                   (AST_visitor& v) { v.visit(this); }
void Flavor_declaration::accept       (AST_visitor& v) { v.visit(this); }
void Function_term::accept            (AST_visitor& v) { v.visit(this); }
void Hash_term::accept                (AST_visitor& v) { v.visit(this); }
void ID_selector::accept              (AST_visitor& v) { v.visit(this); }
void Identifier_term::accept          (AST_visitor& v) { v.visit(this); }
void Media_block::accept              (AST_visitor& v) { v.visit(this); }
void Number_term::accept              (AST_visitor& v) { v.visit(this); }
void Path_import::accept              (AST_visitor& v) { v.visit(this); }
void Percentage_term::accept          (AST_visitor& v) { v.visit(this); }
void Property::accept                 (AST_visitor& v) { v.visit(this); }
void Pseudo_selector::accept          (AST_visitor& v) { v.visit(this); }
void Ruleset::accept                  (AST_visitor& v) { v.visit(this); }
void Selector::accept                 (AST_visitor& v) { v.visit(this); }
void Simple_selector_sequence::accept (AST_visitor& v) { v.visit(this); }
void String_term::accept              (AST_visitor& v) { v.visit(this); }
void Stylesheet::accept               (AST_visitor& v) { v.visit(this); }
void Type_selector::accept            (AST_visitor& v) { v.visit(this); }
void URI_import::accept               (AST_visitor& v) { v.visit(this); }
void URL_term::accept                 (AST_visitor& v) { v.visit(this); }
void Unary_expr::accept               (AST_visitor& v) { v.visit(this); }
void Variable_declaration::accept     (AST_visitor& v) { v.visit(this); }
void Variable_term::accept            (AST_visitor& v) { v.visit(this); }
