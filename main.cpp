#include <string>

#include "disassembler.h"
#include "machineState.h"

int main(int argc, char* argv[]) {

	const std::string fileName = "cpudiag.bin";

	MachineState state(fileName);

	while(!state.isDone()) {
		state.processCommand();
	}

	return 0;

}