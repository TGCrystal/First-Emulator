#ifndef disassembler_h
#define disassembler_h

#include <string>
#include <vector>


/*
	*codebuffer is a pointer to the assembly code
	pc is the program counter

	the bytes used be the instruction will be returned
*/
int Disassemble8080Op(std::vector<unsigned char> &codebuffer, unsigned int pc);

void disassembleROM(std::string fileName);

#endif