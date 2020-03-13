#include <iostream>
#include <string>

#include "disassembler.h"
#include "machineState.h"

int main(int argc, char* argv[]) {

	const std::string fileName = "cpudiag.bin";

	// disassembleROM(fileName);

	MachineState state(fileName);

	while(!state.isDone()) {
		state.printState();
		while (std::cin.get() != '\n');
		state.processCommand();
	}

	return 0;

}