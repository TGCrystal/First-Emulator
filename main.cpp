#include <iostream>
#include <string>

#include "machineState.h"

int main(int argc, char* argv[]) {

	if(argc < 2 || argc > 3) {
		std::cerr << "Incorrect number of arguments" << std::endl;
		exit(1);
	}

	const std::string fileName = argv[1];

	MachineState state(fileName);

	if(argc == 3 && (std::string) argv[2] == "-d") {
		state.printDisassembled();
	}

	else {
		while(!state.isDone()) {
			state.printState();
			while (std::cin.get() != '\n');
			state.processCommand();
		}
	}

	return 0;

}