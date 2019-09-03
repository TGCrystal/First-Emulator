#ifndef disassembler_h
#define disassembler_h

/*
	*codebuffer is a pointer to the assembly code
	pc is the program counter

	the bytes used be the instruction will be returned
*/
int Disassemble8080Op(unsigned char *codebuffer, int pc);

#endif