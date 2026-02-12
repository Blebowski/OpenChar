

#include "Utils.h"
#include "Variables.h"

namespace open_char {

Variables::Variables() :
    run_directory_(std::filesystem::current_path().c_str())
{}

std::string Variables::GetRunDirectory()
{
    return run_directory_;
}

void Variables::SetRunDirectory(std::string &run_directory)
{
    run_directory_ = run_directory;
}

}
