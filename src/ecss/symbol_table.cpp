#include "ecss/symbol_table.h"

using namespace std;
using ecss_core::Var_info;

Var_info::Var_info(string expr, const Location& location, bool readonly)
: m_expr(expr)
, m_location(location)
, m_readonly(readonly)
{}
