#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <bitset>

#include "disassembler.h"

struct ConditionCodes {
	uint8_t z = 1;
	uint8_t s = 1;
	uint8_t p = 1;
	uint8_t cy = 1;
	uint8_t ac = 1;
	uint8_t pad = 3;
};

struct State8080 {
	uint8_t a, b, c, d, e, h, l;
	uint16_t sp, pc;
	uint8_t* memory;
	struct ConditionCodes cc;
	uint8_t int_enable;
};

void UnimplementedInstruction(State8080& state) {
	std::cerr << "Error: Unimplemented Instruction" << std::endl;
	exit(1);
}

int Parity(int num) {
	uint8_t answer = 0;
	while(num != 0) {
		answer += (num & 1);
		num = num >> 1;
	}
	return !(answer & 1);
}

void add(State8080& state, uint8_t num, uint16_t carry) {
	uint16_t answer = (uint16_t) state.a + (uint16_t) num + carry;
	state.cc.z = ((answer & 0xff) == 0);
	state.cc.s = ((answer & 0x80) != 0);
	state.cc.cy = (answer > 0xff);
	state.cc.p = Parity(answer & 0xff);
	state.cc.ac = ((state.a & 0x0f) + (num & 0x0f) + carry) > 0x0f;
	state.a = answer & 0xff;
}

