

#include "Context.h"

namespace open_char {

Context::Context(Tcl_Interp* tcl_interp) :
    library_(this),
    algorithms_(this),
    tcl_interp_(tcl_interp)
{}

Context::~Context()
{}

Library& Context::GetLibrary()
{
    return library_;
}

Algorithms& Context::GetAlgorithms()
{
    return algorithms_;
}

const std::vector<std::string>& Context::GetModels()
{
    return models_;
}

const std::vector<std::string>& Context::GetNetlists()
{
    return netlists_;
}

void Context::AddModel(const std::string& model)
{
    models_.push_back(model);
}

void Context::AddNetlist(const std::string& netlist)
{
    netlists_.push_back(netlist);
}

void Context::AddTclCommand(TclCmd cmd, Tcl_ObjCmdProc* cb)
{
    tcl_commands_.push_back({cmd, cb});
}

std::vector<std::pair<TclCmd, Tcl_ObjCmdProc*>>& Context::GetTclCommands()
{
    return tcl_commands_;
}

Tcl_Interp* Context::GetTclInterp()
{
    assert(tcl_interp_ != nullptr);
    return tcl_interp_;
}

Variables& Context::GetVariables()
{
    return variables_;
}

}