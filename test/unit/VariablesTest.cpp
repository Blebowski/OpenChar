
#include <cassert>
#include <filesystem>

#include "Variables.h"

using namespace open_char;

void run_directory_test(Variables *vars)
{
    std::filesystem::path curr_d = std::filesystem::current_path();
    assert(vars->GetVariable("run_directory") == curr_d.c_str());

    std::string rd = "other/nested/directory";
    vars->SetVariable("run_directory", rd);
    assert(vars->GetVariable("run_directory") == "other/nested/directory");
}

int main()
{
    Variables *vars = new Variables();

    run_directory_test(vars);

    delete vars;
}