#include "ecss/optimizer.h"
#include <algorithm>

using namespace std;
using namespace ecss_core;

namespace
{
    struct Ruleset_less {
        bool operator()(CSS_ruleset_ptr lhs, CSS_ruleset_ptr rhs) {
            return lhs->selectors() < rhs->selectors();
        }
    };

    struct Is_empty_ruleset {
        bool operator()(CSS_ruleset_ptr r) {
            return r->properties().size() == 0;
        }
    };

    void sort_rulesets(vector<CSS_ruleset_ptr>& rulesets)
    {
        stable_sort(rulesets.begin(), rulesets.end(), Ruleset_less());
    }

    void compress_rulesets(vector<CSS_ruleset_ptr>& rs)
    {
        vector<CSS_ruleset_ptr> compressed;
        for (int i = 0, n = rs.size(); i < n; ) {
            const int begin = i++;
            while (i < n && rs[i]->selectors() == rs[begin]->selectors()) {
                rs[begin]->compose_properties(rs[i++].get());
            }
            compressed.push_back(rs[begin]);
        }
        rs = compressed;
    }

    void remove_empty_rulesets(vector<CSS_ruleset_ptr>& rs)
    {
        rs.erase(remove_if(rs.begin(), rs.end(), Is_empty_ruleset()),
                 rs.end());
    }
}

void ecss_core::optimize_css_rulesets(vector<CSS_ruleset_ptr>& rulesets,
                                      Compiler_settings_ptr cs)
{
    remove_empty_rulesets(rulesets);
    if (!cs->preserve_order())
        sort_rulesets(rulesets);
    compress_rulesets(rulesets);
}
