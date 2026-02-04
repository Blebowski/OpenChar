
#include "open_char.h"
#include "Context.h"
#include "Utils.h"

#include <tcl.h>

open_char::Context *ctx;

typedef enum {
    ARG_HELP,
    ARG_FILE,
    ARG_LAST
} t_arg_kind;

typedef struct {
    t_arg_kind      kind;
    std::string     name;
    std::string     val_name;
    bool            present;
    std::string     val;
    std::string     desc;
} t_cli_arg;

static t_cli_arg argt[] = {
    {ARG_HELP, "-help",   "",           false,   "",    "Display this help message"},
    {ARG_FILE, "-file",   "<script>",   false,   "",    "Execute TCL script file"},
    {ARG_LAST, "",        "",           false,   "",    ""}
};

extern "C" {
    int Tclreadline_Init(Tcl_Interp *interp);
}

int ShellInit(Tcl_Interp *interp)
{
    ctx->interp_ = interp;

    if (Tcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    if (Tclreadline_Init(interp) != TCL_OK) {
        open_char::printf("tclreadline Init Failed: %s\n", Tcl_GetStringResult(interp));
        return TCL_ERROR;
    }

    open_char::RegisterTclCommands(ctx);

    if (argt[ARG_FILE].present) {
        open_char::printf("Evaluating file: %s\n", argt[ARG_FILE].val);
        Tcl_EvalFile(interp, argt[ARG_FILE].val.c_str());
    }

    const char* loop_script = R"(
        if {[info commands history] eq ""} {
            auto_load history
        }
        namespace eval my_terminal {
            proc run {} {
                ::tclreadline::readline initialize ~/.openchar_history

                while {1} {
                    set line [::tclreadline::readline read "open_char> "]

                    if {$line eq "exit" || $line eq "quit"} { break }

                    set trimmed [string trim $line]
                    if {[string length $trimmed] > 0} {
                        # Add to Readline (for arrow keys)
                        ::tclreadline::readline add $line

                        # Add to Tcl History (for the 'history' command)
                        # This makes the 'history' command actually show results
                        history add $line

                        if {[catch {uplevel #0 $line} result]} {
                            puts stderr "Error: $result"
                        } elseif {$result ne ""} {
                            puts $result
                        }
                    }
                }
            }
        }
        my_terminal::run
    )";

    if (Tcl_Eval(interp, loop_script) == TCL_ERROR) {
        Tcl_DeleteInterp(interp);
        return TCL_ERROR;
    }

    Tcl_DeleteInterp(interp);
    return TCL_OK;
}

void PrintUsage()
{
    open_char::printf("OpenChar - Open Source library characterization tool\n");
    open_char::printf("Usage: open_char [options]\n\n");

    open_char::printf("Options:\n");
    for (t_cli_arg *ent = argt; ent->kind != ARG_LAST; ent++)
        open_char::printf("    %s %-10s %s\n", ent->name, ent->val_name, ent->desc);
}

int ParseArgs(int argc, char *argv[])
{
    int argi = 1;
    while (argi < argc) {
        bool found = false;

        for (t_cli_arg *ent = argt; ent->kind != ARG_LAST; ent++) {
            if (std::string(argv[argi]) != ent->name)
                continue;

            ent->present = true;
            found = true;

            if (ent->val_name == "")
                break;

            if (argi >= argc - 1) {
                PrintUsage();
                return TCL_ERROR;
            }

            argi++;
            ent->val = std::string(argv[argi]);
            break;
        }

        if (!found) {
            open_char::error("Invalid argument %s\n", argv[argi]);
            PrintUsage();
            return TCL_ERROR;
        }

        argi++;
    }

    return TCL_OK;
}

int main(int argc, char *argv[])
{
    ctx = new open_char::Context;

    int parse_ret = ParseArgs(argc, argv);
    if (parse_ret != TCL_OK)
        return parse_ret;

    if (argt[ARG_HELP].present) {
        PrintUsage();
        return 0;
    }

    Tcl_FindExecutable(argv[0]);
    Tcl_Main(argc, argv, ShellInit);

    // TODO: This is not called since "exit" is handled within Tcl_Main to exit
    //       the code!
    delete ctx;

    return 0;
}