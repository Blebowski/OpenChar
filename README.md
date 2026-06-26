# OpenChar

OpenChar is an open-source VLSI library characterization tool.

Library characterization is the process of analog simulation
of standard cells, IO cells or memory macros to create set
of views for digital ASIC design.

# Features

- Supports combinatorial cells and flip-flops
- Supports NGSpice as a simulation engine
- Controlled by TCL commands
- Generates Liberty (`.lib`) file and Verilog (`.v`) model

# Documentation

The user manual from latest build can be found at:
[User manual](https://blebowski.github.io/OpenChar/openchar_manual.pdf)

# Building

To build Open-Char you will need folowing dependencies on Ubuntu:
```
sudo apt-get install build-essential cmake tcl tcl-dev \
libreadline-dev tcl-tclreadline tk tk-dev ngspice
```

Then to build with `cmake`:
```
mkdir build
cd build
cmake ..
make -j`nproc`
```

# License

OpenChar is published under GPL v2, see `LICENSE.md`.

# Status

OpenChar is currently work-in progress before any production release.

The internal architecture and interfaces will likely change.
The TCL commands may also change in backwards incompatible way,
but this is less probable.

Altough OpenChar is built into a library that is then linked to the
`openchar` executable, the library is not intended to be embedded
into third-party applications


