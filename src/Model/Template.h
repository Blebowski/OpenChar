
#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <vector>
#include <string>

#include "open_char.h"

namespace open_char {

class Template {

    public:
        Template(std::string &name);

        const std::string name_;
        TemplateKind kind_;
        std::vector<double> index_1_;
        std::vector<double> index_2_;

};

}

#endif