
#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <vector>
#include <string>

#include "open_char.h"

namespace open_char {

class Template {

    public:
        Template(std::string name);

        void WriteLiberty(size_t tab, FILE* f);

        void SetKind(TemplKind kind);
        TemplKind GetKind();

        std::string GetName();

        std::vector<double>& GetIndex1();
        std::vector<double>& GetIndex2();

        void AddIndex1(double val);
        void AddIndex2(double val);

    private:
        const std::string name_;
        TemplKind kind_;
        std::vector<double> index_1_;
        std::vector<double> index_2_;

};

}

#endif