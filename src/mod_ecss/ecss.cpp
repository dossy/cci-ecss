// apache and apr headers
#include "apr_buckets.h"
#include "apr_strings.h"
#include "ap_config.h"
#include "util_filter.h"
#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_log.h"
#include "http_main.h"
#include "util_script.h"
#include "http_core.h"

// ecss compiler headers
#include "ecss/parser.h"
#include "ecss/translator.h"

// boost headers
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace ecss_core;

// +-------------------------------------+
// |                                     |
// |    TYPE & MODULE DECLARATIONS
// |                                     |
// +-------------------------------------+

struct My_cfg {
    char* include_path;
    bool preserve_order;
};

struct My_state {
    apr_bucket_brigade* bb;
    bool seen_eos;
};

static void* create_server_config(apr_pool_t*, server_rec*);
static void* merge_server_config(apr_pool_t*, void*, void*);
static My_cfg* get_sconfig(const server_rec* s);
static void register_hooks(apr_pool_t*);
static const char* set_include_path(cmd_parms*, void*, const char*);
static const char* preserve_order(cmd_parms*, void*, int);

static const command_rec ecss_cmds[] =
{
    AP_INIT_TAKE1(
        "ECSS_SetIncludePath",
        (cmd_func) set_include_path,
        NULL,
        RSRC_CONF,
        "Specify the list of paths to search in @include statements"),
    AP_INIT_FLAG(
        "ECSS_PreserveOrder",
        (cmd_func) preserve_order,
        NULL,
        RSRC_CONF,
        "Preserve ruleset ordering"),
    {NULL}
};

module AP_MODULE_DECLARE_DATA ecss_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,                 // per-directory config creator
    NULL,                 // dir config merger
    create_server_config, // server config creator
    merge_server_config,  // server config merger
    ecss_cmds,            // command table
    register_hooks
};

// +-------------------------------------+
// |                                     |
// |    ECSS COMPILATION
// |                                     |
// +-------------------------------------+

// Converts a list of compilation errors into a string suitable for display to
// the end user.
static string format_errors(const vector<Compile_error>& errors)
{
    string answer(
        "<html>"
        "<head>"
        "<title>ecss: Error Log</title>"
        "<style>"
		"* { font-family: verdana; font-size: 12px; padding: 0; margin: 0; }"
		".header { padding: 10px; }"
		"dt, dl, dd { font-family: verdana; font-size: 12px; color: #333; }"
		"dl { padding: 5px; margin-bottom: 10px; background: #fbf6c8; }"
		"dt { float: left; width: 50px; text-align: right; }"
		"dd { margin-left: 60px; }"
		"dd.error { margin-left: 60px; font-weight: bold; }"
        "</style>"
        "</head>"
        "<body>"
        "<div class=\"header\">"
        "<h4>ecss</h4>"
        "<p>Unable to parse this file due to the following errors:</p>"
        "</div>\n");

    for (vector<Compile_error>::const_iterator i(errors.begin());
         i != errors.end(); ++i)
    {
        answer +=
            "<dl><dt>path:</dt><dd>" +
            i->location().filename() +
            "</dd><dt>line #:</dt><dd>" +
            format("%d:%d", i->location().line()+1, i->location().column()) +
            "</dd><dt>error:</dt><dd class=\"error\">" +
            i->what() +
            "</dd></dl>\n";
    }

    answer += "</body></html>\n";
    return answer;
}

// This highly specialized function searches for a query argument named "b"
// and, if found, returns its value (as a pool-allocated character array). If
// no such argument exists, return null. Also see
// try_to_set_user_agent_from_query_args for more info.
static char* extract_browser_arg(ap_filter_t* f)
{
    if (!f->r->args || *f->r->args == '\0')     // reject empty/null args
        return 0;

    const char* p = f->r->args;
    for (;;) {
        if (p[0] == 'b' && p[1] == '=') {       // look for "b=..." here
            p += 2;                             // found it; skip past "b="
            const char* q = strchr(p, '&');     // find the next '&'...
            if (!q) q = strchr(p, '\0');        // ...or the end of the string
            return apr_pstrndup(f->r->pool, p, q-p);
        }
        if (!(p = strchr(p+1, '&')))            // skip to next '&'
            break;                              // give up if there isn't one
        ++p;                                    // skip to char after the '&'
    }
    return 0;                                   // didn't find a "b" arg
}

// Attempts to set the user agent based on a request query argument named "b".
// If no such arg was given, returns false. Otherwise, sets the user agent and
// returns true. We try to be forgiving about the format of the user agent
// name. For example, we will accept both "msie 6.0" and "msie6.0" to mean
// User_agent("msie", "6.0");
static bool try_to_set_user_agent_from_query_args(Compiler_settings_ptr cs,
                                                  ap_filter_t* f)
{
    #define SPACE_CHARS "+ "
    #define VERSION_CHARS "1234567890."

    char* b = extract_browser_arg(f);
    if (!b) return false;

    ap_unescape_url(b);

    const int name_len = strcspn(b, SPACE_CHARS VERSION_CHARS);
    const string name = string(b, name_len);
    const char* vs = strpbrk(b + name_len, VERSION_CHARS);
    const string version = vs ? string(vs, strspn(vs, VERSION_CHARS)) : "";

    cs->set_user_agent(User_agent(name, version));
    return true;

    #undef SPACE_CHARS
    #undef VERSION_CHARS
}

