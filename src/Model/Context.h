
#ifndef CONTEXT_H
#define CONTEXT_H

#include <map>

#include "open_char.h"
#include "Cell.h"
#include "TclCmd.h"
#include "Algorithms.h"
#include "Library.h"
#include "Variables.h"
#include "SimulationPool.h"

namespace open_char {

class Context {

    public:
        Context(Tcl_Interp* tcl_interp_);
        ~Context();

        Library& GetLibrary();
        Algorithms& GetAlgorithms();

        void AddModel(const std::string& model);
        void AddNetlist(const std::string& netlist);

        void AddTclCommand(TclCmd cmd, Tcl_ObjCmdProc* cb);
        std::vector<std::pair<TclCmd, Tcl_ObjCmdProc*>>& GetTclCommands();

        Tcl_Interp* GetTclInterp();

        SimulationPool& GetSimulationPool();

        const std::vector<std::string>& GetModels();
        const std::vector<std::string>& GetNetlists();

        Variables &GetVariables();

    private:
        // Cell library to characterize
        Library library_;

        // Algorithms provider
        Algorithms algorithms_;

        // Analog inputs
        std::vector<std::string> models_;
        std::vector<std::string> netlists_;

        // Registered TCL commands
        std::vector<std::pair<TclCmd, Tcl_ObjCmdProc*>> tcl_commands_;

        // TCL variables
        Variables variables_;

        // TCL interpreter
        Tcl_Interp* tcl_interp_;

        // Simulation pool
        SimulationPool sim_pool_;
};

}

#endif