

#include "Context.h"

namespace open_char {

Context::Context()
{
    algorithms_ = new Algorithms(this);
}

Context::~Context()
{
    delete algorithms_;
}

}