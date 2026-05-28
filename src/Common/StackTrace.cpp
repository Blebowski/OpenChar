#include <execinfo.h>
#include <signal.h>
#include <cxxabi.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <Utils.h>

static void SegFaultHandler(int sig) {
    void *array[10];
    int size = backtrace(array, 10);

    fprintf(stderr, "Error: signal %d:\n", sig);

    // TODO: Refactor this, it is quick AI generated slop!
    char **symbols = backtrace_symbols(array, size);
    for (int i = 0; i < size; i++) {
        // symbol format: "module(mangled_name+offset) [address]"
        char *begin = strchr(symbols[i], '(');
        char *end   = begin ? strchr(begin, '+') : nullptr;

        if (begin && end) {
            *begin = '\0';
            *end   = '\0';
            int status = 0;
            char *demangled = abi::__cxa_demangle(begin + 1, nullptr, nullptr, &status);
            if (status == 0) {
                fprintf(stderr, "  %s : %s+%s\n", symbols[i], demangled, end + 1);
                free(demangled);
            } else {
                fprintf(stderr, "  %s : %s+%s\n", symbols[i], begin + 1, end + 1);
            }
        } else {
            fprintf(stderr, "  %s\n", symbols[i]);
        }
    }
    free(symbols);
    exit(1);
}

void StackTraceInit()
{
    signal(SIGSEGV, SegFaultHandler);
    signal(SIGABRT, SegFaultHandler);
}