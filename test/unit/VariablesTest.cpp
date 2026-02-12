
#include <cassert>
#include <filesystem>

#include "Variables.h"

using namespace open_char;

void run_directory_test(Variables *vars)
{
    std::filesystem::path curr_d = std::filesystem::current_path();
    assert(vars->GetRunDirectory() == curr_d.c_str());

    std::string rd = "other/nested/directory";
    vars->SetRunDirectory(rd);
    assert(vars->GetRunDirectory() == "other/nested/directory");
}

int main()
{
    Variables *vars = new Variables();

    run_directory_test(vars);

    delete vars;
}