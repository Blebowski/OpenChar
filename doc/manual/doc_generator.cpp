
#include "open_char.h"
#include "Utils.h"

#include "Context.h"

static std::string escape_string(std::string s)
{
    std::string res;
    res.reserve(s.size());

    for (auto c : s) {
        if (c == '{' || c == '}') {
            res += '\\';
        }
        res += c;
    }
    return res;
}

int main(int argc, const char **argv)
{
    open_char::Context ctx(nullptr);
    CreateTclCommands(&ctx);

    assert(argc == 2);
    const char *doc_folder = *(argv + 1);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Generate TCL commands documentation
    ///////////////////////////////////////////////////////////////////////////////////////////////

    for (auto [cmd,cb] : ctx.GetTclCommands()) {
        const char *cmd_name = cmd.name_.c_str();

        std::string fpath = std::string(doc_folder) + "/commands/" + std::string(cmd_name) + ".tex";
        FILE *f = fopen(fpath.c_str(), "w");

        fprintf(f, "\\pagebreak\n");
        fprintf(f, "\\subsection*{%s}\n", cmd_name);

        fprintf(f, "\\subsubsection*{Syntax}\n");
        fprintf(f, "\\addcontentsline{toc}{subsection}{\\protect\\numberline{}%s}\n", cmd_name);
        fprintf(f, "\\texttt{\\textbf{%s}}\\newline\n", cmd_name);

        size_t max_opt_len = 0;
        size_t max_hint_len = 0;
        for (auto [opt_name, opt] : cmd.opts_) {
            if (opt_name.size() > max_opt_len) {
                max_opt_len = opt_name.size();
            }
            if (opt.has_value_ && (opt.value_desc_.size() > max_hint_len)) {
                max_hint_len = opt.value_desc_.size();
            }
        }

        for (auto [opt_name, opt] : cmd.opts_) {
            std::string opt_pad = std::string(max_opt_len - opt_name.size(), ' ');
            std::string hint_pad = (opt.has_value_) ? std::string(max_hint_len - opt.value_desc_.size(), ' '):
                                                      std::string(max_hint_len, ' ');

            fprintf(f, "\\hspace*{0.5cm}\\obeyspaces{\\texttt{%s%s %s%s %s}}\\newline\n",
                        opt.name_.c_str(), opt_pad.c_str(),
                        (opt.has_value_) ? escape_string(opt.value_desc_).c_str() : "", hint_pad.c_str(),
                        opt.desc_.c_str());
        }

        fprintf(f, "\\subsubsection*{Description}\n");
        fprintf(f, "%s", cmd.desc_.c_str());

        fclose(f);
    }

    return 0;
}