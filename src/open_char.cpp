
#include <fmt/format.h>
#include <fmt/printf.h>

#include <tcl.h>

#include "open_char.h"
#include "Context.h"


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

int ShellInit(Tcl_Interp *interp)
{
    if (Tcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    ctx->interp_ = interp;

    Tcl_SetVar(interp, "tcl_prompt1",
               "puts -nonewline \"open_char> \"; flush stdout",
               TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "tcl_prompt2",
               "puts -nonewline \"... \"; flush stdout",
               TCL_GLOBAL_ONLY);

    open_char::RegisterTclCommands(ctx);

    if (argt[ARG_FILE].present) {
        fmt::printf("Evaluating file: %s\n", argt[ARG_FILE].val);
        Tcl_EvalFile(interp, argt[ARG_FILE].val.c_str());
    }

    return TCL_OK;
}

void PrintUsage()
{
    fmt::printf("OpenChar - Open Source library characterization tool\n");
    fmt::printf("Usage: open_char [options]\n\n");

    fmt::printf("Options:\n");
    for (t_cli_arg *ent = argt; ent->kind != ARG_LAST; ent++)
        fmt::printf("    %s %-10s %s\n", ent->name, ent->val_name, ent->desc);
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
            fmt::printf("Error: Invalid argument %s\n", argv[argi]);
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