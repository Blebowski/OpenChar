
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

    std::string cmd = R"(
        namespace path [concat [namespace path] open_char]
        namespace eval open_char_terminal {
            proc open_char_completer {text start end line} {

                # Find all available commands
                set matches [info commands "::open_char::${text}*"]

                # Filter namespace prefix
                set clean_matches {}
                foreach m $matches {
                    lappend clean_matches [string map {::open_char:: ""} $m]
                }

                # 3. Find the common prefix among all matches
                set first [lindex $clean_matches 0]
                set last  [lindex $clean_matches end]
                set prefix ""
                for {set i 0} {$i < [string length $first]} {incr i} {
                    if {[string index $first $i] eq [string index $last $i]} {
                        append prefix [string index $first $i]
                    } else {
                        break
                    }
                }

                # Return sorted unique list
                return [linsert $clean_matches 0 $prefix]
            }
            proc execute_command {line interactive} {
                if {$line eq "exit" || $line eq "quit"} { exit }

                set trimmed [string trim $line]
                if {[string length $trimmed] > 0} {
                    # Add to Readline (for arrow keys)
                    ::tclreadline::readline add $line

                    # Add to Tcl History (for the 'history' command)
                    # This makes the 'history' command actually show results
                    history add $line

                    if {$interactive == 0} {
                        puts $line
                        flush stdout
                    }
                    if {[catch {uplevel #0 $line} result]} {
                        if {[string match "invalid command name*" $result]} {
                            puts "Error: $result"
                        }
                    } elseif {$result ne ""} {
                        puts $result
                    }
                }
            }
            proc run {} {
                ::tclreadline::readline initialize ~/.openchar_history
                ::tclreadline::readline builtincompleter 0
                ::tclreadline::readline customcompleter open_char_completer
                )";

    if (argt[ARG_FILE].present) {
        cmd += open_char::sprintf(R"(
                set script_file %s
                if {[file exists $script_file]} {
                    puts "Executing file: $script_file"
                    set chan [open $script_file r]
                    set accumulator ""

                    while {[gets $chan line] >= 0} {
                        append accumulator $line "\n"

                        # Check if the current buffer is a complete Tcl command
                        if {[info complete $accumulator]} {
                            set cmd [string trim $accumulator]
                            if {$cmd ne ""} {
                                execute_command $cmd 0
                            }
                            set accumulator ""
                        }
                    }
                    close $chan
                } else {
                    puts "Failed to open file: $script_file"
                    exit
                }
                )", argt[ARG_FILE].val);
    }

    cmd += R"(  while {1} {
                    set line [::tclreadline::readline read "open_char> "]
                    execute_command $line 1
                }
            }
        }
        open_char_terminal::run)";

    if (Tcl_Eval(interp, cmd.c_str()) == TCL_ERROR) {
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