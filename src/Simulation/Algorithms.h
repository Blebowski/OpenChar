
#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "open_char.h"
#include "Context.h"

namespace open_char {

class Algorithms {

    public:
        Algorithms(Context *ctx);

        bool GetLogicFunction(Cell &cell);

        Context *ctx_;

    private:
        int ToLogic(double val);
};

}

#endif