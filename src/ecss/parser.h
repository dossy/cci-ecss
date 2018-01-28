#ifndef included_ecss_parser
#define included_ecss_parser

#include "ecss/ast.h"
#include "ecss/error.h"
#include "ecss/settings.h"
#include "ecss/utility.h"
#include <cstdio>
#include <string>
#include <vector>

namespace ecss_core {

class Parser : boost::noncopyable {
public:
    Parser(const std::string& input_buffer,
           const std::string& filename,
           Compiler_settings_ptr cs);
    ~Parser();
    Stylesheet_ptr parse();
    const std::vector<Compile_error>& compile_errors() const;

private:
    class Impl;
    Impl* m_impl;
};

} // namespace ecss_core

#endif
