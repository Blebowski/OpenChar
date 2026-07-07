set_gnd GND xyz
set_gnd GND 0.0
set_gnd GND 0.05+0.1
set GND_SUFFIX "AAAA"
set_gnd "GND_${GND_SUFFIX}" 0.2
set_gnd GND [expr 1 + 1]

exit
