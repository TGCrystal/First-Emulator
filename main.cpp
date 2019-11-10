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

void call(State8080& state, unsigned char* opcode, bool condition) {
	if(condition) {
		uint16_t ret = state.pc + 2;
		state.memory[state.sp-1] = (ret >> 8) & 0xff;
		state.memory[state.sp-2] = (ret & 0xff);
		state.sp -= 2;
		state.pc = (opcode[2] << 8) | opcode[1];
	}
	else
		state.pc += 2;
}

void ret(State8080& state, bool condition) {
	if(condition) {
		state.pc = state.memory[state.sp] | (state.memory[state.sp+1] << 8);
		state.sp += 2;
	}
	else
		state.pc += 2;
}

void ana(State8080& state, uint8_t num) {
	state.a = state.a & num;
	state.cc.z = ((answer & 0xff) == 0);
	state.cc.s = ((answer & 0x80) != 0);
	state.cc.cy = 0;
	state.cc.p = Parity(answer);
	state.cc.ac = 0;
}

void xra(State8080& state, uint8_t num) {
	state.a = state.a ^ num;
	state.cc.z = ((answer & 0xff) == 0);
	state.cc.s = ((answer & 0x80) != 0);
	state.cc.cy = 0;
	state.cc.p = Parity(answer);
	state.cc.ac = 0;
}

void ora(State8080& state, uint8_t num) {
	state.a = state.a | num;
	state.cc.z = ((answer & 0xff) == 0);
	state.cc.s = ((answer & 0x80) != 0);
	state.cc.cy = 0;
	state.cc.p = Parity(answer);
	state.cc.ac = 0;
}

