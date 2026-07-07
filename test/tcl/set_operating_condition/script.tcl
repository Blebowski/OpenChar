set_operating_condition -temp 25 -voltage 1.05
set_operating_condition -name typical -voltage 1.05
set_operating_condition -name typical -temp 25
set_operating_condition -name typical -temp 25 -supply_name VDD -voltage 1.05
set_operating_condition -name typical -temp 25 -supply_name NON_EXISTENT
set_operating_condition -name typical -temp not_a_number -voltage 1.05
set_operating_condition -name typical -temp {"hello"} -voltage 1.05
set_operating_condition -name typical -temp 25 -voltage bad_volt
set_operating_condition -name typical -temp 25 -voltage 0.95+0.1
set_vdd VDD 1.2
set_operating_condition -name best -temp 25 -supply_name VDD

exit
