---
name: tcl_test
description: Use when writing or updating a golden-output TCL command test under test/tcl/ in the OpenChar repo (e.g. testing define_cell, define_template, or any other Tcl command defined in src/Tcl/TclCmdDefs.cpp). Covers test layout, how output is generated/compared, and how to derive exact expected stdout/stderr text from the command implementation.
---

# Writing a TCL command test (test/tcl/<cmd_name>)

## Layout

Each command gets its own directory under `test/tcl/<cmd_name>/` with exactly three files:

- `script.tcl` — the Tcl commands to run, ending in a blank line then `exit`.
- `stdout.txt` — expected stdout.
- `stderr.txt` — expected stderr.

Register it in `test/tcl/CMakeLists.txt`:

```
OC_TCL_TEST(<cmd_name>)
```

`OC_TCL_TEST` (in `test/CMakeLists.txt`) wires this to `test/tcl/test_script.sh`, which runs:

```
$OCC -file <TEST_FOLDER>/script.tcl > stdout.txt 2> stderr.txt
diff -I "Executing file:" stdout.txt <TEST_FOLDER>/stdout.txt   # ignores the path on this one line
diff stderr.txt <TEST_FOLDER>/stderr.txt                        # exact match, no filtering
```

`stderr.txt` is compared with a plain `diff` — it must match byte-for-byte, no ignored lines.

## What actually lands in stdout vs stderr

Look at `src/open_char.cpp`'s `execute_command` proc: for non-interactive (`-file`) execution it does
`puts $line; flush stdout` for every command before evaluating it, and then the `CREATE_TCL_COMMAND`
wrapper (`src/Tcl/TclCmdDefs.h`) prints `1` (TCL_OK) or `0` (TCL_ERROR) via `printf`, followed by an
`fflush(stdout)`, if the command was declared with `print_ret_code=true`. So a normal `stdout.txt` looks
like:

```
Executing file: <ignored by -I filter>
<echoed command 1>
<0 or 1>
<echoed command 2>
<0 or 1>
...
```

**Buffering trap:** the `flush stdout` after `puts` and the `fflush(stdout)` after the return-code
`printf` are both required for this interleaved order to happen when stdout is redirected to a file
(as the test does). Without an explicit flush on the C side, glibc fully-buffers stdout when it isn't a
tty, so all the `printf("0"/"1")` calls silently queue up and only get flushed as one block at process
exit — producing "all commands, then all exit codes" instead of interleaved output. If a return-code
flush is ever removed from `TclCmdDefs.h`, `stdout.txt` must go back to the "all commands, then all
codes" layout (echoed commands in order, followed by a block of every return code in matching order) — don't assume interleaved order, verify which buffering behavior is actually in place.

Only `error()`/`fatal()` calls (see `src/Common/Utils.h`) write to stderr, each printing
`"Error: " + fmt + "\n"`. **Do not** trust the literal format string blindly — if a command's format
string already ends in `\n`, you get an extra blank line in stderr (this was an actual bug found in
`DefineTemplate` and later fixed). Check the current format strings in the command you're testing;
don't assume they're newline-free just because one command (`DefineCell`) is.

## Deriving exact expected error text — read the command, don't guess

For the command under test in `src/Tcl/TclCmdDefs.cpp`:

1. Check each `TclCmdOpt` in its `params`: an option name starting with `-` is an *optional switch*
   (`isOptional()` in `TclCmdOpt.cpp` just checks `name_[0] == '-'`); anything else is a *mandatory
   positional argument*.
2. `TclCmd::ParseArgs` (`src/Tcl/TclCmd.cpp`) enforces positional-argument count and rejects unknown
   switches **before** `Execute()` ever runs (see the `CREATE_TCL_COMMAND` macro in `TclCmdDefs.h`:
   it returns early if `ParseArgs` didn't return `TCL_OK`). So a manual `if (!opts_["positional_name"].isSet())`
   check inside `Execute()` for a mandatory positional argument is dead code — it can never fire, and
   should not be tested (there's nothing reachable to assert). Only test the `isSet()` checks that
   guard *optional* (`-xxx`) switches.
3. Walk every `error(...)` call reachable in `Execute()` in source order, and match the literal text
   verbatim (capitalization, punctuation, quoting style like `'%s'` vs `%s`). Trace the *order* the
   checks run in, since that determines which error fires first when multiple conditions are violated
   in one test line.
4. Watch for stale-state bugs in loops/lambdas: e.g. a `min` variable meant to track "previous value
   seen" that's captured by reference into a lambda but never reassigned inside it makes a monotonicity
   check permanently dead (this was a real bug in `DefineTemplate`'s index validation — `min = v;` was
   missing, so `v <= min` could never be true). Don't assume a check is reachable just because the code
   for it exists; trace whether the variables it depends on are actually updated.
5. Also watch for copy-paste bugs where an error message hardcodes one option name (e.g. always says
   `-index_1`) even when reused for validating a different option (e.g. `-index_2`) — reflect the actual
   (buggy) text in the fixture, and separately flag it to the user rather than silently "fixing" the
   expected output to what seems more correct.

## Building the test script

Interleave error-triggering calls with successful calls, reusing a running Tcl interpreter state (e.g.
define supporting templates/cells needed by later commands), similar to `test/tcl/define_cell/script.tcl`.
Cover: each mandatory-*switch* missing, each invalid-value check, each cross-field validation, and a
duplicate-definition check, then one or more successful definitions exercising each valid enum branch
(e.g. every `-type` value), and end with a duplicate of an already-defined name.

## Verifying

Prefer actually building and running the test (`ctest` / `test_script.sh`) over pure static derivation
whenever you're allowed to — buffering behavior and dead-code paths are easy to get subtly wrong by
reading source alone. If you can't build, say so explicitly and flag exactly which parts of the expected
output are inferred vs. verified.