void cmp(State8080& state, uint8_t num) {
	uint8_t tmp = ~num;
	uint16_t answer = (uint16_t) state.a + (uint16_t) tmp + 1;
	state.cc.z = ((answer & 0xff) == 0);
	state.cc.s = ((answer & 0x80) != 0);
	state.cc.cy = (num > state.a);
	state.cc.p = Parity(answer & 0xff);
	state.cc.ac = ((state.a & 0x0f) + (tmp & 0x0f)) > 0x0f;
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
		case 0x20: //unknown
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
		case 0x30: //NOP
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
		case 0x37: //STC
			state.cc.cy = 1;
			break;
		case 0x38: //NOP
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
		case 0x3f: //CMC
			state.cc.cy = !state.cc.cy;
			break;
		case 0x40: //MOV    B,B
			state.b = state.b;
			break;
		case 0x41: //MOV    B,C
			state.b = state.c;
			break;
		case 0x42: //MOV    B,D
			state.b = state.d;
			break;
		case 0x43: //MOV    B,E
			state.b = state.e;
			break;
		case 0x44: //MOV    B,H
			state.b = state.h;
			break;
		case 0x45: //MOV    B,L
			state.b = state.l;
			break;
		case 0x46: //MOV    B,M
			state.b = state.memory[(state.h<<8) | (state.l)];
			break;
		case 0x47: //MOV    B,A
			state.b = state.a;
			break;
		case 0x48: //MOV    C,B
			state.c = state.b;
			break;
		case 0x49: //MOV    C,C
			state.c = state.c;
			break;
		case 0x4a: //MOV    C,D
			state.c = state.d;
			break;
		case 0x4b: //MOV    C,E
			state.c = state.e;
			break;
		case 0x4c: //MOV    C,H
			state.c = state.h;
			break;
		case 0x4d: //MOV    C,L
			state.c = state.l;
			break;
		case 0x4e: //MOV    C,M
			state.c = state.memory[(state.h<<8) | (state.l)];
			break;
		case 0x4f: //MOV    C,A
			state.c = state.a;
			break;
		case 0x50: //MOV    D,B
			state.d = state.b;
			break;
		case 0x51: //MOV    D,C
			state.d = state.c;
			break;
		case 0x52: //MOV    D,D
			state.d = state.d;
			break;
		case 0x53: //MOV    D,E
			state.d = state.e;
			break;
		case 0x54: //MOV    D,H
			state.d = state.h;
			break;
		case 0x55: //MOV    D,L
			state.d = state.l;
			break;
		case 0x56: //MOV    D,M
			state.d = state.memory[(state.h<<8) | (state.l)];
			break;
		case 0x57: //MOV    D,A
			state.d = state.a;
			break;
		case 0x58: //MOV    E,B
			state.e = state.b;
			break;
		case 0x59: //MOV    E,C
			state.e = state.c;
			break;
		case 0x5a: //MOV    E,D
			state.e = state.d;
			break;
		case 0x5b: //MOV    E,E
			state.e = state.e;
			break;
		case 0x5c: //MOV    E,H
			state.e = state.h;
			break;
		case 0x5d: //MOV    E,L
			state.e = state.l;
			break;
		case 0x5e: //MOV    E,M
			state.e = state.memory[(state.h<<8) | (state.l)];
			break;
		case 0x5f: //MOV    E,A
			state.e = state.a;
			break;
		case 0x60: //MOV    H,B
			state.h = state.b;
			break;
		case 0x61: //MOV    H,C
			state.h = state.c;
			break;
		case 0x62: //MOV    H,D
			state.h = state.d;
			break;
		case 0x63: //MOV    H,E
			state.h = state.e;
			break;
		case 0x64: //MOV    H,H
			state.h = state.h;
			break;
		case 0x65: //MOV    H,L
			state.h = state.l;
			break;
		case 0x66: //MOV    H,M
			state.h = state.memory[(state.h<<8) | (state.l)];
			break;
		case 0x67: //MOV    H,A
			state.h = state.a;
			break;
		case 0x68: //MOV    L,B
			state.l = state.b;
			break;
		case 0x69: //MOV    L,C
			state.l = state.c;
			break;
		case 0x6a: //MOV    L,D
			state.l = state.d;
			break;
		case 0x6b: //MOV    L,E
			state.l = state.e;
			break;
		case 0x6c: //MOV    L,H
			state.l = state.h;
			break;
		case 0x6d: //MOV    L,L
			state.l = state.l;
			break;
		case 0x6e: //MOV    L,M
			state.l = state.memory[(state.h<<8) | (state.l)];
			break;
		case 0x6f: //MOV    L,A
			state.l = state.a;
			break;
		case 0x70: //MOV    M,B
			state.memory[(state.h<<8) | (state.l)] = state.b;
			break;
		case 0x71: //MOV    M,C
			state.memory[(state.h<<8) | (state.l)] = state.c;
			break;
		case 0x72: //MOV    M,D
			state.memory[(state.h<<8) | (state.l)] = state.d;
			break;
		case 0x73: //MOV    M,E
			state.memory[(state.h<<8) | (state.l)] = state.e;
			break;
		case 0x74: //MOV    M,H
			state.memory[(state.h<<8) | (state.l)] = state.h;
			break;
		case 0x75: //MOV    M,L
			state.memory[(state.h<<8) | (state.l)] = state.l;
			break;
		case 0x76:
			exit(0);
			break;
		case 0x77: //MOV    M,A
			state.memory[(state.h<<8) | (state.l)] = state.a;
			break;
		case 0x78: //MOV    A,B
			state.a = state.b;
			break;
		case 0x79: //MOV    A,C
			state.a = state.c;
			break;
		case 0x7a: //MOV    A,D
			state.a = state.d;
			break;
		case 0x7b: //MOV    A,E
			state.a = state.e;
			break;
		case 0x7c: //MOV    A,H
			state.a = state.h;
			break;
		case 0x7d: //MOV    A,L
			state.a = state.l;
			break;
		case 0x7e: //MOV    A,M
			state.a = state.memory[(state.h<<8) | (state.l)];
			break;
		case 0x7f: //MOV    A,A
			state.a = state.a;
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
		case 0xa0: //ANA    B
			ana(state, state.b);
			break;
		case 0xa1: //ANA    C
			ana(state, state.c);
			break;
		case 0xa2: //ANA    D
			ana(state, state.d);
			break;
		case 0xa3: //ANA    E
			ana(state, state.e);
			break;
		case 0xa4: //ANA    H
			ana(state, state.h);
			break;
		case 0xa5: //ANA    L
			ana(state, state.l);
			break;
		case 0xa6: //ANA    M
			ana(state, state.memory[(state.h<<8) | (state.l)]);
			break;
		case 0xa7: //ANA    A
			ana(state, state.a);
			break;
		case 0xa8: //XRA    B
			xra(state, state.b);
			break;
		case 0xa9: //XRA    C
			xra(state, state.c);
			break;
		case 0xaa: //XRA    D
			xra(state, state.d);
			break;
		case 0xab: //XRA    E
			xra(state, state.e);
			break;
		case 0xac: //XRA    H
			xra(state, state.h);
			break;
		case 0xad: //XRA    L
			xra(state, state.l);
			break;
		case 0xae: //XRA    M
			xra(state, state.memory[(state.h<<8) | (state.l)]);
			break;
		case 0xaf: //XRA    A
			xra(state, state.a);
			break;
		case 0xb0: //ORA    B
			ora(state, state.b);
			break;
		case 0xb1: //ORA    C
			ora(state, state.c);
			break;
		case 0xb2: //ORA    D
			ora(state, state.d);
			break;
		case 0xb3: //ORA    E
			ora(state, state.e);
			break;
		case 0xb4: //ORA    H
			ora(state, state.h);
			break;
		case 0xb5: //ORA    L
			ora(state, state.l);
			break;
		case 0xb6: //ORA    M
			ora(state, state.memory[(state.h<<8) | (state.l)]);
			break;
		case 0xb7: //ORA    A
			ora(state, state.a);
			break;
		case 0xb8: //CMP    B
			cmp(state, state.b);
			break;
		case 0xb9: //CMP    C
			cmp(state, state.c);
			break;
		case 0xba: //CMP    D
			cmp(state, state.d);
			break;
		case 0xbb: //CMP    E
			cmp(state, state.e);
			break;
		case 0xbc: //CMP    H
			cmp(state, state.h);
			break;
		case 0xbd: //CMP    L
			cmp(state, state.l);
			break;
		case 0xbe: //CMP    M
			cmp(state, state.memory[(state.h<<8) | (state.l)]);
			break;
		case 0xbf: //CMP    A
			cmp(state, state.a);
			break;
		case 0xc0: //RNZ
			ret(state, !state.cc.z);
			break;
		case 0xc1: //POP    B
			state.c = state.memory[state.sp];
			state.b = state.memory[state.sp+1];
			state.sp += 2;
			break;
		case 0xc2: //JNZ
			if(!state.cc.z)
				state.pc = (opcode[2] << 8) | opcode[1];
			else
				state.pc += 2;
			break;
		case 0xc3: //JMP
			state.pc = (opcode[2] << 8) | opcode[1];
			break;
		case 0xc4: //CNZ
			call(state, opcode, !state.cc.z);
			break;
		case 0xc5: //PUSH   B
			state.memory[state.sp-1] = state.b;    
            state.memory[state.sp-2] = state.c;    
            state.sp -= 2;
			break;
		case 0xc6: //ADI
			add(state, opcode[1], 0);
			state++;
			break;
		case 0xc7:
			// std::cout << "RST    0";
			break;
		case 0xc8: //RZ
			ret(state, state.cc.z);
			break;
		case 0xc9: //RET
			ret(state, true);
			break;
		case 0xca: //JZ
			if(state.cc.z)
				state.pc = (opcode[2] << 8) | opcode[1];
			else
				state.pc += 2;
			break;
		case 0xcb: //NOP
			break;
		case 0xcc: //CZ
			call(state, opcode, state.cc.z);
			break;
		case 0xcd: //CALL
			call(state, opcode, true);
			break;
		case 0xce: //ACI
			add(state, opcode[1], state.cc.cy);
			break;
		case 0xcf:
			// std::cout << "RST    1";
			break;
		case 0xd0: //RNC
			ret(state, !state.cc.cy);
			break;
		case 0xd1: //POP    D
			state.d = state.memory[state.sp];
			state.e = state.memory[state.sp+1];
			state.sp += 2;
			break;
		case 0xd2: //JNC
			if(!state.cc.cy)
				state.pc = (opcode[2] << 8) | opcode[1];
			else
				state.pc += 2;
			break;
		case 0xd3: //OUT
			state.pc++;
			break;
		case 0xd4: //CNC
			call(state, opcode, !state.cc.cy);
			break;
		case 0xd5: //PUSH   D
			state.memory[state.sp-1] = state.d;    
            state.memory[state.sp-2] = state.e;    
            state.sp -= 2;
			break;
		case 0xd6: //SUI
			add(state, ~opcode[1] + 1, 0);
			state.pc++;
			break;
		case 0xd7:
			// std::cout << "RST    2";
			break;
		case 0xd8: //RC
			ret(state, state.cc.cy);
			break;
		case 0xd9: //NOP
			break;
		case 0xda: //JC
			if(state.cc.cy)
				state.pc = (opcode[2] << 8) | opcode[1];
			else
				state.pc += 2;
			break;
		case 0xdb: //IN
			state.pc++;
			break;
		case 0xdc: //CC
			call(state, opcode, state.cc.cy);
			break;
		case 0xdd: //NOP
			break;
		case 0xde: //SBI
			add(state, ~(opcode[1]+1) + 1, 0);
			break;
		case 0xdf:
			// std::cout << "RST    3";
			break;
		case 0xe0: //RPO
			ret(state, !state.cc.p);
			break;
		case 0xe1: //POP    H
			state.h = state.memory[state.sp];
			state.l = state.memory[state.sp+1];
			state.sp += 2;
			break;
		case 0xe2: //JPO
			if(!state.cc.p)
				state.pc = (opcode[2] << 8) | opcode[1];
			else
				state.pc += 2;
			break;
		case 0xe3: //XTHL
			uint8_t tmp = state.l;
			state.l = memory[state.sp];
			memory[state.sp] = state.l;
			break;
		case 0xe4: //CPO
			call(state, opcode, !state.cc.p);
			break;
		case 0xe5: //PUSH   H
			state.memory[state.sp-1] = state.h;    
            state.memory[state.sp-2] = state.l;    
            state.sp -= 2;
			break;
		case 0xe6:
			// std::cout << "ANI    #$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xe7:
			// std::cout << "RST    4";
			break;
		case 0xe8: //RPE
			ret(state, state.cc.p);
			break;
		case 0xe9: //PCHL
			state.pc = (state.h<<8) | (state.l);
			break;
		case 0xea: //JPE
			if(state.cc.p)
				state.pc = (opcode[2] << 8) | opcode[1];
			else
				state.pc += 2;
			break;
		case 0xeb:
			// std::cout << "XCHG";
			break;
		case 0xec: //CPE
			call(state, opcode, state.cc.p);
			break;
		case 0xed: //NOP
			break;
		case 0xee:
			// std::cout << "XRI    #$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xef:
			// std::cout << "RST    5";
			break;
		case 0xf0: //RP
			ret(state, !state.cc.s);
			break;
		case 0xf1: //POP    PSW
			state.a = state.memory[state.sp+1];  
            state.cc.z  = (0x01 == (state.memory[state.sp] & 0x01));    
            state.cc.s  = (0x02 == (state.memory[state.sp] & 0x02));    
            state.cc.p  = (0x04 == (state.memory[state.sp] & 0x04));    
            state.cc.cy = (0x05 == (state.memory[state.sp] & 0x08));    
            state.cc.ac = (0x10 == (state.memory[state.sp] & 0x10));    
            state.sp += 2; 
			break;
		case 0xf2: //JP
			if(!state.cc.s)
				state.pc = (opcode[2] << 8) | opcode[1];
			else
				state.pc += 2;
			break;
		case 0xf3: //DI
			state.int_enable = 0;
			break;
		case 0xf4: //CP
			call(state, opcode, !state.cc.s);
			break;
		case 0xf5: //PUSH   PSW
			state.memory[state.sp-1] = state.a;
            state.memory[state.sp-2] = (state.cc.z |
                            state.cc.s << 1 |
                            state.cc.p << 2 |
                            state.cc.cy << 3 |
                            state.cc.ac << 4 );
            state.sp -= 2;
			break;
		case 0xf6:
			// std::cout << "ORI    #$" << std::setw(2) << std::setfill('0') << std::hex << +codebuffer[pc+1];
			break;
		case 0xf7:
			// std::cout << "RST    6";
			break;
		case 0xf8: //RM
			ret(state, state.cc.s);
			break;
		case 0xf9: //SPHL
			state.sp = (state.h << 8) | state.l;
			break;
		case 0xfa: //JM
			if(state.cc.s)
				state.pc = (opcode[2] << 8) | opcode[1];
			else
				state.pc += 2;
			break;
		case 0xfb: //EI
			state.int_enable = 1;
			break;
		case 0xfc: //CM
			call(state, opcode, state.cc.s);
			break;
		case 0xfd: //NOP
			break;
		case 0xfe: //CPI
			cmp(state, opcode[1]);
			state.pc++;
			break;
		case 0xff:
			// std::cout << "RST    7";
			break;
	}
	state.pc += 1;
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