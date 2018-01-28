#include "ecss/css.h"
#include "ecss/utility.h"

using namespace std;
using namespace ecss_core;

// +------------------+
// | CSS_selector_set |
// +------------------+

CSS_selector_set::CSS_selector_set(const string& s)
{
    add_selector(s);
}

void CSS_selector_set::add_selector(const CSS_selector& s)
{
    m_set.insert(s);
}

void CSS_selector_set::compose(const CSS_selector_set& s)
{
    m_set.insert(s.m_set.begin(), s.m_set.end());
}

bool CSS_selector_set::operator==(const CSS_selector_set& rhs) const
{
    return m_set == rhs.m_set;
}

bool CSS_selector_set::operator< (const CSS_selector_set& rhs) const
{
    return m_set < rhs.m_set;
}

string CSS_selector_set::to_string() const
{
    if (m_set.empty()) {
        return "";
    }

    string answer;
    Set::const_iterator i = m_set.begin();
    for (;;) {
        answer += *i;
        if (++i == m_set.end()) break;
        answer += ',';
    }
    return answer;
}

// +--------------+
// | CSS_property |
// +--------------+

CSS_property::CSS_property(const string& name,
                           const string& expr,
                           bool important)
: m_name(name)
, m_expr(expr)
, m_important(important)
{}

string CSS_property::to_string() const
{
    return m_name + ":" + m_expr + (m_important ? " !important" : "") + ';';
}

// +------------------+
// | CSS_property_set |
// +------------------+

void CSS_property_set::add_property(const CSS_property& p)
{
    for (List::iterator i = m_list.begin(); i != m_list.end(); ++i) {
        if (i->name() == p.name()) {
            if (i->important() && !p.important())
                return;
            m_list.erase(i);
            break;
        }
    }
    m_list.push_back(p);
}

void CSS_property_set::compose(const CSS_property_set& s)
{
    // FIXME: We're O(N*M) but could be O(M) with a change of data structure.
    for (List::const_iterator i(s.m_list.begin()); i != s.m_list.end(); ++i) {
        add_property(*i);
    }
}

int CSS_property_set::size() const
{
    return m_list.size();
}

string CSS_property_set::to_string() const
{
    if (m_list.empty()) {
        return "";
    }

    string answer;
    List::const_iterator i(m_list.begin());
    for (;;) {
        answer += i->to_string();
        if (++i == m_list.end()) {
            break;
        }
    }
    return answer;
}

// +-------------+
// | CSS_ruleset |
// +-------------+

CSS_ruleset::CSS_ruleset(const CSS_selector_set& s, const CSS_property_set& p)
: m_selectors(s)
, m_properties(p)
{}

void CSS_ruleset::compose_properties(CSS_ruleset* rs)
{
    m_properties.compose(rs->properties());
}

string CSS_ruleset::to_string() const
{
    return m_selectors.to_string() + '{' + m_properties.to_string() + '}';
}

// +-----------------+
// | CSS_media_block |
// +-----------------+

void CSS_media_block::add_medium(string medium)
{
    m_media.push_back(medium);
}

void CSS_media_block::add_ruleset(CSS_ruleset_ptr r)
{
    m_css_rulesets.push_back(r);
}

string CSS_media_block::to_string() const
{
    string answer = "@media " + join(", ", m_media) + " {\n";
    for (int i = 0; i < int(m_css_rulesets.size()); i++) {
        answer += m_css_rulesets[i]->to_string();
        answer += '\n';
    }
    answer += '}';
    return answer;
}
