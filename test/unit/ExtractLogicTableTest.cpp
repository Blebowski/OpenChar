
#include "open_char.h"
#include "Context.h"

int main()
{
    using namespace open_char;

    Context ctx;
    ctx.includes_.push_back("../common/buf.cdl");

    Cell c1{"INV"};
    c1.AddPin("Y",   PinDirection::OUT,     PinKind::DATA);
    c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);
    c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);


    return 0;
}