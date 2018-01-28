#include "ecss/translator.h"
#include "ecss/evaluator.h"
#include "ecss/optimizer.h"
#include <algorithm>
#include <memory>
#include <set>

using namespace std;
using namespace ecss_core;

namespace
{
    const char* selector_combinator_to_string(Selector::Combinator c)
    {
        switch (c) {
        case Selector::DESCENDANT:       return "";
        case Selector::CHILD:            return "> ";
        case Selector::ADJACENT_SIBLING: return "+ ";
        case Selector::GENERAL_SIBLING:  return "~ ";
        }
        ECSS_PANIC(("Invalid selector combinator: %d", c));
        assert(0); // not reached
    }

    const char* attr_selector_op_to_string(Attribute_selector::Operator op)
    {
        switch (op) {
        case Attribute_selector::NONE:            return "";
        case Attribute_selector::EQUALS:          return "=";
        case Attribute_selector::INCLUDES:        return "~=";
        case Attribute_selector::DASH_MATCH:      return "|=";
        case Attribute_selector::SUBSTRING_MATCH: return "*=";
        case Attribute_selector::PREFIX_MATCH:    return "^=";
        case Attribute_selector::SUFFIX_MATCH:    return "$=";
        }
        ECSS_PANIC(("Invalid attribute selector operator: %d", op));
        assert(0); // not reached
    }

    const char* binary_expr_op_to_string(Binary_expr::Operator op)
    {
        switch (op) {
        case Binary_expr::WHITESPACE: return " ";
        case Binary_expr::COMMA:      return ", ";
        case Binary_expr::EQUALS:     return " =";
        case Binary_expr::PLUS:       return " +";
        case Binary_expr::MINUS:      return " -";
        case Binary_expr::STAR:       return " *";
        case Binary_expr::SLASH:      return " /";
        case Binary_expr::CONCAT:     return " _";
        }
        ECSS_PANIC(("Invalid binary expression operator: %d", op));
        assert(0); // not reached
    }
}

// +----------------+
// | AST_translator |
// +----------------+

AST_translator::AST_translator(Compiler_settings_ptr cs)
: m_compiler_settings(cs)
{}

const vector<Compile_error>& AST_translator::compile_errors() const
{
    return m_errors;
}

string AST_translator::translate(AST_node* node)
{
    assert(node);

    m_emit_target_stack.clear();
    m_simple_selector_sequence_stack.clear();
    m_symbol_table.clear();
    m_flavor_symbol_table.clear();
    m_flavor_stack.clear();
    m_selector_array_stack.clear();
    m_property_set_stack.clear();
    m_css_rulesets.clear();
    m_css_media_blocks.clear();
    m_errors.clear();

    push_emit_target();
    node->accept(*this);
    string output = pop_emit_target();

    optimize_css_rulesets(m_css_rulesets, m_compiler_settings);

    for (int i = 0, n = m_css_imports.size(); i < n; i++) {
        output += m_css_imports[i];
        output += '\n';
    }
    for (int i = 0, n = m_css_rulesets.size(); i < n; i++) {
        output += m_css_rulesets[i]->to_string();
        output += '\n';
    }
    for (int i = 0, n = m_css_media_blocks.size(); i < n; i++) {
        output += m_css_media_blocks[i]->to_string();
        output += '\n';
    }

    return output;
}

void AST_translator::visit(Stylesheet* node)
{
    if (!node->charset().empty()) {
        emit("@charset ");
        emit(node->charset());
        emit(";\n");
    }
    for (int i = 0; i < node->num_imports(); i++) {
        node->import(i)->accept(*this);
    }
    for (int i = 0; i < node->num_declarations(); i++) {
        node->declaration(i)->accept(*this);
    }
}

void AST_translator::visit(Path_import* node)
{
    push_emit_target();
    emit("@import ");
    emit(node->target());
    emit(";");
    m_css_imports.push_back(pop_emit_target());
}

void AST_translator::visit(URI_import* node)
{
    push_emit_target();
    emit("@import url(");
    emit(node->target());
    emit(");");
    m_css_imports.push_back(pop_emit_target());
}

void AST_translator::visit(Variable_declaration* node)
{
    push_emit_target();
    node->expr()->accept(*this);
    const string expr = pop_emit_target();
    Var_info info(expr, node->location(), node->readonly());
    set_variable(node->name(), info);
}

void AST_translator::visit(Flavor_declaration* node)
{
    Flavor_symbol_table::iterator i = m_flavor_symbol_table.find(node->name());
    if (i != m_flavor_symbol_table.end()) {
        add_compile_error(
            node->location(),
            "theme \"" + node->name() + "\" already defined at "
            + i->second->location().to_string());
    }
    m_flavor_symbol_table.insert(make_pair(node->name(), node));
}

