//=============================================================================
// This program is a basic stdin->stdout filter; it reads ecss code from stdin,
// translates it into css, and writes the result to stdout. The caller may
// optionally provide a list of directories on the command line to be searched
// when a relative path is used in an @include statement.
// ============================================================================

#include "ecss/file_util.h"
#include "ecss/parser.h"
#include "ecss/translator.h"
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace ecss_core;
namespace po = boost::program_options;

namespace
{
    Compiler_settings_ptr g_settings;   // options for this compile run
    po::variables_map g_vm;             // stores command-line vars
    string g_input_filename;            // input path ("" for stdin)
    string g_input_buffer;              // the input
    string g_output_buffer;             // the output (translated css)

    void display_compile_errors(vector<Compile_error> errors)
    {
        for (vector<Compile_error>::iterator i(errors.begin());
             i != errors.end(); ++i)
        {
            cout << "ecss: " << i->to_string() << endl;
        }
    }

    void init_settings()
    {
        g_settings.reset(new Compiler_settings);
        if (g_vm.count("include-path")) {
            const vector<string> include_paths =
                g_vm["include-path"].as< vector<string> >();
            for (int i = 0, n = include_paths.size(); i < n; i++) {
                g_settings->add_include_dir(include_paths[i]);
            }
        }
        if (g_vm.count("preserve-order")) {
            g_settings->set_preserve_order(true);
        }
    }

    void read_input()
    {
        if (g_vm.count("input-file")) {
            try {
                g_input_filename = g_vm["input-file"].as<string>();
                g_input_buffer = read_file(g_input_filename);
            }
            catch (IO_error& e) {
                cout << "ecss: failed to open \"" << g_input_filename
                     << "\" for reading: " << e.to_string() << endl;
                exit(2);
            }
        }
        else {
            g_input_buffer = read_stdin();
        }
    }

    void translate()
    {
        Parser parser(g_input_buffer, g_input_filename, g_settings);
        Stylesheet_ptr stylesheet = parser.parse();
        if (!stylesheet || !parser.compile_errors().empty()) {
            display_compile_errors(parser.compile_errors());
            exit(2);
        }
        AST_translator translator(g_settings);
        g_output_buffer = translator.translate(stylesheet.get());
        if (!translator.compile_errors().empty()) {
            display_compile_errors(translator.compile_errors());
            exit(2);
        }
    }

    void write_output()
    {
        if (g_vm.count("output-file")) {
            string output_filename = g_vm["output-file"].as<string>();
            try {
                write_file(output_filename, g_output_buffer);
            }
            catch (IO_error& e) {
                cout << "ecss: failed to write to \"" << output_filename
                     << "\": " << e.to_string() << endl;
                exit(2);
            }
        }
        else {
            cout << g_output_buffer;
        }
    }
}


int main(int argc, char** argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "display this message")
        ("include-path,I", po::value<vector<string> >(), "set include path")
        ("input-file,i", po::value<string>(), "input file")
        ("output-file,o", po::value<string>(), "output file")
        ("preserve-order,p", "preserve ruleset declaration order");

    po::store(po::parse_command_line(argc, argv, desc), g_vm);
    po::notify(g_vm);

    if (g_vm.count("help")) {
        cout << desc << "\n";
        exit(1);
    }

    init_settings();
    read_input();
    translate();
    write_output();

    return 0;
}
