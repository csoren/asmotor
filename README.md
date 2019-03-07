# ASMotor

ASMotor is a portable and generic assembler engine and development system written in ANSI C11 and licensed under the GNU Public License v3. The package consists of the assembler, the librarian and the linker. It can be used as either a cross or native development system.

The assembler syntax is based on the Motorola style macro language.

Currently supported CPUs are the 680x0 family, 6502, MIPS32, Z80, Game Boy, DCPU-16 and CHIP-8/SCHIP.

ASMotor is the spiritual successor to RGBDS, which was a fairly popular development package for the Game Boy. ASMotor is written by the original RGBDS author.

# Installing

## Building from source

### Linux and macOS
A script (```install.sh```) is included that will install the compiled binaries into the $HOME/bin directory. This path should be added to your $PATH for easier use.

Provided you have the necessary prerequisites, ```git``` and ```cmake```, installed, the latest version of ASMotor is easily installed using

```
    git clone --recursive https://github.com/asmotor/asmotor.git && cd asmotor && ./install.sh
```

To install ```git``` and ```cmake```, it is suggested you use [brew](https://brew.sh) or [MacPorts](https://www.macports.org).

# Further reading
Dive into the documentation to learn more about:

* [Introduction](doc/Introduction.md), goals and background
* [Invoking the assembler](doc/Assembler.md) and basic syntax
* [Symbols](doc/Symbol.md) and labels
* [Control structures](doc/ControlStructures.md) like ```INCLUDE```, ```MACRO```'s and conditional assembling.
* [Expressions](doc/Expressions.md) and how they're built
* [Printing diagnostic messages](doc/Diagnostics.md), warnings and errors
* [Organising code](doc/OrganisingCode.md) into sections. How to define data.
* [The linker](doc/Linker.md)

## Index and reference
* [CPU specific](doc/CpuSpecifics.md) details
* [Index of all directives](doc/IndexDirectives.md)
* [Index of all functions](doc/IndexFunctions.md)
* [Operator reference](doc/ReferenceOperators.md)
* [String member reference](doc/ReferenceStringMembers.md)