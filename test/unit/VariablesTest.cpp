////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenChar - VLSI library characterizer
// Copyright (C) 2026  Ondrej Ille
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see
// <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <filesystem>

#include "Variables.h"
#include "Utils.h"

using namespace open_char;

void run_directory_test(Variables *vars)
{
    std::filesystem::path curr_d = std::filesystem::current_path();
    assert(vars->GetVariable("run_directory") == curr_d.c_str());

    std::string rd = "other/nested/directory";
    vars->SetVariable("run_directory", rd);
    assert(vars->GetVariable("run_directory") == "other/nested/directory");
}

void delay_test(Variables *vars)
{
    std::filesystem::path curr_d = std::filesystem::current_path();
    assert(vars->GetVariable("delay_in_rise")  == "0.5");
    assert(vars->GetVariable("delay_in_fall")  == "0.5");
    assert(vars->GetVariable("delay_out_rise") == "0.5");
    assert(vars->GetVariable("delay_out_fall") == "0.5");

    vars->SetVariable("delay_in_rise",  "0.7");
    vars->SetVariable("delay_in_fall",  "0.8");
    vars->SetVariable("delay_out_rise", "0.9");
    vars->SetVariable("delay_out_fall", "0.95");

    assert(vars->GetVariable("delay_in_rise")  == "0.7");
    assert(vars->GetVariable("delay_in_fall")  == "0.8");
    assert(vars->GetVariable("delay_out_rise") == "0.9");
    assert(vars->GetVariable("delay_out_fall") == "0.95");

    assert(COMPARE_FLOATS(vars->GetDoubleVariable("delay_in_rise") , 0.7 ));
    assert(COMPARE_FLOATS(vars->GetDoubleVariable("delay_in_fall") , 0.8 ));
    assert(COMPARE_FLOATS(vars->GetDoubleVariable("delay_out_rise"), 0.9 ));
    assert(COMPARE_FLOATS(vars->GetDoubleVariable("delay_out_fall"), 0.95));
}

int main()
{
    Variables *vars = new Variables();

    run_directory_test(vars);

    delete vars;
}