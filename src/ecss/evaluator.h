#ifndef included_ecss_evaluator
#define included_ecss_evaluator

#include "ecss/ast.h"
#include "ecss/error.h"
#include "ecss/symbol_table.h"
#include <string>
#include <vector>

namespace ecss_core {

class AST_evaluator : public AST_visitor {
public:
    AST_evaluator(const Symbol_table&);
    std::string eval(Expr* expr);

private:
    void visit(Stylesheet*);
    void visit(Path_import*);
    void visit(URI_import*);
    void visit(Variable_declaration*);
    void visit(Flavor_declaration*);
    void visit(Flavor*);
    void visit(Media_block*);
    void visit(Ruleset*);
    void visit(Selector*);
    void visit(Simple_selector_sequence*);
    void visit(Type_selector*);
    void visit(Attribute_selector*);
    void visit(Class_selector*);
    void visit(ID_selector*);
    void visit(Pseudo_selector*);
    void visit(Property*);
    void visit(Unary_expr*);
    void visit(Binary_expr*);
    void visit(Dimension_term*);
    void visit(Function_term*);
    void visit(Hash_term*);
    void visit(Identifier_term*);
    void visit(Number_term*);
    void visit(Percentage_term*);
    void visit(String_term*);
    void visit(URL_term*);
    void visit(Variable_term*);

private:
    class Value {
    public:
        Value(const std::string& s);
        Value(double d);
        const std::string& text() const;
        double number() const;
        Value add(const Value&);
        Value subtract(const Value&);
        Value multiply(const Value&);
        Value divide(const Value&);
        Value concatenate(const Value&);
        Value negate();
    private:
        std::string m_string;
        double m_double;
    };

private:
    void push_value(const Value&);
    Value pop_value();

private:
    const Symbol_table& m_symbol_table;
    std::vector<Value> m_stack;
};

} // namespace ecss_core

#endif