void AST_translator::visit(Flavor* node)
{
    Flavor_symbol_table::iterator i = m_flavor_symbol_table.find(node->name());
    if (i == m_flavor_symbol_table.end()) {
        add_compile_error(node->location(),
                          "theme \"" + node->name() + "\" not defined");
        return;
    }
    for (int j = 0; j < int(m_flavor_stack.size()); j++) {
        if (m_flavor_stack[j] == node->name()) {
            add_compile_error(node->location(), "circular theme detected");
            return;
        }
    }
    m_flavor_stack.push_back(node->name());
    i->second->ruleset()->accept(*this);
    m_flavor_stack.pop_back();
}

void AST_translator::visit(Media_block* node)
{
    assert(!m_inside_css_media_block);
    m_inside_css_media_block.reset(new CSS_media_block);

    for (int i = 0; i < node->num_media(); i++) {
        m_inside_css_media_block->add_medium(node->medium(i));
    }

    for (int i = 0; i < node->num_rulesets(); i++) {
        node->ruleset(i)->accept(*this);
    }

    assert(m_inside_css_media_block);
    m_css_media_blocks.push_back(m_inside_css_media_block);
    m_inside_css_media_block.reset();
}

void AST_translator::visit(Ruleset* node)
{
    if (node->num_selectors() != 0) {
        enter_ruleset_scope();
    }

    for (int i = 0, n = node->num_selectors(); i < n; i++) {
        node->selector(i)->accept(*this);
    }
    for (int i = 0, n = node->num_children(); i < n; i++) {
        node->child(i)->accept(*this);
    }

    if (node->num_selectors() != 0) {
        leave_ruleset_scope();
    }
}

void AST_translator::visit(Selector* node)
{
    push_emit_target();
    if (node->unary_combinator() != Selector::DESCENDANT) {
        emit(selector_combinator_to_string(node->unary_combinator()));
    }
    node->lhs()->accept(*this);
    if (node->rhs()) {
        emit(' ');
        emit(selector_combinator_to_string(node->binary_combinator()));
        // KLUDGE: Because selectors are defined recursively, our rhs will
        // push its own emit-target, which we need to pop and emit.
        node->rhs()->accept(*this);
        emit(m_selector_array_stack.back().back());
        m_selector_array_stack.back().pop_back();
    }
    m_selector_array_stack.back().push_back(pop_emit_target());
}

void AST_translator::visit(Simple_selector_sequence* node)
{
    const int num_simple_selectors = node->num_simple_selectors();
    m_simple_selector_sequence_stack.push_back(num_simple_selectors);
    for (int i = 0; i < num_simple_selectors; i++) {
        node->simple_selector(i)->accept(*this);
    }
    m_simple_selector_sequence_stack.pop_back();
}

void AST_translator::visit(Type_selector* node)
{
    if (node->type() != "*" || m_simple_selector_sequence_stack.back() == 1) {
        emit(node->type());
    }
}

void AST_translator::visit(Attribute_selector* node)
{
    emit('[');
    emit(node->name());
    if (node->op() != Attribute_selector::NONE) {
        emit(attr_selector_op_to_string(node->op()));
        emit(node->value());
    }
    emit(']');
}

void AST_translator::visit(Class_selector* node)
{
    emit(node->value());
}

void AST_translator::visit(ID_selector* node)
{
    emit(node->id());
}

void AST_translator::visit(Pseudo_selector* node)
{
    emit(':');
    emit(node->s());
}

void AST_translator::visit(Property* node)
{
    push_emit_target();
    emit(node->name());
    const string name = pop_emit_target();
    push_emit_target();
    node->expr()->accept(*this);
    const string expr = pop_emit_target();

    // Add the property only if it applies to the current user agent.
    if (m_compiler_settings->user_agent().match(node->user_agent())) {
        m_property_set_stack.back().add_property(
            CSS_property(name, expr, node->important()));
    }
}

void AST_translator::visit(Unary_expr* node)
{
    emit(node->op()==Unary_expr::MINUS ? '-' : '+');
    node->term()->accept(*this);
}

void AST_translator::visit(Binary_expr* node)
{
    node->lhs()->accept(*this);
    emit(binary_expr_op_to_string(node->op()));
    node->rhs()->accept(*this);
}

void AST_translator::visit(Dimension_term* node)
{
    emit(node->value());
}

void AST_translator::visit(Function_term* node)
{
    if (node->function_name() == "eval") {
        emit(evaluate(node->expr().get()));
    }
    else {
        emit(node->function_name());
        emit('(');
        if (node->expr()) {
            node->expr()->accept(*this);
        }
        emit(')');
    }
}

