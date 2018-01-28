#include "ecss/evaluator.h"

using namespace std;
using ecss_core::AST_evaluator;

// +-------+
// | Value |
// +-------+

AST_evaluator::Value::Value(const string& s)
: m_string(s)
{
    try {
        m_double = boost::lexical_cast<double>(s);
    }
    catch (boost::bad_lexical_cast) {
        m_double = 0.0;
    }
}

AST_evaluator::Value::Value(double d)
: m_string(boost::lexical_cast<string>(d))
, m_double(d)
{}

const string& AST_evaluator::Value::text() const
{
    return m_string;
}

double AST_evaluator::Value::number() const
{
    return m_double;
}

AST_evaluator::Value AST_evaluator::Value::add(const Value& rhs)
{
    return Value(number() + rhs.number());
}

AST_evaluator::Value AST_evaluator::Value::subtract(const Value& rhs)
{
    return Value(number() - rhs.number());
}

AST_evaluator::Value AST_evaluator::Value::multiply(const Value& rhs)
{
    return Value(number() * rhs.number());
}

AST_evaluator::Value AST_evaluator::Value::divide(const Value& rhs)
{
    return Value(number() / rhs.number());
}

AST_evaluator::Value AST_evaluator::Value::concatenate(const Value& rhs)
{
    return Value(text() + rhs.text());
}

AST_evaluator::Value AST_evaluator::Value::negate()
{
    return Value(-1.0 * number());
}

// +---------------+
// | AST_evaluator |
// +---------------+

AST_evaluator::AST_evaluator(const Symbol_table& symbol_table)
: m_symbol_table(symbol_table)
{}

string AST_evaluator::eval(Expr* expr)
{
    expr->accept(*this);
    return pop_value().text();
}

void AST_evaluator::visit(Unary_expr* node)
{
    node->term()->accept(*this);
    Value value = pop_value();
    switch (node->op()) {
    case Unary_expr::MINUS:
        push_value(value.negate());
        break;
    case Unary_expr::PLUS:
        push_value(value);
        break;
    }
}

#define ILLEGAL_OPERATOR(s)                                             \
    throw Compile_error(node->location(), "illegal operator '"s"' in eval")

void AST_evaluator::visit(Binary_expr* node)
{
    node->lhs()->accept(*this);
    Value lhs = pop_value();

    node->rhs()->accept(*this);
    Value rhs = pop_value();

    switch (node->op()) {
    case Binary_expr::PLUS:
        push_value(lhs.add(rhs));
        break;
    case Binary_expr::MINUS:
        push_value(lhs.subtract(rhs));
        break;
    case Binary_expr::STAR:
        push_value(lhs.multiply(rhs));
        break;
    case Binary_expr::SLASH:
        push_value(lhs.divide(rhs));
        break;
    case Binary_expr::CONCAT:
        push_value(lhs.concatenate(rhs));
        break;
    case Binary_expr::COMMA:
        ILLEGAL_OPERATOR(",");
    case Binary_expr::EQUALS:
        ILLEGAL_OPERATOR("=");
    default:
        ECSS_PANIC(("unexpected binary operator %d", node->op()));
    }
}

void AST_evaluator::visit(Dimension_term* node)
{
    push_value(node->value());
}

void AST_evaluator::visit(Function_term* node)
{
    const string fn = node->function_name();

    if (fn == "floor") {
        node->expr()->accept(*this);
        push_value(Value(int(pop_value().number())));
    }
    else
        throw Compile_error(node->location(),
                            "illegal function '" + fn + "' in eval");
}

void AST_evaluator::visit(Hash_term* node)
{
    push_value(node->value());
}

void AST_evaluator::visit(Identifier_term* node)
{
    push_value(node->value());
}

void AST_evaluator::visit(Number_term* node)
{
    push_value(node->value());
}

void AST_evaluator::visit(Percentage_term* node)
{
    push_value(node->value());
}

void AST_evaluator::visit(String_term* node)
{
    push_value(node->value());
}

void AST_evaluator::visit(URL_term* node)
{
    push_value(node->value());
}

void AST_evaluator::visit(Variable_term* node)
{
    Symbol_table::const_iterator i = m_symbol_table.find(node->name());
    if (i != m_symbol_table.end()) {
        push_value(i->second.expr());
        return;
    }
    throw Compile_error(node->location(),
                        "variable " + node->name() + " not defined");
}

void AST_evaluator::push_value(const Value& v)
{
    m_stack.push_back(v);
}

AST_evaluator::Value AST_evaluator::pop_value()
{
    Value answer(m_stack.back());
    m_stack.pop_back();
    return answer;
}

#define UNEXPECTED_NODE(Type)                                          \
    void AST_evaluator::visit(Type*) {                                  \
        ECSS_PANIC(("Unexpected node type in AST being eval'd."));    \
        return;                                                         \
    }

UNEXPECTED_NODE(Attribute_selector)
UNEXPECTED_NODE(Class_selector)
UNEXPECTED_NODE(Flavor)
UNEXPECTED_NODE(Flavor_declaration)
UNEXPECTED_NODE(ID_selector)
UNEXPECTED_NODE(Media_block)
UNEXPECTED_NODE(Path_import)
UNEXPECTED_NODE(Property)
UNEXPECTED_NODE(Pseudo_selector)
UNEXPECTED_NODE(Ruleset)
UNEXPECTED_NODE(Selector)
UNEXPECTED_NODE(Simple_selector_sequence)
UNEXPECTED_NODE(Stylesheet)
UNEXPECTED_NODE(Type_selector)
UNEXPECTED_NODE(URI_import)
UNEXPECTED_NODE(Variable_declaration)

