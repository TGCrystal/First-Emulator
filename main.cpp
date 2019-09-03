#include <iostream>
#include <iomanip>


/*
	*codebuffer is a pointer to the assembly code
	pc is the program counter

	the bytes used be the instruction will be returned
*/
int Disassemble8080Op(unsigned char *codebuffer, int pc) {
	unsigned char *code = &codebuffer[pc];
	int opBytes = 1;
	std::cout << std::hex << pc << " ";
	switch(*code)
	{
		case 0x00: 
			std::cout << "NOP";
			break;
		case 0x01:
			std::cout << "LXI    B," << std::hex << code[2] << code[1];
			opBytes = 3;
			break;
		case 0x02: 
			std::cout << "STAX   B";
			break;
		case 0x03: 
			std::cout << "INX    B";
			break;
		case 0x04: 
			std::cout << "INR    B";
			break;
		case 0x05: 
			std::cout << "DCR    B";
			break;
		case 0x06: 
			std::cout << "MVI    B," << std::hex << code[1];
			opBytes = 2;
			break;
		case 0x07:
			std::cout << "RLC";
			break;
		case 0x08: 
			std::cout << "NOP";
			break;
		case 0x09: 
			std::cout << "DAD    B";
			break;
		case 0x0a: 
			std::cout << "LDAX   B";
			break;
		case 0x0b: 
			std::cout << "DCX    B";
			break;
		case 0x0c: 
			std::cout << "INR    C";
			break;
		case 0x0d: 
			std::cout << "DCR    C";
			break;
		case 0x0e: 
			std::cout << "MVI    C," << std::hex << code[1];
			opBytes = 2;
			break;
		case 0x0f:
			std::cout << "RRC";
			break;
		case 0x10: 
			std::cout << "NOP";
			break;
		case 0x11:
			std::cout << "LXI    D," << std::hex << code[2] << code[1];
			opBytes = 3;
			break;
		case 0x12: 
			std::cout << "STAX   D";
			break;
		case 0x13: 
			std::cout << "INX    D";
			break;
		case 0x14: 
			std::cout << "INR    D";
			break;
		case 0x15: 
			std::cout << "DCR    D";
			break;
		case 0x16: 
			std::cout << "MVI    D," << std::hex << code[1];
			opBytes = 2;
			break;
		case 0x17:
			std::cout << "RAL";
			break;
		case 0x18: 
			std::cout << "NOP";
			break;
		case 0x19: 
			std::cout << "DAD    D";
			break;
		case 0x1a: 
			std::cout << "LDAX   D";
			break;
		case 0x1b: 
			std::cout << "DCX    D";
			break;
		case 0x1c: 
			std::cout << "INR    E";
			break;
		case 0x1d: 
			std::cout << "DCR    E";
			break;
		case 0x1e: 
			std::cout << "MVI    E," << std::hex << code[1];
			opBytes = 2;
			break;
		case 0x1f:
			std::cout << "RAR";
			break;
		case 0x20:
			std::cout << "RIM";
			break;
		case 0x21:
			std::cout << "LXI    H," << std::hex << code[2] << code[1];
			opBytes = 3;
			break;
		case 0x22:
			std::cout << "SHLD   " << std::hex << code[2] << code[1];
			opBytes = 3;
			break;
		case 0x23: 
			std::cout << "INX    H";
			break;
		case 0x24: 
			std::cout << "INR    H";
			break;
		case 0x25: 
			std::cout << "DCR    H";
			break;
		case 0x26: 
			std::cout << "MVI    H," << std::hex << code[1];
			opBytes = 2;
			break;
		case 0x27:
			std::cout << "DAA";
			break;
		case 0x28: 
			std::cout << "NOP";
			break;
		case 0x29: 
			std::cout << "DAD    H";
			break;
		case 0x2a: 
			std::cout << "LHLD   " << std::hex << code[2] << code[1];
			opBytes = 3;
			break;
		case 0x2b: 
			std::cout << "DCX    H";
			break;
		case 0x2c: 
			std::cout << "INR    L";
			break;
		case 0x2d: 
			std::cout << "DCR    L";
			break;
		case 0x2e: 
			std::cout << "MVI    L," << std::hex << code[1];
			opBytes = 2;
			break;
		case 0x2f:
			std::cout << "CMA";
			break;
		case 0x30:
			std::cout << "SIM";
			break;
		case 0x31:
			std::cout << "LXI    SP," << std::hex << code[2] << code[1];
			opBytes = 3;
			break;
		case 0x32:
			std::cout << "STA    " << std::hex << code[2] << code[1];
			opBytes = 3;
			break;
		case 0x33: 
			std::cout << "INX    SP";
			break;
		case 0x34: 
			std::cout << "INR    M";
			break;
		case 0x35: 
			std::cout << "DCR    M";
			break;
		case 0x36: 
			std::cout << "MVI    M," << std::hex << code[1];
			opBytes = 2;
			break;
		case 0x37:
			std::cout << "STC";
			break;
		case 0x38: 
			std::cout << "NOP";
			break;
		case 0x39: 
			std::cout << "DAD    SP";
			break;
		case 0x3a: 
			std::cout << "LDA    " << std::hex << code[2] << code[1];
			opBytes = 3;
			break;
		case 0x3b: 
			std::cout << "DCX    SP";
			break;
		case 0x3c: 
			std::cout << "INR    A";
			break;
		case 0x3d: 
			std::cout << "DCR    A";
			break;
		case 0x3e: 
			std::cout << "MVI    A," << std::hex << code[1];
			opBytes = 2;
			break;
		case 0x3f:
			std::cout << "CMC";
			break;
		case 0x40:
			std::cout << "MOV    B,B";
			break;
		case 0x41:
			std::cout << "MOV    B,C";
			break;
		case 0x42:
			std::cout << "MOV    B,D";
			break;
		case 0x43:
			std::cout << "MOV    B,E";
			break;
		case 0x44:
			std::cout << "MOV    B,H";
			break;
		case 0x45:
			std::cout << "MOV    B,L";
			break;
		case 0x46:
			std::cout << "MOV    B,M";
			break;
		case 0x47:
			std::cout << "MOV    B,A";
			break;
		case 0x48:
			std::cout << "MOV    C,B";
			break;
		case 0x49:
			std::cout << "MOV    C,C";
			break;
		case 0x4a:
			std::cout << "MOV    C,D";
			break;
		case 0x4b:
			std::cout << "MOV    C,E";
			break;
		case 0x4c:
			std::cout << "MOV    C,H";
			break;
		case 0x4d:
			std::cout << "MOV    C,L";
			break;
		case 0x4e:
			std::cout << "MOV    C,M";
			break;
		case 0x4f:
			std::cout << "MOV    C,A";
			break;
		case 0x50:
			std::cout << "MOV    D,B";
			break;
		case 0x51:
			std::cout << "MOV    D,C";
			break;
		case 0x52:
			std::cout << "MOV    D,D";
			break;
		case 0x53:
			std::cout << "MOV    D,E";
			break;
		case 0x54:
			std::cout << "MOV    D,H";
			break;
		case 0x55:
			std::cout << "MOV    D,L";
			break;
		case 0x56:
			std::cout << "MOV    D,M";
			break;
		case 0x57:
			std::cout << "MOV    D,A";
			break;
		case 0x58:
			std::cout << "MOV    E,B";
			break;
		case 0x59:
			std::cout << "MOV    E,C";
			break;
		case 0x5a:
			std::cout << "MOV    E,D";
			break;
		case 0x5b:
			std::cout << "MOV    E,E";
			break;
		case 0x5c:
			std::cout << "MOV    E,H";
			break;
		case 0x5d:
			std::cout << "MOV    E,L";
			break;
		case 0x5e:
			std::cout << "MOV    E,M";
			break;
		case 0x5f:
			std::cout << "MOV    E,A";
			break;
		case 0x60:
			std::cout << "MOV    H,B";
			break;
		case 0x61:
			std::cout << "MOV    H,C";
			break;
		case 0x62:
			std::cout << "MOV    H,D";
			break;
		case 0x63:
			std::cout << "MOV    H,E";
			break;
		case 0x64:
			std::cout << "MOV    H,H";
			break;
		case 0x65:
			std::cout << "MOV    H,L";
			break;
		case 0x66:
			std::cout << "MOV    H,M";
			break;
		case 0x67:
			std::cout << "MOV    H,A";
			break;
		case 0x68:
			std::cout << "MOV    L,B";
			break;
		case 0x69:
			std::cout << "MOV    L,C";
			break;
		case 0x6a:
			std::cout << "MOV    L,D";
			break;
		case 0x6b:
			std::cout << "MOV    L,E";
			break;
		case 0x6c:
			std::cout << "MOV    L,H";
			break;
		case 0x6d:
			std::cout << "MOV    L,L";
			break;
		case 0x6e:
			std::cout << "MOV    L,M";
			break;
		case 0x6f:
			std::cout << "MOV    L,A";
			break;
	}

	std::cout << "\n";

	return opBytes;
}

int main(int argc, char* argv[]) {

	// unsigned char A, B, C, D, E, H, L;
	// unsigned char* pc;

  return 0;

}