void AST_translator::visit(Hash_term* node)
{
    emit(node->value());
}

void AST_translator::visit(Identifier_term* node)
{
    emit(node->value());
}

void AST_translator::visit(Number_term* node)
{
    emit(node->value());
}

void AST_translator::visit(Percentage_term* node)
{
    emit(node->value());
}

void AST_translator::visit(String_term* node)
{
    emit(node->value());
}

void AST_translator::visit(URL_term* node)
{
    emit("url(");
    emit(node->value());
    emit(")");
}

void AST_translator::visit(Variable_term* node)
{
    Symbol_table::iterator i = m_symbol_table.find(node->name());
    if (i != m_symbol_table.end()) {
        emit(i->second.expr());
        return;
    }
    add_compile_error(node->location(),
                      "variable " + node->name() + " not defined");
}

void AST_translator::enter_ruleset_scope()
{
    m_selector_array_stack.resize(m_selector_array_stack.size() + 1);
    m_property_set_stack.resize(m_property_set_stack.size() + 1);
}

void AST_translator::leave_ruleset_scope()
{
    CSS_selector_array selectors = assemble_selectors();
    CSS_property_set properties = assemble_properties();
    for (int i = 0, n = selectors.size(); i < n; i++) {
        add_css_ruleset(
            CSS_ruleset_ptr(new CSS_ruleset(CSS_selector_set(selectors[i]),
                                            properties)));
    }
    m_selector_array_stack.pop_back();
    m_property_set_stack.pop_back();
}

void AST_translator::add_css_ruleset(CSS_ruleset_ptr r)
{
    if (m_inside_css_media_block) {
        m_inside_css_media_block->add_ruleset(r);
    }
    else {
        m_css_rulesets.push_back(r);
    }
}

AST_translator::CSS_selector_array AST_translator::assemble_selectors()
{
    typedef CSS_selector_array_stack::iterator Stack_iter;
    typedef CSS_selector_array::iterator Iter;

    CSS_selector_array_stack& stk = m_selector_array_stack; // for brevity

    if (stk.empty()) {
        return CSS_selector_array();
    }

    CSS_selector_array answer(stk.front());
    CSS_selector_array tmp;

    for (Stack_iter i(stk.begin()+1); i != stk.end(); ++i) {
        tmp.clear();
        tmp.swap(answer);
        for (Iter j(i->begin()); j != i->end(); ++j) {
            for (Iter k(tmp.begin()); k != tmp.end(); ++k) {
                answer.push_back(*k + ' ' + *j);
            }
        }
    }
    return answer;
}

CSS_property_set AST_translator::assemble_properties()
{
    assert(!m_property_set_stack.empty());
    return m_property_set_stack.back();
}

void AST_translator::set_variable(string name, Var_info new_info)
{
    Symbol_table::iterator iter = m_symbol_table.find(name);
    if (iter != m_symbol_table.end()) {
        Var_info& old_info = iter->second;
        if (old_info.readonly()) {
            add_compile_error(
                new_info.location(),
                "variable " + name + " already defined readonly at "
                + old_info.location().to_string());
        }
        else if (new_info.readonly()) {
            add_compile_error(
                new_info.location(),
                "variable " + name + " already defined non-readonly at "
                + old_info.location().to_string());
        }
        old_info.set_expr(new_info.expr());
    }
    m_symbol_table.insert(make_pair(name, new_info));
}

void AST_translator::push_emit_target()
{
    m_emit_target_stack.push_back("");
}

string AST_translator::pop_emit_target()
{
    assert(!m_emit_target_stack.empty());
    string answer = m_emit_target_stack.back();
    m_emit_target_stack.pop_back();
    return answer;
}

void AST_translator::emit(const char* s)
{
    assert(!m_emit_target_stack.empty());
    m_emit_target_stack.back() += s;
}

void AST_translator::emit(char c)
{
    assert(!m_emit_target_stack.empty());
    m_emit_target_stack.back() += c;
}

void AST_translator::emit(const std::string& s)
{
    assert(!m_emit_target_stack.empty());
    m_emit_target_stack.back() += s;
}

string AST_translator::evaluate(Expr* expr)
{
    try {
        AST_evaluator evaluator(m_symbol_table);
        return evaluator.eval(expr);
    }
    catch (const Compile_error& e) {
        m_errors.push_back(e);
        return "";
    }
}

void AST_translator::add_compile_error(const Location& loc, const string& msg)
{
    m_errors.push_back(Compile_error(loc, msg));
}

