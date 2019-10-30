#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

#include "disassembler.h"


int main(int argc, char* argv[]) {

	// unsigned char A, B, C, D, E, H, L;
	// unsigned char* pc;

	disassembleROM("invaders.h");

	return 0;

}