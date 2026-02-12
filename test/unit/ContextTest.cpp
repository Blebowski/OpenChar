
#include <cassert>

#include "Context.h"
#include "Library.h"
#include "Algorithms.h"

int main()
{
    using namespace open_char;

    Context *ctx = new Context(nullptr);

    [[maybe_unused]] Library &lib = ctx->GetLibrary();
    [[maybe_unused]] Algorithms &algos = ctx->GetAlgorithms();

    ctx->AddModel("ABCDE");
    ctx->AddModel("MY_MODEL");

    assert(ctx->GetModels()[0] == "ABCDE");
    assert(ctx->GetModels()[1] == "MY_MODEL");

    ctx->AddNetlist("PATH TO FANCY NETLIST");
    ctx->AddNetlist("NEXT NETLIST");
    ctx->AddNetlist("ANOTHER ONE");

    assert(ctx->GetNetlists()[0] == "PATH TO FANCY NETLIST");
    assert(ctx->GetNetlists()[1] == "NEXT NETLIST");
    assert(ctx->GetNetlists()[2] == "ANOTHER ONE");

    // Skip testing TCL commands, this will be well tested in end-to-end tests.

    delete ctx;
}