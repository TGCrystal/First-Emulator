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
			std::string input;
			int numCommands = 1;
			std::getline(std::cin, input);
			if(input.length() > 0 && input[0] >= '0' && input[0] <= '9')
				numCommands = std::stoi(input);
			for(int i = 0; i < numCommands; i++)
				state.processCommand();
		}
	}

	std::cout << "End of memory reached" << std::endl;
	
	return 0;

}