
#include "Template.h"
#include "Utils.h"

namespace open_char {

Template::Template(std::string name) :
    name_(name)
{}

void Template::WriteLiberty(size_t tab, FILE* f)
{
    TAB_FPRINTF(tab, f, "index_1 (\"");

    size_t i = 0;
    for (const auto & v : index_1_) {
        fprintf(f, "%f%s", v, (i < index_1_.size() - 1) ? ", " : "");
        i++;
    }
    fprintf(f, "\")\n");

    TAB_FPRINTF(tab, f, "index_2 (\"");
    i = 0;
    for (const auto & v : index_2_) {
        fprintf(f, "%f%s", v, (i < index_2_.size() - 1) ? ", " : "");
        i++;
    }
    fprintf(f, "\")\n");
}

}