void Emulate8080Op(State8080& state) {
	unsigned char* opcode = &state.memory[state.pc];
	switch(*opcode) {
		case 0x00: //NOP
			break;
		case 0x01: //LXI    B,word
			state.c = opcode[1];
			state.b = opcode[2];
			state.pc += 2;
			break;
		case 0x02: 
			// std::cout << "STAX   B";
			break;
		case 0x03: 
			// std::cout << "INX    B";
			break;
		case 0x04: 
			// std::cout << "INR    B";
			break;
		case 0x05: 
			// std::cout << "DCR    B";
			break;
		case 0x06: 
			// std::cout << "MVI    B,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0x07:
			// std::cout << "RLC";
			break;
		case 0x08: 
			// std::cout << "NOP";
			break;
		case 0x09: 
			// std::cout << "DAD    B";
			break;
		case 0x0a: 
			// std::cout << "LDAX   B";
			break;
		case 0x0b: 
			// std::cout << "DCX    B";
			break;
		case 0x0c: 
			// std::cout << "INR    C";
			break;
		case 0x0d: 
			// std::cout << "DCR    C";
			break;
		case 0x0e: 
			// std::cout << "MVI    C,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0x0f:
			// std::cout << "RRC";
			break;
		case 0x10: 
			// std::cout << "NOP";
			break;
		case 0x11:
			// std::cout << "LXI    D,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0x12: 
			// std::cout << "STAX   D";
			break;
		case 0x13: 
			// std::cout << "INX    D";
			break;
		case 0x14: 
			// std::cout << "INR    D";
			break;
		case 0x15: 
			// std::cout << "DCR    D";
			break;
		case 0x16: 
			// std::cout << "MVI    D,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0x17:
			// std::cout << "RAL";
			break;
		case 0x18: 
			// std::cout << "NOP";
			break;
		case 0x19: 
			// std::cout << "DAD    D";
			break;
		case 0x1a: 
			// std::cout << "LDAX   D";
			break;
		case 0x1b: 
			// std::cout << "DCX    D";
			break;
		case 0x1c: 
			// std::cout << "INR    E";
			break;
		case 0x1d: 
			// std::cout << "DCR    E";
			break;
		case 0x1e: 
			// std::cout << "MVI    E,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0x1f:
			// std::cout << "RAR";
			break;
		case 0x20:
			// std::cout << "RIM";
			break;
		case 0x21:
			// std::cout << "LXI    H,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0x22:
			// std::cout << "SHLD   $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0x23: 
			// std::cout << "INX    H";
			break;
		case 0x24: 
			// std::cout << "INR    H";
			break;
		case 0x25: 
			// std::cout << "DCR    H";
			break;
		case 0x26: 
			// std::cout << "MVI    H,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0x27:
			// std::cout << "DAA";
			break;
		case 0x28: 
			// std::cout << "NOP";
			break;
		case 0x29: 
			// std::cout << "DAD    H";
			break;
		case 0x2a: 
			// std::cout << "LHLD   $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0x2b: 
			// std::cout << "DCX    H";
			break;
		case 0x2c: 
			// std::cout << "INR    L";
			break;
		case 0x2d: 
			// std::cout << "DCR    L";
			break;
		case 0x2e: 
			// std::cout << "MVI    L,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0x2f:
			// std::cout << "CMA";
			break;
		case 0x30:
			// std::cout << "NOP";
			break;
		case 0x31:
			// std::cout << "LXI    SP,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0x32:
			// std::cout << "STA    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0x33: 
			// std::cout << "INX    SP";
			break;
		case 0x34: 
			// std::cout << "INR    M";
			break;
		case 0x35: 
			// std::cout << "DCR    M";
			break;
		case 0x36: 
			// std::cout << "MVI    M,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0x37:
			// std::cout << "STC";
			break;
		case 0x38: 
			// std::cout << "NOP";
			break;
		case 0x39: 
			// std::cout << "DAD    SP";
			break;
		case 0x3a: 
			// std::cout << "LDA    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0x3b: 
			// std::cout << "DCX    SP";
			break;
		case 0x3c: 
			// std::cout << "INR    A";
			break;
		case 0x3d: 
			// std::cout << "DCR    A";
			break;
		case 0x3e: 
			// std::cout << "MVI    A,#$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0x3f:
			// std::cout << "CMC";
			break;
		case 0x40:
			// std::cout << "MOV    B,B";
			break;
		case 0x41:
			// std::cout << "MOV    B,C";
			break;
		case 0x42:
			// std::cout << "MOV    B,D";
			break;
		case 0x43:
			// std::cout << "MOV    B,E";
			break;
		case 0x44:
			// std::cout << "MOV    B,H";
			break;
		case 0x45:
			// std::cout << "MOV    B,L";
			break;
		case 0x46:
			// std::cout << "MOV    B,M";
			break;
		case 0x47:
			// std::cout << "MOV    B,A";
			break;
		case 0x48:
			// std::cout << "MOV    C,B";
			break;
		case 0x49:
			// std::cout << "MOV    C,C";
			break;
		case 0x4a:
			// std::cout << "MOV    C,D";
			break;
		case 0x4b:
			// std::cout << "MOV    C,E";
			break;
		case 0x4c:
			// std::cout << "MOV    C,H";
			break;
		case 0x4d:
			// std::cout << "MOV    C,L";
			break;
		case 0x4e:
			// std::cout << "MOV    C,M";
			break;
		case 0x4f:
			// std::cout << "MOV    C,A";
			break;
		case 0x50:
			// std::cout << "MOV    D,B";
			break;
		case 0x51:
			// std::cout << "MOV    D,C";
			break;
		case 0x52:
			// std::cout << "MOV    D,D";
			break;
		case 0x53:
			// std::cout << "MOV    D,E";
			break;
		case 0x54:
			// std::cout << "MOV    D,H";
			break;
		case 0x55:
			// std::cout << "MOV    D,L";
			break;
		case 0x56:
			// std::cout << "MOV    D,M";
			break;
		case 0x57:
			// std::cout << "MOV    D,A";
			break;
		case 0x58:
			// std::cout << "MOV    E,B";
			break;
		case 0x59:
			// std::cout << "MOV    E,C";
			break;
		case 0x5a:
			// std::cout << "MOV    E,D";
			break;
		case 0x5b:
			// std::cout << "MOV    E,E";
			break;
		case 0x5c:
			// std::cout << "MOV    E,H";
			break;
		case 0x5d:
			// std::cout << "MOV    E,L";
			break;
		case 0x5e:
			// std::cout << "MOV    E,M";
			break;
		case 0x5f:
			// std::cout << "MOV    E,A";
			break;
		case 0x60:
			// std::cout << "MOV    H,B";
			break;
		case 0x61:
			// std::cout << "MOV    H,C";
			break;
		case 0x62:
			// std::cout << "MOV    H,D";
			break;
		case 0x63:
			// std::cout << "MOV    H,E";
			break;
		case 0x64:
			// std::cout << "MOV    H,H";
			break;
		case 0x65:
			// std::cout << "MOV    H,L";
			break;
		case 0x66:
			// std::cout << "MOV    H,M";
			break;
		case 0x67:
			// std::cout << "MOV    H,A";
			break;
		case 0x68:
			// std::cout << "MOV    L,B";
			break;
		case 0x69:
			// std::cout << "MOV    L,C";
			break;
		case 0x6a:
			// std::cout << "MOV    L,D";
			break;
		case 0x6b:
			// std::cout << "MOV    L,E";
			break;
		case 0x6c:
			// std::cout << "MOV    L,H";
			break;
		case 0x6d:
			// std::cout << "MOV    L,L";
			break;
		case 0x6e:
			// std::cout << "MOV    L,M";
			break;
		case 0x6f:
			// std::cout << "MOV    L,A";
			break;
		case 0x70:
			// std::cout << "MOV    M,B";
			break;
		case 0x71:
			// std::cout << "MOV    M,C";
			break;
		case 0x72:
			// std::cout << "MOV    M,D";
			break;
		case 0x73:
			// std::cout << "MOV    M,E";
			break;
		case 0x74:
			// std::cout << "MOV    M,H";
			break;
		case 0x75:
			// std::cout << "HLT";
			break;
		case 0x76:
			// std::cout << "MOV    M,M";
			break;
		case 0x77:
			// std::cout << "MOV    M,A";
			break;
		case 0x78:
			// std::cout << "MOV    A,B";
			break;
		case 0x79:
			// std::cout << "MOV    A,C";
			break;
		case 0x7a:
			// std::cout << "MOV    A,D";
			break;
		case 0x7b:
			// std::cout << "MOV    A,E";
			break;
		case 0x7c:
			// std::cout << "MOV    A,H";
			break;
		case 0x7d:
			// std::cout << "MOV    A,L";
			break;
		case 0x7e:
			// std::cout << "MOV    A,M";
			break;
		case 0x7f:
			// std::cout << "MOV    A,A";
			break;
		case 0x80: //ADD    B
			add(state, state.b, 0);
			break;
		case 0x81: //ADD    C
			add(state, state.c, 0);
			break;
		case 0x82: //ADD    D
			add(state, state.d, 0);
			break;
		case 0x83: //ADD    E
			add(state, state.e, 0);
			break;
		case 0x84: //ADD    H
			add(state, state.h, 0);
			break;
		case 0x85: //ADD    L
			add(state, state.l, 0);
			break;
		case 0x86: //ADD    M
			add(state, state.memory[(state.h<<8) | (state.l)], 0);
			break;
		case 0x87: //ADD    A
			add(state, state.a, 0);
			break;
		case 0x88: //ADC    B
			add(state, state.b, state.cc.cy);
			break;
		case 0x89: //ADC    C
			add(state, state.c, state.cc.cy);
			break;
		case 0x8a: //ADC    D
			add(state, state.d, state.cc.cy);
			break;
		case 0x8b: //ADC    E
			add(state, state.e, state.cc.cy);
			break;
		case 0x8c: //ADC    H
			add(state, state.h, state.cc.cy);
			break;
		case 0x8d: //ADC    L
			add(state, state.l, state.cc.cy);
			break;
		case 0x8e: //ADC    M
			add(state, state.memory[(state.h<<8) | (state.l)], state.cc.cy);
			break;
		case 0x8f: //ADC    A
			add(state, state.a, state.cc.cy);
			break;
		case 0x90: //SUB    B
			add(state, ~state.b + 1, state.cc.cy);
			break;
		case 0x91: //SUB    C
			add(state, ~state.c + 1, state.cc.cy);
			break;
		case 0x92: //SUB    D
			add(state, ~state.d + 1, state.cc.cy);
			break;
		case 0x93: //SUB    E
			add(state, ~state.e + 1, state.cc.cy);
			break;
		case 0x94: //SUB    H
			add(state, ~state.h + 1, state.cc.cy);
			break;
		case 0x95: //SUB    L
			add(state, ~state.l + 1, state.cc.cy);
			break;
		case 0x96: //SUB    M
			add(state, ~state.memory[(state.h<<8) | (state.l)] + 1, state.cc.cy);
			break;
		case 0x97: //SUB    A
			add(state, ~state.a + 1, state.cc.cy);
			break;
		case 0x98: //SBB    B
			add(state, ~(1+state.b) + 1, state.cc.cy);
			break;
		case 0x99: //SBB    C
			add(state, ~(1+state.c) + 1, state.cc.cy);
			break;
		case 0x9a: //SBB    D
			add(state, ~(1+state.d) + 1, state.cc.cy);
			break;
		case 0x9b: //SBB    E
			add(state, ~(1+state.e) + 1, state.cc.cy);
			break;
		case 0x9c: //SBB    H
			add(state, ~(1+state.h) + 1, state.cc.cy);
			break;
		case 0x9d: //SBB    L
			add(state, ~(1+state.l) + 1, state.cc.cy);
			break;
		case 0x9e: //SBB    M
			add(state, ~(1+state.memory[(state.h<<8) | (state.l)]) + 1, state.cc.cy);
			break;
		case 0x9f: //SBB    A
			add(state, ~(1+state.a) + 1, state.cc.cy);
			break;
		case 0xa0:
			// std::cout << "ANA    B";
			break;
		case 0xa1:
			// std::cout << "ANA    C";
			break;
		case 0xa2:
			// std::cout << "ANA    D";
			break;
		case 0xa3:
			// std::cout << "ANA    E";
			break;
		case 0xa4:
			// std::cout << "ANA    H";
			break;
		case 0xa5:
			// std::cout << "ANA    L";
			break;
		case 0xa6:
			// std::cout << "ANA    M";
			break;
		case 0xa7:
			// std::cout << "ANA    A";
			break;
		case 0xa8:
			// std::cout << "XRA    B";
			break;
		case 0xa9:
			// std::cout << "XRA    C";
			break;
		case 0xaa:
			// std::cout << "XRA    D";
			break;
		case 0xab:
			// std::cout << "XRA    E";
			break;
		case 0xac:
			// std::cout << "XRA    H";
			break;
		case 0xad:
			// std::cout << "XRA    L";
			break;
		case 0xae:
			// std::cout << "XRA    M";
			break;
		case 0xaf:
			// std::cout << "XRA    A";
			break;
		case 0xb0:
			// std::cout << "ORA    B";
			break;
		case 0xb1:
			// std::cout << "ORA    C";
			break;
		case 0xb2:
			// std::cout << "ORA    D";
			break;
		case 0xb3:
			// std::cout << "ORA    E";
			break;
		case 0xb4:
			// std::cout << "ORA    H";
			break;
		case 0xb5:
			// std::cout << "ORA    L";
			break;
		case 0xb6:
			// std::cout << "ORA    M";
			break;
		case 0xb7:
			// std::cout << "ORA    A";
			break;
		case 0xb8:
			// std::cout << "CMP    B";
			break;
		case 0xb9:
			// std::cout << "CMP    C";
			break;
		case 0xba:
			// std::cout << "CMP    D";
			break;
		case 0xbb:
			// std::cout << "CMP    E";
			break;
		case 0xbc:
			// std::cout << "CMP    H";
			break;
		case 0xbd:
			// std::cout << "CMP    L";
			break;
		case 0xbe:
			// std::cout << "CMP    M";
			break;
		case 0xbf:
			// std::cout << "CMP    A";
			break;
		case 0xc0:
			// std::cout << "RNZ";
			break;
		case 0xc1:
			// std::cout << "POP    B";
			break;
		case 0xc2:
			// std::cout << "JNZ    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xc3:
			// std::cout << "JMP    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xc4:
			// std::cout << "CNZ    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xc5:
			// std::cout << "PUSH   B";
			break;
		case 0xc6: //ADI
			add(state, opcode[1], 0);
			break;
		case 0xc7:
			// std::cout << "RST    0";
			break;
		case 0xc8:
			// std::cout << "RZ";
			break;
		case 0xc9:
			// std::cout << "RET";
			break;
		case 0xca:
			// std::cout << "JZ     $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xcb:
			// std::cout << "NOP";
			break;
		case 0xcc:
			// std::cout << "CZ     $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xcd:
			// std::cout << "CALL   $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xce:
			// std::cout << "ACI    " << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xcf:
			// std::cout << "RST    1";
			break;
		case 0xd0:
			// std::cout << "RNC";
			break;
		case 0xd1:
			// std::cout << "POP    D";
			break;
		case 0xd2:
			// std::cout << "JNC    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xd3:
			// std::cout << "OUT    #$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xd4:
			// std::cout << "CNC    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xd5:
			// std::cout << "PUSH   D";
			break;
		case 0xd6: //SUI
			add(state, ~opcode[1] + 1, 0);
			break;
		case 0xd7:
			// std::cout << "RST    2";
			break;
		case 0xd8:
			// std::cout << "RC";
			break;
		case 0xd9:
			// std::cout << "NOP";
			break;
		case 0xda:
			// std::cout << "JC     $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xdb:
			// std::cout << "IN     #$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xdc:
			// std::cout << "CC     $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xdd:
			// std::cout << "NOP";
			break;
		case 0xde:
			// std::cout << "SBI    #$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xdf:
			// std::cout << "RST    3";
			break;
		case 0xe0:
			// std::cout << "RPO";
			break;
		case 0xe1:
			// std::cout << "POP    H";
			break;
		case 0xe2:
			// std::cout << "JPO    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xe3:
			// std::cout << "XTHL";
			break;
		case 0xe4:
			// std::cout << "CPO    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xe5:
			// std::cout << "PUSH   H";
			break;
		case 0xe6:
			// std::cout << "ANI    #$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xe7:
			// std::cout << "RST    4";
			break;
		case 0xe8:
			// std::cout << "RPE";
			break;
		case 0xe9:
			// std::cout << "PCHL";
			break;
		case 0xea:
			// std::cout << "JPE    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xeb:
			// std::cout << "XCHG";
			break;
		case 0xec:
			// std::cout << "CPE    $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xed:
			// std::cout << "NOP";
			break;
		case 0xee:
			// std::cout << "XRI    #$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xef:
			// std::cout << "RST    5";
			break;
		case 0xf0:
			// std::cout << "RP";
			break;
		case 0xf1:
			// std::cout << "POP    PSW";
			break;
		case 0xf2:
			// std::cout << "JP     $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xf3:
			// std::cout << "DI";
			break;
		case 0xf4:
			// std::cout << "CP     $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xf5:
			// std::cout << "PUSH   PSW";
			break;
		case 0xf6:
			// std::cout << "ORI    #$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xf7:
			// std::cout << "RST    6";
			break;
		case 0xf8:
			// std::cout << "RM";
			break;
		case 0xf9:
			// std::cout << "SPHL";
			break;
		case 0xfa:
			// std::cout << "JM     $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xfb:
			// std::cout << "EI";
			break;
		case 0xfc:
			// std::cout << "CM     $" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+2] << std::setw(2) << std::setfill('0') << +codebuffer[pc+1];
			break;
		case 0xfd:
			// std::cout << "NOP";
			break;
		case 0xfe:
			// std::cout << "CPI    #$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xff:
			// std::cout << "RST    7";
			break;
	}
}

void printTest(uint8_t test) {
	std::cout << std::bitset<16>((uint16_t) test) << std::endl;
}

int main(int argc, char* argv[]) {

	uint8_t test = 0x99;
	printTest(test);
	printTest(~(test+1));
	printTest(~(test+1) + 1);
	printTest(test);

	return 0;

}