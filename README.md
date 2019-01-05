# Implementation of Y86 Simulator
This is a very simple implementation of the Y86 Simulator. It has limited pipelining support, and does not handle any data / instruction hazards in the pipeline. <br>
The maximum supported memory is 16KB (memory and data inclusive) <br>

## Description
There are two parts to this: the parser, and the simulator <br>
The parser is responsible to read a ".ys" file, and generate an object (".yo") file. <br>
The simulator program will take the generated ".yo" file as an output, and has options to run the program in several modes:
* Run the full program
* Run the program one (simulated) cycle at a time
* Run the next "n" clock cycles
* Toggle Pipeline mode
* Show the contents of the registers
* Show the pipeline stage registers
* Show the contents of the memory

## Building the code
Run `make` from the trunk folder

## Running the Code
The binaries are placed under the `bin` folder. <br><br>

Specify an assembly file as input to the parser: <br>
`bin/y86-parser samples/addnos.ys` <br>
This generates the "samples/addnos.yo" file <br>

To run the simulator, run `bin/y86-sim` which opens a menu based interface to execute instructions <br>

## Cleanup
Run `make clean` from the trunk folder to remove any existing object files and binaries