// Creates a Compiler_settings object based on the configuration settings
// passed to this module via Apache directives.
static Compiler_settings_ptr make_compiler_settings(ap_filter_t* f)
{
    Compiler_settings_ptr cs(new Compiler_settings);

    if (!try_to_set_user_agent_from_query_args(cs, f)) {
        const char* user_agent = apr_table_get(f->r->headers_in, "User-Agent");
        if (user_agent && *user_agent)
            cs->set_user_agent_string(user_agent);
    }

    My_cfg* cfg = get_sconfig(f->r->server);
    cs->set_preserve_order(cfg->preserve_order);
    if (cfg->include_path)
        cs->set_include_path(cfg->include_path);

    return cs;
}

// Given a character buffer representing raw ecss code, translate it to css and
// returns the resulting string. If any compilation errors occur, formats the
// errors into a string and returns that instead.
static pair<string, bool>
translate_ecss_to_css(ap_filter_t* f, const char* buf, int len)
{
    Compiler_settings_ptr settings(make_compiler_settings(f));
    Parser parser(string(buf, len), f->r->filename, settings);
    Stylesheet_ptr stylesheet = parser.parse();
    if (!stylesheet || !parser.compile_errors().empty())
        return make_pair(format_errors(parser.compile_errors()), false);
    AST_translator translator(settings);
    string css = translator.translate(stylesheet.get());
    if (!translator.compile_errors().empty())
        return make_pair(format_errors(translator.compile_errors()), false);
    return make_pair(css, true);
}

// +-------------------------------------+
// |                                     |
// |    CONFIGURATION HANDLING
// |                                     |
// +-------------------------------------+

static void* create_server_config(apr_pool_t* p, server_rec* s)
{
    My_cfg* cfg = (My_cfg*) apr_pcalloc(p, sizeof(*cfg));
    cfg->include_path = NULL;
    cfg->preserve_order = false;
    return cfg;
}

static void* merge_server_config(apr_pool_t* p, void* server1, void* server2)
{
    My_cfg* merged_conf = (My_cfg*) apr_pcalloc(p, sizeof(*merged_conf));
    My_cfg* s1conf = (My_cfg*) server1;
    My_cfg* s2conf = (My_cfg*) server2;

    // When it comes to the include path, the more-specific configuration
    // takes precedence. No merging necessary.
    merged_conf->include_path = s2conf->include_path
        ? apr_pstrdup(p, s2conf->include_path) : NULL;

    merged_conf->preserve_order =
        s1conf->preserve_order || s2conf->preserve_order;

    return merged_conf;
}

static My_cfg* get_sconfig(const server_rec* s)
{
    return (My_cfg*) ap_get_module_config(s->module_config, &ecss_module);
}

// +-------------------------------------+
// |                                     |
// |    COMMANDS
// |                                     |
// +-------------------------------------+

static const char* set_include_path(cmd_parms* cmd, void* dummy, const char* arg)
{
    My_cfg* cfg = get_sconfig(cmd->server);
    cfg->include_path = apr_pstrdup(cmd->pool, arg);
    return NULL; // success
}

static const char* preserve_order(cmd_parms* cmd, void* dummy, int flag)
{
    My_cfg* cfg = get_sconfig(cmd->server);
    cfg->preserve_order = (bool) flag;
    return NULL; // success
}

// +-------------------------------------+
// |                                     |
// |    MAIN FILTER ROUTINES
// |                                     |
// +-------------------------------------+

// This is the main filter handler. Because the ecss compiler requires all of
// its input before it can begin translating, we must accumulate bucket
// brigades in a private area until we encounter the EOS bucket.
static apr_status_t filter(ap_filter_t* f, apr_bucket_brigade* bb)
{
    if (APR_BRIGADE_EMPTY(bb))
        return APR_SUCCESS;

    // If our context is null, we are in the very first call to this filter
    // for this request, which means we can manipulate the response headers.
    // We also need to initialize our filter state and store it in f->ctx.
    My_state* ctx = (My_state*) f->ctx;
    if (ctx == NULL) {
        f->ctx = ctx = (My_state*) apr_pcalloc(f->r->pool, sizeof(*ctx));
        ctx->bb = 0;
        ctx->seen_eos = false;

        apr_table_unset(f->r->headers_out, "Content-Length");
        f->r->content_type = "text/css";
    }

    // If we have already seen the EOS bucket, all bucket brigades that follow
    // it must be ignored and passed to the next filter.
    if (ctx->seen_eos)
        return ap_pass_brigade(f->next, bb);

    // Accumulate bb's buckets in ctx->bb, leaving bb empty.
    ap_save_brigade(f, &ctx->bb, &bb, f->r->pool);

    // If we have not seen the EOS bucket, return and wait for more buckets.
    if (!APR_BUCKET_IS_EOS(APR_BRIGADE_LAST(ctx->bb)))
        return APR_SUCCESS;
    ctx->seen_eos = true;

    // Our bucket brigade (ctx->bb) now contains all of the buckets up to the
    // EOS bucket. Flatten it into a character buffer and clear the brigade.
    char* buf;
    apr_size_t len;
    apr_brigade_pflatten(ctx->bb, &buf, &len, f->r->pool);
    apr_brigade_cleanup(ctx->bb);

    // Run the ecss compiler on the character string, then convert it back
    // into a bucket brigade and pass it to the next filter (plus EOS).
    const pair<string, bool> answer = translate_ecss_to_css(f, buf, len);
    if (!answer.second)
        f->r->content_type = "text/html";
    ap_fputs(f, ctx->bb, answer.first.c_str());
	APR_BRIGADE_INSERT_TAIL(ctx->bb, apr_bucket_eos_create(f->c->bucket_alloc));
    ap_pass_brigade(f->next, ctx->bb);

    return APR_SUCCESS;
}

static void register_hooks(apr_pool_t* p)
{
    ap_register_output_filter("ECSS", filter, NULL, AP_FTYPE_RESOURCE);
}
