#include <iostream>
#include <string>

#include "machineState.h"

int main(int argc, char* argv[]) {

	const std::string fileName = "cpudiag.bin";

	MachineState state(fileName);
	state.printDisassembled();

	// while(!state.isDone()) {
	// 	state.printState();
	// 	while (std::cin.get() != '\n');
	// 	state.processCommand();
	// }

	return 0;

}