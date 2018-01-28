#ifndef included_ecss_translator
#define included_ecss_translator

#include "ecss/ast.h"
#include "ecss/css.h"
#include "ecss/error.h"
#include "ecss/settings.h"
#include "ecss/symbol_table.h"
#include <map>
#include <vector>

namespace ecss_core {

class AST_translator : public AST_visitor {
public:
    AST_translator(Compiler_settings_ptr);
    virtual ~AST_translator() {}
    std::string translate(AST_node* node);
    const std::vector<Compile_error>& compile_errors() const;

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
    typedef std::map<std::string, Flavor_declaration*> Flavor_symbol_table;
    typedef std::vector<CSS_selector> CSS_selector_array;
    typedef std::vector<CSS_selector_array> CSS_selector_array_stack;
    typedef std::vector<CSS_property_set> CSS_property_set_stack;

private:
    void enter_ruleset_scope();
    void leave_ruleset_scope();
    void add_css_ruleset(CSS_ruleset_ptr);
    CSS_selector_array assemble_selectors();
    CSS_property_set assemble_properties();
    void set_variable(std::string name, Var_info info);
    void push_emit_target();
    std::string pop_emit_target();
    void emit(const char*);
    void emit(char);
    void emit(const std::string&);
    std::string evaluate(Expr* expr);
    void add_compile_error(const Location&, const std::string&);

private:
    Compiler_settings_ptr m_compiler_settings;
    std::vector<std::string> m_emit_target_stack;
    std::vector<int> m_simple_selector_sequence_stack;
    Symbol_table m_symbol_table;
    Flavor_symbol_table m_flavor_symbol_table;
    std::vector<std::string> m_flavor_stack;
    CSS_selector_array_stack m_selector_array_stack;
    CSS_property_set_stack m_property_set_stack;
    CSS_media_block_ptr m_inside_css_media_block;
    std::vector<std::string> m_css_imports;
    std::vector<CSS_ruleset_ptr> m_css_rulesets;
    std::vector<CSS_media_block_ptr> m_css_media_blocks;
    std::vector<Compile_error> m_errors;
};

} // namespace ecss_core

#endif
