
#ifndef VARIABLES_H
#define VARIABLES_H

#include <string>
#include <map>

#include "open_char.h"

namespace open_char {

class Variables {

    public:
        Variables();

        std::string SetVariable(std::string name, std::string value);
        std::string GetVariable(std::string name);

        void PrintVariables();

    private:

        enum class VarKind {
            STRING,
            INT,
            DOUBLE
        };

        struct Var {
            VarKind     kind;
            std::string s_val;
            int         i_val;
            double      d_val;
        } ;

        std::map<std::string, Var> variables_;
};

}

#endif