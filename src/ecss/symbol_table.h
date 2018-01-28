#ifndef included_ecss_symbol_table
#define included_ecss_symbol_table

#include "ecss/lexer.h"
#include <string>
#include <map>

namespace ecss_core {

class Var_info {
public:
    Var_info(std::string expr, const Location& location, bool readonly);
    void set_expr(std::string expr) { m_expr = expr; }
    const std::string& expr() const { return m_expr; }
    const Location& location() const { return m_location; }
    bool readonly() const { return m_readonly; }

private:
    std::string m_expr;     // string representation of the variable's value
    Location m_location;    // variable's point of definition in the source
    bool m_readonly;        // was this variable tagged as read-only?
};

typedef std::map<std::string, Var_info> Symbol_table;

} // namespace ecss_core

#endif
