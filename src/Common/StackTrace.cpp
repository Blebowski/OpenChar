#include <execinfo.h>
#include <signal.h>

#include <Utils.h>

static void SegFaultHandler(int sig) {
    void *array[10];
    int size = backtrace(array, 10);

    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

void StackTraceInit()
{
    signal(SIGSEGV, SegFaultHandler);
    signal(SIGABRT, SegFaultHandler);
}