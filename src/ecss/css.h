#ifndef included_ecss_css
#define included_ecss_css

#include <boost/shared_ptr.hpp>
#include <list>
#include <string>
#include <map>
#include <set>
#include <vector>

namespace ecss_core {

typedef std::string CSS_selector;

class CSS_selector_set {
public:
    CSS_selector_set() {}
    explicit CSS_selector_set(const std::string&);
    void add_selector(const CSS_selector&);
    void compose(const CSS_selector_set&);
    bool operator==(const CSS_selector_set&) const;
    bool operator< (const CSS_selector_set&) const;
    std::string to_string() const;

private:
    typedef std::set<std::string> Set;
    Set m_set;
};

class CSS_property {
public:
    CSS_property(const std::string& name,
                 const std::string& expr,
                 bool important);
    const std::string& name() const { return m_name; }
    const std::string& expr() const { return m_expr; }
    bool important() const { return m_important; }
    std::string to_string() const;

private:
    std::string m_name;
    std::string m_expr;
    bool m_important;
};

class CSS_property_set {
public:
    void add_property(const CSS_property&);
    void compose(const CSS_property_set&);
    int size() const;
    std::string to_string() const;

private:
    typedef std::list<CSS_property> List;
    List m_list;
};

class CSS_ruleset {
public:
    CSS_ruleset(const CSS_selector_set& s, const CSS_property_set& p);
    void compose_properties(CSS_ruleset* r);
    CSS_selector_set& selectors() { return m_selectors; }
    const CSS_selector_set& selectors() const { return m_selectors; }
    CSS_property_set& properties() { return m_properties; }
    const CSS_property_set& properties() const { return m_properties; }
    std::string to_string() const;

private:
    CSS_selector_set m_selectors;
    CSS_property_set m_properties;
};

typedef boost::shared_ptr<CSS_ruleset> CSS_ruleset_ptr;

class CSS_media_block {
public:
    void add_medium(std::string medium);
    void add_ruleset(CSS_ruleset_ptr r);
    std::string to_string() const;

private:
    std::vector<std::string> m_media;
    std::vector<CSS_ruleset_ptr> m_css_rulesets;
};

typedef boost::shared_ptr<CSS_media_block> CSS_media_block_ptr;
/*
class CSS_checker : public AST_visitor {
public:
    CSS_checker(Compiler_settings_ptr);
    virtual ~CSS_checker() {}
    void run(AST_node* node);
    const std::vector<Compile_error>& errors() const;

private:
    void visit(Stylesheet*);
    void visit(Path_import*);
    void visit(URI_import*);
    void visit(Variable_declaration*);
    void visit(Flavor*);
    void visit(Flavor_declaration*);
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
};
*/

} // namespace ecss_core

#endif
