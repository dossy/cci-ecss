#ifndef included_ecss_optimizer
#define included_ecss_optimizer

#include "ecss/css.h"
#include "ecss/settings.h"

namespace ecss_core {

void optimize_css_rulesets(std::vector<CSS_ruleset_ptr>& rulesets,
                           Compiler_settings_ptr cs);

} // namespace ecss_core

#endif
