#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "machineState.h"

bool isascii(const std::string& fileName) {
	//Thanks to https://stackoverflow.com/questions/277521/how-to-identify-the-file-content-as-ascii-or-binary
	std::ifstream input;
	input.open(fileName, std::ios::in);
	if(!input) {
		std::cerr << "File " << fileName << " not found" << std::endl;
		exit(1);
	}
	int i;
	while((i = input.get()) != EOF && i <= 127) {}
	input.clear();
	input.close();
	return i == EOF;
}

int Parity(int num) {
	uint8_t answer = 0;
	while(num != 0) {
		answer += (num & 1);
		num = num >> 1;
	}
	return !(answer & 1);
}

MachineState::MachineState(const std::string& fileName) {

	std::ifstream input;

	if(isascii(fileName)) {
		std::vector<unsigned char> asciiConverter;
		input.open(fileName, std::ios::in);
		std::string tmp;
		bool firstChar = true;
		while(input >> tmp) {
			for(unsigned int i = 0; i < tmp.size(); i++) {
				//tempchar takes care of 0 in ascii != 0 in hex
				uint8_t tempchar = tmp[i];
				if(tempchar < 58) tempchar -= 48;
				else tempchar -= 87;

				if(firstChar) {
					asciiConverter.push_back(tempchar);
					firstChar = false;
				}
				else {
					asciiConverter[asciiConverter.size()-1] = asciiConverter[asciiConverter.size()-1] << 4;
					asciiConverter[asciiConverter.size()-1] += tempchar;
					firstChar = true;
				}
			}
		}
		memorySize = asciiConverter.size();
		memory = new unsigned char[memorySize];
		for(unsigned int i = 0; i < memorySize; i++) {
			memory[i] = asciiConverter[i];
		}
		this->pc = 0;
	}
	else {
		input.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);
		memorySize = input.tellg();
    	input.seekg (0, std::ios::beg);
		char* memorySigned = new char[memorySize];
		memory = new unsigned char[memorySize+256]; //For files where real code starts at 100
    	input.read (memorySigned, memorySize);
    	for(unsigned int i = 0; i < 256; i++) memory[i] = 0;
    	for(unsigned int i = 0; i < memorySize; i++)
    		memory[i+256] = (unsigned char) memorySigned[i];
    	memorySize += 256;
    	delete[] memorySigned;
    	this->pc = 0x100;
	}
	input.close();

	//establish initial values
	this->sp = 0x3ff;
	this->a = 0;
	this->b = 0;
	this->c = 0;
	this->d = 0;
	this->e = 0;
	this->h = 0;
	this->l = 0;
}

MachineState::~MachineState() {
	delete[] memory;
}

void MachineState::printState() const {
	std::cout << "pc,sp: " << std::hex << std::setw(4) << std::setfill('0') << +this->pc << "," << +this->sp << "\n";
	std::cout << "a\tb c\td e\th l\n";
	std::cout << std::setw(2) << std::setfill('0') << (int) this->a << "\t";
	std::cout << std::setw(2) << std::setfill('0') << (int) this->b;
	std::cout << std::setw(2) << std::setfill('0') << (int) this->c << "\t";
	std::cout << std::setw(2) << std::setfill('0') << (int) this->d;
	std::cout << std::setw(2) << std::setfill('0') << (int) this->e << "\t";
	std::cout << std::setw(2) << std::setfill('0') << (int) this->h;
	std::cout << std::setw(2) << std::setfill('0') << (int) this->l << "\n";
	std::cout << "z,s,p,cy,ac: " << +this->cc[0] << "," << +this->cc[1] << "," << +this->cc[2] << ","
								<< +this->cc[3] << "," << +this->cc[4] << "\n";
	std::cout << "Next Instruction: ";
	this->getOpcode(this->pc);
	std::cout << "                  ";
	this->getOpcodeDescription(this->pc);
	std::cout << std::endl;
}

void MachineState::printDisassembled() const {
	for(uint16_t i = 0; i < memorySize; ) {
		i += this->getOpcode(i);
	}
}

void MachineState::processCommand() {
	uint8_t temp8;
	uint16_t temp16;
	switch(this->memory[this->pc]) {
		case 0x00: //NOP
			break;
		case 0x01: //LXI    B,word
			this->c = this->memory[this->pc+1];
			this->b = this->memory[this->pc+2];
			this->pc += 2; break;
		case 0x02: //STAX   B
			this->memory[(this->b<<8) | (this->c)] = this->a; break;
		case 0x03: //INX    B
			temp16 = (this->b<<8) | (this->c);
			temp16++;
			this->b = temp16 >> 8;
			this->c = temp16 & 0xff; break;
		case 0x04: //INR    B
			this->cc[4] = ((this->b & 0x0f) + 1) > 0x0f;
			this->b++;
			this->cc[0] = ((this->b & 0xff) == 0);
			this->cc[1] = ((this->b & 0x80) != 0);
			this->cc[2] = Parity(this->b); break;
		case 0x05: //DCR    B
			this->cc[4] = ((this->b & 0x0f) + 0x0f) > 0x0f;
			this->b--;
			this->cc[0] = ((this->b & 0xff) == 0);
			this->cc[1] = ((this->b & 0x80) != 0);
			this->cc[2] = Parity(this->b); break;
		case 0x06: //MVI    B
			this->b = this->memory[this->pc+1];
			this->pc++; break;
		case 0x07: //RLC
			this->cc[3] = (this->a & 0x80) >> 7;
			this->a = (this->a<<1) | this->cc[3]; break;
		case 0x08: //NOP
			break;
		case 0x09: //DAD    B
			dad((this->b<<8)|this->c); break;
		case 0x0a: //LDAX   B
			this->a = this->memory[(this->b<<8) | (this->c)]; break;
		case 0x0b: //DCX    B
			temp16 = (this->b<<8) | this->c;
			temp16--;
			this->b = temp16>>8;
			this->c = temp16&0xff; break;
		case 0x0c: //INR    C
			this->cc[4] = ((this->c & 0x0f) + 1) > 0x0f;
			this->c++;
			this->cc[0] = ((this->c & 0xff) == 0);
			this->cc[1] = ((this->c & 0x80) != 0);
			this->cc[2] = Parity(this->c); break;
		case 0x0d: //DCR    C
			this->cc[4] = ((this->c & 0x0f) + 0x0f) > 0x0f;
			this->c--;
			this->cc[0] = ((this->c & 0xff) == 0);
			this->cc[1] = ((this->c & 0x80) != 0);
			this->cc[2] = Parity(this->c); break;
		case 0x0e: //MVI    C
			this->c = this->memory[this->pc+1];
			this->pc++; break;
		case 0x0f: //RRC
			this->cc[3] = this->a & 0x01;
			this->a = (this->a>>1) | (this->cc[3]<<7); break;
		case 0x10: //NOP
			break;
		case 0x11: //LXI    D,word
			this->e = this->memory[this->pc+1];
			this->d = this->memory[this->pc+2];
			this->pc += 2; break;
		case 0x12:  //STAX   D
			this->memory[(this->d<<8) | (this->e)] = this->a; break;
		case 0x13: //INX    D
			temp16 = (this->d<<8) | (this->e);
			temp16++;
			this->d = temp16 >> 8;
			this->e = temp16 & 0xff; break;
		case 0x14: //INR    D
			this->cc[4] = ((this->d & 0x0f) + 1) > 0x0f;
			this->d++;
			this->cc[0] = ((this->d & 0xff) == 0);
			this->cc[1] = ((this->d & 0x80) != 0);
			this->cc[2] = Parity(this->d); break;
		case 0x15: //DCR    D
			this->cc[4] = ((this->d & 0x0f) + 0x0f) > 0x0f;
			this->d--;
			this->cc[0] = ((this->d & 0xff) == 0);
			this->cc[1] = ((this->d & 0x80) != 0);
			this->cc[2] = Parity(this->d); break;
		case 0x16: //MVI    D
			this->d = this->memory[this->pc+1];
			this->pc++; break;
		case 0x17: //RAL
			temp8 = this->cc[3];
			this->cc[3] = (this->a & 0x80) >> 7;
			this->a = (this->a<<1) | temp8; break;
		case 0x18: //NOP
			break;
		case 0x19: //DAD    D
			dad((this->d<<8)|this->e); break;
		case 0x1a: //LDAX   D
			this->a = this->memory[(this->d<<8) | (this->e)]; break;
		case 0x1b: //DCX    D
			temp16 = (this->d<<8) | this->e;
			temp16--;
			this->d = temp16>>8;
			this->e = temp16&0xff; break;
		case 0x1c: //INR    E
			this->cc[4] = ((this->e & 0x0f) + 1) > 0x0f;
			this->e++;
			this->cc[0] = ((this->e & 0xff) == 0);
			this->cc[1] = ((this->e & 0x80) != 0);
			this->cc[2] = Parity(this->e); break;
		case 0x1d: //DCR    E
			this->cc[4] = ((this->e & 0x0f) + 0x0f) > 0x0f;
			this->e--;
			this->cc[0] = ((this->e & 0xff) == 0);
			this->cc[1] = ((this->e & 0x80) != 0);
			this->cc[2] = Parity(this->e); break;
		case 0x1e: //MVI    E
			this->e = this->memory[this->pc+1];
			this->pc++; break;
		case 0x1f: //RAR
			temp8 = this->cc[3];
			this->cc[3] = this->a & 0x01;
			this->a = (this->a>>1) | (temp8<<7); break;
		case 0x20: //NOP
			break;
		case 0x21: //LXI    H,word
			this->l = this->memory[this->pc+1];
			this->h = this->memory[this->pc+2];
			this->pc += 2; break;
		case 0x22: //SHLD
			temp16 = (this->memory[this->pc+2]<<8) | this->memory[this->pc+1];
			this->memory[temp16] = this->l;
			this->memory[temp16+1] = this->h;
			this->pc += 2; break;
		case 0x23: //INX    H
			temp16 = (this->h<<8) | (this->l);
			temp16++;
			this->h = temp16 >> 8;
			this->l = temp16 & 0xff; break;
		case 0x24: //INR    H
			this->cc[4] = ((this->h & 0x0f) + 1) > 0x0f;
			this->h++;
			this->cc[0] = ((this->h & 0xff) == 0);
			this->cc[1] = ((this->h & 0x80) != 0);
			this->cc[2] = Parity(this->h); break;
		case 0x25: //DCR    H
			this->cc[4] = ((this->h & 0x0f) + 0x0f) > 0x0f;
			this->h--;
			this->cc[0] = ((this->h & 0xff) == 0);
			this->cc[1] = ((this->h & 0x80) != 0);
			this->cc[2] = Parity(this->h); break;
		case 0x26: //MVI    H
			this->h = this->memory[this->pc+1];
			this->pc++; break;
		case 0x27: //DAA
			if(this->cc[4] || ((this->a & 0x0f) > 9)) {
				temp8 = (this->a & 0x0f) + 6;
				this->cc[4] = (temp8 > 0x0f);
				this->a += 6;
			}
			if(this->cc[3] || (this->a>>4) > 9) {
				temp8 = (this->a>>4) + 6;
				this->cc[3] = (temp8 > 0x0f);
				this->a = (this->a&0x0f) | (temp8<<4);
			}
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a); break;
		case 0x28: //NOP
			break;
		case 0x29: //DAD    H
			dad((this->h<<8)|this->l); break;
		case 0x2a: //LHLD
			temp16 = (this->memory[this->pc+2]<<8) | this->memory[this->pc+1];
			this->l = this->memory[temp16];
			this->h = this->memory[temp16+1];
			this->pc += 2; break;
		case 0x2b: //DCX    H
			temp16 = (this->h<<8) | this->l;
			temp16--;
			this->h = temp16>>8;
			this->l = temp16&0xff; break;
		case 0x2c:  //INR    L
			this->cc[4] = ((this->l & 0x0f) + 1) > 0x0f;
			this->l++;
			this->cc[0] = ((this->l & 0xff) == 0);
			this->cc[1] = ((this->l & 0x80) != 0);
			this->cc[2] = Parity(this->l); break;
		case 0x2d: //DCR    L
			this->cc[4] = ((this->l & 0x0f) + 0x0f) > 0x0f;
			this->l--;
			this->cc[0] = ((this->l & 0xff) == 0);
			this->cc[1] = ((this->l & 0x80) != 0);
			this->cc[2] = Parity(this->l); break;
		case 0x2e: //MVI    L
			this->l = this->memory[this->pc+1];
			this->pc++; break;
		case 0x2f: //CMA
			this->a = ~this->a; break;
		case 0x30: //NOP
			break;
		case 0x31: //LXI    SP,word
			this->sp = (this->memory[this->pc+2]<<8) | this->memory[this->pc+1];
			this->pc += 2; break;
		case 0x32: //STA
			this->memory[this->memory[this->pc+2]<<8 | this->memory[this->pc+1]] = this->a;
			this->pc += 2; break;
		case 0x33:  //INX    SP
			this->sp++; break;
		case 0x34:  //INR    M
			this->cc[4] = ((this->memory[(this->h<<8) | (this->l)] & 0x0f) + 1) > 0x0f;
			this->memory[(this->h<<8) | (this->l)]++;
			this->cc[0] = ((this->memory[(this->h<<8) | (this->l)] & 0xff) == 0);
			this->cc[1] = ((this->memory[(this->h<<8) | (this->l)] & 0x80) != 0);
			this->cc[2] = Parity(this->memory[(this->h<<8) | (this->l)]); break;
		case 0x35: //DCR    M
			this->cc[4] = ((this->memory[(this->h<<8) | (this->l)] & 0x0f) + 0x0f) > 0x0f;
			this->memory[(this->h<<8) | (this->l)]--;
			this->cc[0] = ((this->memory[(this->h<<8) | (this->l)] & 0xff) == 0);
			this->cc[1] = ((this->memory[(this->h<<8) | (this->l)] & 0x80) != 0);
			this->cc[2] = Parity(this->memory[(this->h<<8) | (this->l)]); break;
		case 0x36: //MVI    M
			this->memory[(this->h<<8) | (this->l)] = this->memory[this->pc+1];
			this->pc++; break;
		case 0x37: //STC
			this->cc[3] = 1; break;
		case 0x38: //NOP
			break;
		case 0x39: //DAD    SP
			dad(this->sp); break;
		case 0x3a: //LDA
			this->a = this->memory[this->memory[this->pc+2]<<8 | this->memory[this->pc+1]];
			this->pc += 2; break;
		case 0x3b: //DCX    SP
			this->sp--; break;
		case 0x3c: //INR    A
			this->cc[4] = ((this->a & 0x0f) + 1) > 0x0f;
			this->a++;
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a); break;
		case 0x3d: //DCR    A
			this->cc[4] = ((this->a & 0x0f) + 0x0f) > 0x0f;
			this->a--;
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a); break;
		case 0x3e: //MVI    A
			this->a = this->memory[this->pc+1];
			this->pc++; break;
		case 0x3f: //CMC
			this->cc[3] = !this->cc[3]; break;
		case 0x40: //MOV    B,B
			this->b = this->b; break;
		case 0x41: //MOV    B,C
			this->b = this->c; break;
		case 0x42: //MOV    B,D
			this->b = this->d; break;
		case 0x43: //MOV    B,E
			this->b = this->e; break;
		case 0x44: //MOV    B,H
			this->b = this->h; break;
		case 0x45: //MOV    B,L
			this->b = this->l; break;
		case 0x46: //MOV    B,M
			this->b = this->memory[(this->h<<8) | (this->l)]; break;
		case 0x47: //MOV    B,A
			this->b = this->a; break;
		case 0x48: //MOV    C,B
			this->c = this->b; break;
		case 0x49: //MOV    C,C
			this->c = this->c; break;
		case 0x4a: //MOV    C,D
			this->c = this->d; break;
		case 0x4b: //MOV    C,E
			this->c = this->e; break;
		case 0x4c: //MOV    C,H
			this->c = this->h; break;
		case 0x4d: //MOV    C,L
			this->c = this->l; break;
		case 0x4e: //MOV    C,M
			this->c = this->memory[(this->h<<8) | (this->l)]; break;
		case 0x4f: //MOV    C,A
			this->c = this->a; break;
		case 0x50: //MOV    D,B
			this->d = this->b; break;
		case 0x51: //MOV    D,C
			this->d = this->c; break;
		case 0x52: //MOV    D,D
			this->d = this->d; break;
		case 0x53: //MOV    D,E
			this->d = this->e; break;
		case 0x54: //MOV    D,H
			this->d = this->h; break;
		case 0x55: //MOV    D,L
			this->d = this->l; break;
		case 0x56: //MOV    D,M
			this->d = this->memory[(this->h<<8) | (this->l)]; break;
		case 0x57: //MOV    D,A
			this->d = this->a; break;
		case 0x58: //MOV    E,B
			this->e = this->b; break;
		case 0x59: //MOV    E,C
			this->e = this->c; break;
		case 0x5a: //MOV    E,D
			this->e = this->d; break;
		case 0x5b: //MOV    E,E
			this->e = this->e; break;
		case 0x5c: //MOV    E,H
			this->e = this->h; break;
		case 0x5d: //MOV    E,L
			this->e = this->l; break;
		case 0x5e: //MOV    E,M
			this->e = this->memory[(this->h<<8) | (this->l)]; break;
		case 0x5f: //MOV    E,A
			this->e = this->a; break;
		case 0x60: //MOV    H,B
			this->h = this->b; break;
		case 0x61: //MOV    H,C
			this->h = this->c; break;
		case 0x62: //MOV    H,D
			this->h = this->d; break;
		case 0x63: //MOV    H,E
			this->h = this->e; break;
		case 0x64: //MOV    H,H
			this->h = this->h; break;
		case 0x65: //MOV    H,L
			this->h = this->l; break;
		case 0x66: //MOV    H,M
			this->h = this->memory[(this->h<<8) | (this->l)]; break;
		case 0x67: //MOV    H,A
			this->h = this->a; break;
		case 0x68: //MOV    L,B
			this->l = this->b; break;
		case 0x69: //MOV    L,C
			this->l = this->c; break;
		case 0x6a: //MOV    L,D
			this->l = this->d; break;
		case 0x6b: //MOV    L,E
			this->l = this->e; break;
		case 0x6c: //MOV    L,H
			this->l = this->h; break;
		case 0x6d: //MOV    L,L
			this->l = this->l; break;
		case 0x6e: //MOV    L,M
			this->l = this->memory[(this->h<<8) | (this->l)]; break;
		case 0x6f: //MOV    L,A
			this->l = this->a; break;
		case 0x70: //MOV    M,B
			this->memory[(this->h<<8) | (this->l)] = this->b; break;
		case 0x71: //MOV    M,C
			this->memory[(this->h<<8) | (this->l)] = this->c; break;
		case 0x72: //MOV    M,D
			this->memory[(this->h<<8) | (this->l)] = this->d; break;
		case 0x73: //MOV    M,E
			this->memory[(this->h<<8) | (this->l)] = this->e; break;
		case 0x74: //MOV    M,H
			this->memory[(this->h<<8) | (this->l)] = this->h; break;
		case 0x75: //MOV    M,L
			this->memory[(this->h<<8) | (this->l)] = this->l; break;
		case 0x76: //HLT
			exit(0); break;
		case 0x77: //MOV    M,A
			this->memory[(this->h<<8) | (this->l)] = this->a; break;
		case 0x78: //MOV    A,B
			this->a = this->b; break;
		case 0x79: //MOV    A,C
			this->a = this->c; break;
		case 0x7a: //MOV    A,D
			this->a = this->d; break;
		case 0x7b: //MOV    A,E
			this->a = this->e; break;
		case 0x7c: //MOV    A,H
			this->a = this->h; break;
		case 0x7d: //MOV    A,L
			this->a = this->l; break;
		case 0x7e: //MOV    A,M
			this->a = this->memory[(this->h<<8) | (this->l)]; break;
		case 0x7f: //MOV    A,A
			this->a = this->a; break;
		case 0x80: //ADD    B
			add(this->b, 0); break;
		case 0x81: //ADD    C
			add(this->c, 0); break;
		case 0x82: //ADD    D
			add(this->d, 0); break;
		case 0x83: //ADD    E
			add(this->e, 0); break;
		case 0x84: //ADD    H
			add(this->h, 0); break;
		case 0x85: //ADD    L
			add(this->l, 0); break;
		case 0x86: //ADD    M
			add(this->memory[(this->h<<8) | (this->l)], 0); break;
		case 0x87: //ADD    A
			add(this->a, 0); break;
		case 0x88: //ADC    B
			add(this->b, this->cc[3]); break;
		case 0x89: //ADC    C
			add(this->c, this->cc[3]); break;
		case 0x8a: //ADC    D
			add(this->d, this->cc[3]); break;
		case 0x8b: //ADC    E
			add(this->e, this->cc[3]); break;
		case 0x8c: //ADC    H
			add(this->h, this->cc[3]); break;
		case 0x8d: //ADC    L
			add(this->l, this->cc[3]); break;
		case 0x8e: //ADC    M
			add(this->memory[(this->h<<8) | (this->l)], this->cc[3]); break;
		case 0x8f: //ADC    A
			add(this->a, this->cc[3]); break;
		case 0x90: //SUB    B
			sub(this->b, 0); break;
		case 0x91: //SUB    C
			sub(this->c, 0); break;
		case 0x92: //SUB    D
			sub(this->d, 0); break;
		case 0x93: //SUB    E
			sub(this->e, 0); break;
		case 0x94: //SUB    H
			sub(this->h, 0); break;
		case 0x95: //SUB    L
			sub(this->l, 0); break;
		case 0x96: //SUB    M
			sub(this->memory[(this->h<<8) | (this->l)], 0); break;
		case 0x97: //SUB    A
			sub(this->a, 0); break;
		case 0x98: //SBB    B
			sub(this->b, this->cc[3]); break;
		case 0x99: //SBB    C
			sub(this->c, this->cc[3]); break;
		case 0x9a: //SBB    D
			sub(this->d, this->cc[3]); break;
		case 0x9b: //SBB    E
			sub(this->e, this->cc[3]); break;
		case 0x9c: //SBB    H
			sub(this->h, this->cc[3]); break;
		case 0x9d: //SBB    L
			sub(this->l, this->cc[3]); break;
		case 0x9e: //SBB    M
			sub(this->memory[(this->h<<8) | (this->l)], this->cc[3]); break;
		case 0x9f: //SBB    A
			sub(this->a, this->cc[3]); break;
		case 0xa0: //ANA    B
			ana(this->b); break;
		case 0xa1: //ANA    C
			ana(this->c); break;
		case 0xa2: //ANA    D
			ana(this->d); break;
		case 0xa3: //ANA    E
			ana(this->e); break;
		case 0xa4: //ANA    H
			ana(this->h); break;
		case 0xa5: //ANA    L
			ana(this->l); break;
		case 0xa6: //ANA    M
			ana(this->memory[(this->h<<8) | (this->l)]); break;
		case 0xa7: //ANA    A
			ana(this->a); break;
		case 0xa8: //XRA    B
			xra(this->b); break;
		case 0xa9: //XRA    C
			xra(this->c); break;
		case 0xaa: //XRA    D
			xra(this->d); break;
		case 0xab: //XRA    E
			xra(this->e); break;
		case 0xac: //XRA    H
			xra(this->h); break;
		case 0xad: //XRA    L
			xra(this->l); break;
		case 0xae: //XRA    M
			xra(this->memory[(this->h<<8) | (this->l)]); break;
		case 0xaf: //XRA    A
			xra(this->a); break;
		case 0xb0: //ORA    B
			ora(this->b); break;
		case 0xb1: //ORA    C
			ora(this->c); break;
		case 0xb2: //ORA    D
			ora(this->d); break;
		case 0xb3: //ORA    E
			ora(this->e); break;
		case 0xb4: //ORA    H
			ora(this->h); break;
		case 0xb5: //ORA    L
			ora(this->l); break;
		case 0xb6: //ORA    M
			ora(this->memory[(this->h<<8) | (this->l)]); break;
		case 0xb7: //ORA    A
			ora(this->a); break;
		case 0xb8: //CMP    B
			cmp(this->b); break;
		case 0xb9: //CMP    C
			cmp(this->c); break;
		case 0xba: //CMP    D
			cmp(this->d); break;
		case 0xbb: //CMP    E
			cmp(this->e); break;
		case 0xbc: //CMP    H
			cmp(this->h); break;
		case 0xbd: //CMP    L
			cmp(this->l); break;
		case 0xbe: //CMP    M
			cmp(this->memory[(this->h<<8) | (this->l)]); break;
		case 0xbf: //CMP    A
			cmp(this->a); break;
		case 0xc0: //RNZ
			ret(!this->cc[0]); break;
		case 0xc1: //POP    B
			this->b = this->memory[this->sp+1];
			this->c = this->memory[this->sp];
			this->sp += 2; break;
		case 0xc2: //JNZ
			if(!this->cc[0])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xc3: //JMP
			this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1; break;
		case 0xc4: //CNZ
			call(!this->cc[0]); break;
		case 0xc5: //PUSH   B
			this->memory[this->sp-1] = this->b;    
            this->memory[this->sp-2] = this->c;    
            this->sp -= 2; break;
		case 0xc6: //ADI
			add(this->memory[this->pc+1], 0);
			this->pc++; break;
		case 0xc7: //RST    0
			rst(0); break;
		case 0xc8: //RZ
			ret(this->cc[0]); break;
		case 0xc9: //RET
			ret(true); break;
		case 0xca: //JZ
			if(this->cc[0])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xcb: //NOP
			break;
		case 0xcc: //CZ
			call(this->cc[0]); break;
		case 0xcd: //CALL
			// if (5 ==  ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]))    
   //          {    
   //              if (this->c == 9)    
   //              {    
   //                  uint16_t offset = (this->d<<8) | (this->e);    
   //                  char *str = (char*) &this->memory[offset+3];  //skip the prefix bytes    
   //                  while (*str != '$')    
   //                      printf("%c", *str++);    
   //                  printf("\n");    
   //              }    
   //              else if (this->c == 2)    
   //              {    
   //                  //saw this in the inspected code, never saw it called    
   //                  printf ("print char routine called\n");    
   //              }    
   //          }    
   //          else if (0 ==  ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]))    
   //          {    
   //              exit(0);    
   //          }    
   //          else    
				call(true);
			break;
		case 0xce: //ACI
			add(this->memory[this->pc+1], this->cc[3]);
			this->pc++; break;
		case 0xcf: //RST    1
			rst(1); break;
		case 0xd0: //RNC
			ret(!this->cc[3]); break;
		case 0xd1: //POP    D
			this->d = this->memory[this->sp+1];
			this->e = this->memory[this->sp];
			this->sp += 2; break;
		case 0xd2: //JNC
			if(!this->cc[3])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xd3: //OUT
			MachineOUT();
			this->pc++; break;
		case 0xd4: //CNC
			call(!this->cc[3]); break;
		case 0xd5: //PUSH   D
			this->memory[this->sp-1] = this->d;    
            this->memory[this->sp-2] = this->e;    
            this->sp -= 2; break;
		case 0xd6: //SUI
			sub(this->memory[this->pc+1], 0);
			this->pc++; break;
		case 0xd7: //RST    2
			rst(2); break;
		case 0xd8: //RC
			ret(this->cc[3]); break;
		case 0xd9: //NOP
			break;
		case 0xda: //JC
			if(this->cc[3])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xdb: //IN
    	    this->a = MachineIN();
    	    this->pc++; break;
		case 0xdc: //CC
			call(this->cc[3]); break;
		case 0xdd: //NOP
			break;
		case 0xde: //SBI
			sub(this->memory[this->pc+1], this->cc[3]);
			this->pc++; break;
		case 0xdf: //RST    3
			rst(3); break;
		case 0xe0: //RPO
			ret(!this->cc[2]); break;
		case 0xe1: //POP    H
			this->h = this->memory[this->sp+1];
			this->l = this->memory[this->sp];
			this->sp += 2; break;
		case 0xe2: //JPO
			if(!this->cc[2])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xe3: //XTHL
			temp8 = this->l;
			this->l = this->memory[this->sp];
			this->memory[this->sp] = temp8;
			temp8 = this->h;
			this->h = this->memory[this->sp+1];
			this->memory[this->sp+1] = temp8; break;
		case 0xe4: //CPO
			call(!this->cc[2]); break;
		case 0xe5: //PUSH   H
			this->memory[this->sp-1] = this->h;    
            this->memory[this->sp-2] = this->l;    
            this->sp -= 2; break;
		case 0xe6: //ANI
			this->a = this->a & this->memory[this->pc+1];
			this->cc[3] = 0;
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a & 0xff);
			this->pc++; break;
		case 0xe7: //RST    4
			rst(4); break;
		case 0xe8: //RPE
			ret(this->cc[2]); break;
		case 0xe9: //PCHL
			this->pc = ((this->h<<8) | (this->l)) - 1; break;
		case 0xea: //JPE
			if(this->cc[2])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xeb: //XCHG
			temp8 = this->d;
			this->d = this->h;
			this->h = temp8;
			temp8 = this->e;
			this->e = this->l;
			this->l = temp8; break;
		case 0xec: //CPE
			call(this->cc[2]); break;
		case 0xed: //NOP
			break;
		case 0xee: //XRI
			this->a = this->a ^ this->memory[this->pc+1];
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a & 0xff);
			this->cc[3] = 0;
			this->pc++; break;
		case 0xef: //RST    5
			rst(5); break;
		case 0xf0: //RP
			ret(!this->cc[1]); break;
		case 0xf1: //POP    PSW
			this->a = this->memory[this->sp+1];
            this->cc[3]  = (01 == (this->memory[this->sp] & 01));
            this->cc[2]  = (04 == (this->memory[this->sp] & 04));
            this->cc[4]  = (16 == (this->memory[this->sp] & 16));
            this->cc[0] = (64 == (this->memory[this->sp] & 64));
            this->cc[1] = (128 == (this->memory[this->sp] & 128));
            this->sp += 2;  break;
		case 0xf2: //JP
			if(!this->cc[1])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xf3: //DI
			this->int_enable = 0; break;
		case 0xf4: //CP
			call(!this->cc[1]); break;
		case 0xf5: //PUSH   PSW
			this->memory[this->sp-1] = this->a;
            this->memory[this->sp-2] = (this->cc[3] | 2 |
                            this->cc[2] << 2 |
                            this->cc[4] << 4 |
                            this->cc[0] << 6 |
                            this->cc[1] << 7 );
            this->sp -= 2; break;
		case 0xf6: //ORI
			this->a = this->a | this->memory[this->pc+1];
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a & 0xff);
			this->cc[3] = 0;
			this->pc++; break;
		case 0xf7: //RST    6
			rst(6); break;
		case 0xf8: //RM
			ret(this->cc[1]); break;
		case 0xf9: //SPHL
			this->sp = (this->h << 8) | this->l; break;
		case 0xfa: //JM
			if(this->cc[1])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xfb: //EI
			this->int_enable = 1; break;
		case 0xfc: //CM
			call(this->cc[1]); break;
		case 0xfd: //NOP
			break;
		case 0xfe: //CPI
			cmp(this->memory[this->pc+1]);
			this->pc++; break;
		case 0xff: //RST    7
			rst(7); break;
	}
	this->pc++;
}

void MachineState::sub(uint8_t num, uint8_t carry) {
	num = ~num;
	carry = ~carry;
	num++;
	carry++;
	uint16_t answer = (uint16_t) this->a + (uint16_t) num + (uint16_t) carry;
	this->cc[0] = ((answer & 0xff) == 0);
	this->cc[1] = ((answer & 0x80) != 0);
	this->cc[2] = Parity(answer & 0xff);
	this->cc[3] = !(answer > 0xff);
	this->cc[4] = ((this->a & 0x0f) + (num & 0x0f) + carry) > 0x0f;
	this->a = answer & 0xff;
}

void MachineState::add(uint8_t num, uint16_t carry) {
	uint16_t answer = (uint16_t) this->a + (uint16_t) num + carry;
	this->cc[0] = ((answer & 0xff) == 0);
	this->cc[1] = ((answer & 0x80) != 0);
	this->cc[2] = Parity(answer & 0xff);
	this->cc[3] = (answer > 0xff);
	this->cc[4] = ((this->a & 0x0f) + (num & 0x0f) + carry) > 0x0f;
	this->a = answer & 0xff;
}

void MachineState::call(bool condition) {
	if(condition) {
		uint16_t ret = (uint16_t) this->pc + 3;
		this->memory[this->sp-1] = (ret >> 8) & 0xff;
		this->memory[this->sp-2] = (ret & 0xff);
		this->sp -= 2;
		this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
	}
	else
		this->pc += 2;
}

void MachineState::ret(bool condition) {
	if(condition) {
		this->pc = (this->memory[this->sp] | (this->memory[this->sp+1] << 8)) - 1;
		this->sp += 2;
	}
}

void MachineState::ana(uint8_t num) {
	this->a = this->a & num;
	this->cc[0] = ((this->a & 0xff) == 0);
	this->cc[1] = ((this->a & 0x80) != 0);
	this->cc[2] = Parity(this->a);
	this->cc[3] = 0;
	this->cc[4] = 0;
}

void MachineState::xra(uint8_t num) {
	this->a = this->a ^ num;
	this->cc[0] = ((this->a & 0xff) == 0);
	this->cc[1] = ((this->a & 0x80) != 0);
	this->cc[2] = Parity(this->a);
	this->cc[3] = 0;
	this->cc[4] = 0;
}

void MachineState::ora(uint8_t num) {
	this->a = this->a | num;
	this->cc[0] = ((this->a & 0xff) == 0);
	this->cc[1] = ((this->a & 0x80) != 0);
	this->cc[2] = Parity(this->a);
	this->cc[3] = 0;
	this->cc[4] = 0;
}

void MachineState::cmp(uint8_t num) {
	uint8_t tmp = ~num;
	tmp++;
	uint16_t answer = (uint16_t) this->a + (uint16_t) tmp;
	this->cc[0] = ((answer & 0xff) == 0);
	this->cc[1] = ((answer & 0x80) != 0);
	this->cc[2] = Parity(answer & 0xff);
	this->cc[3] = (num > this->a);
	this->cc[4] = ((this->a & 0x0f) + (tmp & 0x0f)) > 0x0f;
}

void MachineState::dad(uint16_t num) {
	uint32_t tmp = (this->h<<8 | this->l) + num;
	this->cc[3] = (tmp > 0xffff);
	uint16_t answer = tmp & 0xffff;
	this->h = answer>>8;
	this->l = tmp & 0xff;
}

void MachineState::rst(uint8_t num) {
	this->sp--;
	this->memory[this->sp] = (this->pc&0xff);
	this->sp--;
	this->memory[this->sp] = (this->pc>>4);
}

uint8_t MachineState::MachineIN() {
	uint8_t port = this->memory[this->pc+1];
	uint16_t temp16;
	uint8_t answer;
	switch(port) {
		case 3:
			temp16 = (shift1<<8) | shift0;
			answer = ((temp16 >> (8-shift_offset)) & 0xff);
			break;
	}
	return answer;
}

void MachineState::MachineOUT() { //Example has uint8_t value parameter from somewhere?
	uint8_t port = this->memory[this->pc+1];
	switch(port) {
		case 2:
			shift_offset = value & 0x7;
			break;
		case 4:
			shift0 = shift1;
			shift1 = value;
	}
}

int MachineState::getOpcode(uint16_t index) const {
	unsigned char code = memory[index];
	int opBytes = 1;
	std::cout << std::hex << std::setw(4) << std::setfill('0') << index << " ";
	switch(code)
	{
		case 0x00: 
			std::cout << "NOP"; break;
		case 0x01:
			std::cout << "LXI    B,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0x02: 
			std::cout << "STAX   B"; break;
		case 0x03: 
			std::cout << "INX    B"; break;
		case 0x04: 
			std::cout << "INR    B"; break;
		case 0x05: 
			std::cout << "DCR    B"; break;
		case 0x06: 
			std::cout << "MVI    B,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0x07:
			std::cout << "RLC"; break;
		case 0x08: 
			std::cout << "NOP"; break;
		case 0x09: 
			std::cout << "DAD    B"; break;
		case 0x0a: 
			std::cout << "LDAX   B"; break;
		case 0x0b: 
			std::cout << "DCX    B"; break;
		case 0x0c: 
			std::cout << "INR    C"; break;
		case 0x0d: 
			std::cout << "DCR    C"; break;
		case 0x0e: 
			std::cout << "MVI    C,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0x0f:
			std::cout << "RRC"; break;
		case 0x10: 
			std::cout << "NOP"; break;
		case 0x11:
			std::cout << "LXI    D,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0x12: 
			std::cout << "STAX   D"; break;
		case 0x13: 
			std::cout << "INX    D"; break;
		case 0x14: 
			std::cout << "INR    D"; break;
		case 0x15: 
			std::cout << "DCR    D"; break;
		case 0x16: 
			std::cout << "MVI    D,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0x17:
			std::cout << "RAL"; break;
		case 0x18: 
			std::cout << "NOP"; break;
		case 0x19: 
			std::cout << "DAD    D"; break;
		case 0x1a: 
			std::cout << "LDAX   D"; break;
		case 0x1b: 
			std::cout << "DCX    D"; break;
		case 0x1c: 
			std::cout << "INR    E"; break;
		case 0x1d: 
			std::cout << "DCR    E"; break;
		case 0x1e: 
			std::cout << "MVI    E,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0x1f:
			std::cout << "RAR"; break;
		case 0x20:
			std::cout << "NOP"; break;
		case 0x21:
			std::cout << "LXI    H,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0x22:
			std::cout << "SHLD   $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0x23: 
			std::cout << "INX    H"; break;
		case 0x24: 
			std::cout << "INR    H"; break;
		case 0x25: 
			std::cout << "DCR    H"; break;
		case 0x26: 
			std::cout << "MVI    H,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0x27:
			std::cout << "DAA"; break;
		case 0x28: 
			std::cout << "NOP"; break;
		case 0x29: 
			std::cout << "DAD    H"; break;
		case 0x2a: 
			std::cout << "LHLD   $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0x2b: 
			std::cout << "DCX    H"; break;
		case 0x2c: 
			std::cout << "INR    L"; break;
		case 0x2d: 
			std::cout << "DCR    L"; break;
		case 0x2e: 
			std::cout << "MVI    L,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0x2f:
			std::cout << "CMA"; break;
		case 0x30:
			std::cout << "NOP"; break;
		case 0x31:
			std::cout << "LXI    SP,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0x32:
			std::cout << "STA    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0x33: 
			std::cout << "INX    SP"; break;
		case 0x34: 
			std::cout << "INR    M"; break;
		case 0x35: 
			std::cout << "DCR    M"; break;
		case 0x36: 
			std::cout << "MVI    M,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0x37:
			std::cout << "STC"; break;
		case 0x38: 
			std::cout << "NOP"; break;
		case 0x39: 
			std::cout << "DAD    SP"; break;
		case 0x3a: 
			std::cout << "LDA    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0x3b: 
			std::cout << "DCX    SP"; break;
		case 0x3c: 
			std::cout << "INR    A"; break;
		case 0x3d: 
			std::cout << "DCR    A"; break;
		case 0x3e: 
			std::cout << "MVI    A,#$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0x3f:
			std::cout << "CMC"; break;
		case 0x40:
			std::cout << "MOV    B,B"; break;
		case 0x41:
			std::cout << "MOV    B,C"; break;
		case 0x42:
			std::cout << "MOV    B,D"; break;
		case 0x43:
			std::cout << "MOV    B,E"; break;
		case 0x44:
			std::cout << "MOV    B,H"; break;
		case 0x45:
			std::cout << "MOV    B,L"; break;
		case 0x46:
			std::cout << "MOV    B,M"; break;
		case 0x47:
			std::cout << "MOV    B,A"; break;
		case 0x48:
			std::cout << "MOV    C,B"; break;
		case 0x49:
			std::cout << "MOV    C,C"; break;
		case 0x4a:
			std::cout << "MOV    C,D"; break;
		case 0x4b:
			std::cout << "MOV    C,E"; break;
		case 0x4c:
			std::cout << "MOV    C,H"; break;
		case 0x4d:
			std::cout << "MOV    C,L"; break;
		case 0x4e:
			std::cout << "MOV    C,M"; break;
		case 0x4f:
			std::cout << "MOV    C,A"; break;
		case 0x50:
			std::cout << "MOV    D,B"; break;
		case 0x51:
			std::cout << "MOV    D,C"; break;
		case 0x52:
			std::cout << "MOV    D,D"; break;
		case 0x53:
			std::cout << "MOV    D,E"; break;
		case 0x54:
			std::cout << "MOV    D,H"; break;
		case 0x55:
			std::cout << "MOV    D,L"; break;
		case 0x56:
			std::cout << "MOV    D,M"; break;
		case 0x57:
			std::cout << "MOV    D,A"; break;
		case 0x58:
			std::cout << "MOV    E,B"; break;
		case 0x59:
			std::cout << "MOV    E,C"; break;
		case 0x5a:
			std::cout << "MOV    E,D"; break;
		case 0x5b:
			std::cout << "MOV    E,E"; break;
		case 0x5c:
			std::cout << "MOV    E,H"; break;
		case 0x5d:
			std::cout << "MOV    E,L"; break;
		case 0x5e:
			std::cout << "MOV    E,M"; break;
		case 0x5f:
			std::cout << "MOV    E,A"; break;
		case 0x60:
			std::cout << "MOV    H,B"; break;
		case 0x61:
			std::cout << "MOV    H,C"; break;
		case 0x62:
			std::cout << "MOV    H,D"; break;
		case 0x63:
			std::cout << "MOV    H,E"; break;
		case 0x64:
			std::cout << "MOV    H,H"; break;
		case 0x65:
			std::cout << "MOV    H,L"; break;
		case 0x66:
			std::cout << "MOV    H,M"; break;
		case 0x67:
			std::cout << "MOV    H,A"; break;
		case 0x68:
			std::cout << "MOV    L,B"; break;
		case 0x69:
			std::cout << "MOV    L,C"; break;
		case 0x6a:
			std::cout << "MOV    L,D"; break;
		case 0x6b:
			std::cout << "MOV    L,E"; break;
		case 0x6c:
			std::cout << "MOV    L,H"; break;
		case 0x6d:
			std::cout << "MOV    L,L"; break;
		case 0x6e:
			std::cout << "MOV    L,M"; break;
		case 0x6f:
			std::cout << "MOV    L,A"; break;
		case 0x70:
			std::cout << "MOV    M,B"; break;
		case 0x71:
			std::cout << "MOV    M,C"; break;
		case 0x72:
			std::cout << "MOV    M,D"; break;
		case 0x73:
			std::cout << "MOV    M,E"; break;
		case 0x74:
			std::cout << "MOV    M,H"; break;
		case 0x75:
			std::cout << "MOV    M,L"; break;
		case 0x76:
			std::cout << "HLT"; break;
		case 0x77:
			std::cout << "MOV    M,A"; break;
		case 0x78:
			std::cout << "MOV    A,B"; break;
		case 0x79:
			std::cout << "MOV    A,C"; break;
		case 0x7a:
			std::cout << "MOV    A,D"; break;
		case 0x7b:
			std::cout << "MOV    A,E"; break;
		case 0x7c:
			std::cout << "MOV    A,H"; break;
		case 0x7d:
			std::cout << "MOV    A,L"; break;
		case 0x7e:
			std::cout << "MOV    A,M"; break;
		case 0x7f:
			std::cout << "MOV    A,A"; break;
		case 0x80:
			std::cout << "ADD    B"; break;
		case 0x81:
			std::cout << "ADD    C"; break;
		case 0x82:
			std::cout << "ADD    D"; break;
		case 0x83:
			std::cout << "ADD    E"; break;
		case 0x84:
			std::cout << "ADD    H"; break;
		case 0x85:
			std::cout << "ADD    L"; break;
		case 0x86:
			std::cout << "ADD    M"; break;
		case 0x87:
			std::cout << "ADD    A"; break;
		case 0x88:
			std::cout << "ADC    B"; break;
		case 0x89:
			std::cout << "ADC    C"; break;
		case 0x8a:
			std::cout << "ADC    D"; break;
		case 0x8b:
			std::cout << "ADC    E"; break;
		case 0x8c:
			std::cout << "ADC    H"; break;
		case 0x8d:
			std::cout << "ADC    L"; break;
		case 0x8e:
			std::cout << "ADC    M"; break;
		case 0x8f:
			std::cout << "ADC    A"; break;
		case 0x90:
			std::cout << "SUB    B"; break;
		case 0x91:
			std::cout << "SUB    C"; break;
		case 0x92:
			std::cout << "SUB    D"; break;
		case 0x93:
			std::cout << "SUB    E"; break;
		case 0x94:
			std::cout << "SUB    H"; break;
		case 0x95:
			std::cout << "SUB    L"; break;
		case 0x96:
			std::cout << "SUB    M"; break;
		case 0x97:
			std::cout << "SUB    A"; break;
		case 0x98:
			std::cout << "SBB    B"; break;
		case 0x99:
			std::cout << "SBB    C"; break;
		case 0x9a:
			std::cout << "SBB    D"; break;
		case 0x9b:
			std::cout << "SBB    E"; break;
		case 0x9c:
			std::cout << "SBB    H"; break;
		case 0x9d:
			std::cout << "SBB    L"; break;
		case 0x9e:
			std::cout << "SBB    M"; break;
		case 0x9f:
			std::cout << "SBB    A"; break;
		case 0xa0:
			std::cout << "ANA    B"; break;
		case 0xa1:
			std::cout << "ANA    C"; break;
		case 0xa2:
			std::cout << "ANA    D"; break;
		case 0xa3:
			std::cout << "ANA    E"; break;
		case 0xa4:
			std::cout << "ANA    H"; break;
		case 0xa5:
			std::cout << "ANA    L"; break;
		case 0xa6:
			std::cout << "ANA    M"; break;
		case 0xa7:
			std::cout << "ANA    A"; break;
		case 0xa8:
			std::cout << "XRA    B"; break;
		case 0xa9:
			std::cout << "XRA    C"; break;
		case 0xaa:
			std::cout << "XRA    D"; break;
		case 0xab:
			std::cout << "XRA    E"; break;
		case 0xac:
			std::cout << "XRA    H"; break;
		case 0xad:
			std::cout << "XRA    L"; break;
		case 0xae:
			std::cout << "XRA    M"; break;
		case 0xaf:
			std::cout << "XRA    A"; break;
		case 0xb0:
			std::cout << "ORA    B"; break;
		case 0xb1:
			std::cout << "ORA    C"; break;
		case 0xb2:
			std::cout << "ORA    D"; break;
		case 0xb3:
			std::cout << "ORA    E"; break;
		case 0xb4:
			std::cout << "ORA    H"; break;
		case 0xb5:
			std::cout << "ORA    L"; break;
		case 0xb6:
			std::cout << "ORA    M"; break;
		case 0xb7:
			std::cout << "ORA    A"; break;
		case 0xb8:
			std::cout << "CMP    B"; break;
		case 0xb9:
			std::cout << "CMP    C"; break;
		case 0xba:
			std::cout << "CMP    D"; break;
		case 0xbb:
			std::cout << "CMP    E"; break;
		case 0xbc:
			std::cout << "CMP    H"; break;
		case 0xbd:
			std::cout << "CMP    L"; break;
		case 0xbe:
			std::cout << "CMP    M"; break;
		case 0xbf:
			std::cout << "CMP    A"; break;
		case 0xc0:
			std::cout << "RNZ"; break;
		case 0xc1:
			std::cout << "POP    B"; break;
		case 0xc2:
			std::cout << "JNZ    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xc3:
			std::cout << "JMP    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xc4:
			std::cout << "CNZ    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xc5:
			std::cout << "PUSH   B"; break;
		case 0xc6:
			std::cout << "ADI    #$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0xc7:
			std::cout << "RST    0"; break;
		case 0xc8:
			std::cout << "RZ"; break;
		case 0xc9:
			std::cout << "RET"; break;
		case 0xca:
			std::cout << "JZ     $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xcb:
			std::cout << "NOP"; break;
		case 0xcc:
			std::cout << "CZ     $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xcd:
			std::cout << "CALL   $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xce:
			std::cout << "ACI    " << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0xcf:
			std::cout << "RST    1"; break;
		case 0xd0:
			std::cout << "RNC"; break;
		case 0xd1:
			std::cout << "POP    D"; break;
		case 0xd2:
			std::cout << "JNC    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xd3:
			std::cout << "OUT    #$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0xd4:
			std::cout << "CNC    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xd5:
			std::cout << "PUSH   D"; break;
		case 0xd6:
			std::cout << "SUI    #$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0xd7:
			std::cout << "RST    2"; break;
		case 0xd8:
			std::cout << "RC"; break;
		case 0xd9:
			std::cout << "NOP"; break;
		case 0xda:
			std::cout << "JC     $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xdb:
			std::cout << "IN     #$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0xdc:
			std::cout << "CC     $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xdd:
			std::cout << "NOP"; break;
		case 0xde:
			std::cout << "SBI    #$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0xdf:
			std::cout << "RST    3"; break;
		case 0xe0:
			std::cout << "RPO"; break;
		case 0xe1:
			std::cout << "POP    H"; break;
		case 0xe2:
			std::cout << "JPO    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xe3:
			std::cout << "XTHL"; break;
		case 0xe4:
			std::cout << "CPO    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xe5:
			std::cout << "PUSH   H"; break;
		case 0xe6:
			std::cout << "ANI    #$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0xe7:
			std::cout << "RST    4"; break;
		case 0xe8:
			std::cout << "RPE"; break;
		case 0xe9:
			std::cout << "PCHL"; break;
		case 0xea:
			std::cout << "JPE    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xeb:
			std::cout << "XCHG"; break;
		case 0xec:
			std::cout << "CPE    $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xed:
			std::cout << "NOP"; break;
		case 0xee:
			std::cout << "XRI    #$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0xef:
			std::cout << "RST    5"; break;
		case 0xf0:
			std::cout << "RP"; break;
		case 0xf1:
			std::cout << "POP    PSW"; break;
		case 0xf2:
			std::cout << "JP     $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xf3:
			std::cout << "DI"; break;
		case 0xf4:
			std::cout << "CP     $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xf5:
			std::cout << "PUSH   PSW"; break;
		case 0xf6:
			std::cout << "ORI    #$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0xf7:
			std::cout << "RST    6"; break;
		case 0xf8:
			std::cout << "RM"; break;
		case 0xf9:
			std::cout << "SPHL"; break;
		case 0xfa:
			std::cout << "JM     $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xfb:
			std::cout << "EI"; break;
		case 0xfc:
			std::cout << "CM     $" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2] << std::setw(2) << std::setfill('0') << (int) memory[index+1];
			opBytes = 3; break;
		case 0xfd:
			std::cout << "NOP"; break;
		case 0xfe:
			std::cout << "CPI    #$" << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			opBytes = 2; break;
		case 0xff:
			std::cout << "RST    7"; break;

	}

	std::cout << "\n";

	return opBytes;
}

int MachineState::getOpcodeDescription(uint16_t index) const {
	unsigned char code = memory[index];
	int opBytes = 1;
	std::cout << std::hex << std::setw(4) << std::setfill('0') << index << " ";
	switch(code)
	{
		case 0x01: //LXI B
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2];
			std::cout << " moved into B, ";
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into C";
			opBytes = 3; break;
		case 0x02: //STAX B
			std::cout << "Store A in location specified by BC"; break;
		case 0x03: //INX B
			std::cout << "BC++"; break;
		case 0x04: //INR B
			std::cout << "B++"; break;
		case 0x05: //DCR B
			std::cout << "B--"; break;
		case 0x06: //MVI B
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into B";
			opBytes = 2; break;
		case 0x07: //RLC
			std::cout << "A<<1, shifted off bit placed onto other end"; break;
		case 0x09: //DAD B
			std::cout << "HL += BC"; break;
		case 0x0a: //LDAX B
			std::cout << "A = memory[BC]"; break;
		case 0x0b: //DCX B
			std::cout << "BC--"; break;
		case 0x0c: //INR C
			std::cout << "C++"; break;
		case 0x0d: //DCR C
			std::cout << "C--"; break;
		case 0x0e: //MVI C
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into C";
			opBytes = 2; break;
		case 0x0f: //RRC
			std::cout << "A>>1, shifted off bit placed onto other end"; break;
		case 0x11: //LXI D
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2];
			std::cout << " moved into D, ";
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into E";
			opBytes = 3; break;
		case 0x12: //STAX D
			std::cout << "Store A in location specified by DE"; break;
		case 0x13: //INX D
			std::cout << "DE++"; break;
		case 0x14: //INR D
			std::cout << "D++"; break;
		case 0x15: //DCR D
			std::cout << "D--"; break;
		case 0x16: //MVI D
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into D";
			opBytes = 2; break;
		case 0x17: //RAL
			std::cout << "A<<1, shifted off bit placed into carry, carry placed into other end"; break;
		case 0x19: //DAD D
			std::cout << "HL += DE"; break;
		case 0x1a: //LDAX D
			std::cout << "A = memory[DE]"; break;
		case 0x1b: //DCX D
			std::cout << "DE--"; break;
		case 0x1c: //INR E
			std::cout << "E++"; break;
		case 0x1d: //DCR E
			std::cout << "E--"; break;
		case 0x1e: //MVI E
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into E";
			opBytes = 2; break;
		case 0x1f: //RAR
			std::cout << "A>>1, shifted off bit placed into carry, carry placed into other end"; break;
		case 0x21: //LXI H
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+2];
			std::cout << " moved into H, ";
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into L";
			opBytes = 3; break;
		case 0x22: //SHLD
			std::cout << "memory[(byte3)(byte2)] = L, memory[(byte3)(byte2)+1] = H";
			opBytes = 3; break;
		case 0x23: //INX H
			std::cout << "HL++"; break;
		case 0x24: //INR H
			std::cout << "H++"; break;
		case 0x25: //DRC H
			std::cout << "H--"; break;
		case 0x26: //MVI H
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into H";
			opBytes = 2; break;
		case 0x27: //DAA
			std::cout << "Decimal Adjust Accumulator (check manual for details)"; break;
		case 0x29: //DAD H
			std::cout << "HL += HL"; break;
		case 0x2a: //LHLD
			std::cout << "L = memory[(byte3)(byte2)], H = memory[(byte3)(byte2)+1]";
			opBytes = 3; break;
		case 0x2b: //DCX H
			std::cout << "HL--"; break;
		case 0x2c: //INR L
			std::cout << "L++"; break;
		case 0x2d: //DCR L
			std::cout << "L--"; break;
		case 0x2e: //MVI L
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into L";
			opBytes = 2; break;
		case 0x2f: //CMA
			std::cout << "~A (negate bits of A)"; break;
		case 0x31: //LXI SP
			std::cout << "SP = (byte3)(byte2)";
			opBytes = 3; break;
		case 0x32: //STA
			std::cout << "memory[(byte3)(byte2)] = A";
			opBytes = 3; break;
		case 0x33: //INX SP
			std::cout << "SP++"; break;
		case 0x34: //INR M
			std::cout << "memory[HL]++"; break;
		case 0x35: //DCR M
			std::cout << "memory[HL]--"; break;
		case 0x36: //MVI M
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into memory[HL]";
			opBytes = 2; break;
		case 0x37: //STC
			std::cout << "Carry flag = 1"; break;
		case 0x39: //DAD SP
			std::cout << "HL += SP"; break;
		case 0x3a: //LDA
			std::cout << "A = memory[(byte3)(byte2)]";
			opBytes = 3; break;
		case 0x3b: //DCX
			std::cout << "SP--"; break;
		case 0x3c: //INR A
			std::cout << "A++"; break;
		case 0x3d: //DCR A
			std::cout << "A--"; break;
		case 0x3e: //MVI
			std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) memory[index+1];
			std::cout << " moved into A";
			opBytes = 2; break;
		case 0x3f: //CMC
			std::cout << "carry bit *= -1"; break;
		case 0x76: //HLT
			std::cout << "PC increments to next insturction, then waits for interrupt"; break;
		case 0xc0: //RNZ
			std::cout << "Return if zero bit = 0"; break;
		case 0xc1: //POP B
			std::cout << "C = memory[SP], B = memory[SP+1], SP += 2"; break;
		case 0xc2: //JNZ
			std::cout << "Jump if zero bit = 0";
			opBytes = 3; break;
		case 0xc3: //JMP
			std::cout << "Unconditional jump";
			opBytes = 3; break;
		case 0xc4: //CNZ
			std::cout << "Call if zero bit = 0";
			opBytes = 3; break;
		case 0xc5: //PUSH B
			std::cout << "memory[SP-1] = B, memory[SP-2] = C, SP -= 2"; break;
		case 0xc7: //RST 0
			std::cout << "memory[SP-1] = (PC highest 8 bits), memory[SP-2] = (PC lowest 8 bits), SP -= 2, PC = 0"; break;
		case 0xc8: //RZ
			std::cout << "Return if zero bit = 1"; break;
		case 0xc9: //RET
			std::cout << "Unconditional return"; break;
		case 0xca: //JZ
			std::cout << "Jump if zero bit = 1";
			opBytes = 3; break;
		case 0xcc: //CZ
			std::cout << "Call if zero bit = 1";
			opBytes = 3; break;
		case 0xcd: //CALL
			std::cout << "Unconditional Call, also special output for diagnostic";
			opBytes = 3; break;
		case 0xcf: //RST 1
			std::cout << "memory[SP-1] = (PC highest 8 bits), memory[SP-2] = (PC lowest 8 bits), SP -= 2, PC = 8"; break;
		case 0xd0: //RNC
			std::cout << "Return if carry bit = 0"; break;
		case 0xd1: //POP D
			std::cout << "E = memory[SP], D = memory[SP+1], SP += 2"; break;
		case 0xd2: //JNC
			std::cout << "Jump if carry bit = 0";
			opBytes = 3; break;
		case 0xd4: //CNC
			std::cout << "Call if carry bit = 0";
			opBytes = 3; break;
		case 0xd5: //PUSH D
			std::cout << "memory[SP-1] = D, memory[SP-2] = E, SP -= 2"; break;
		case 0xd7: //RST 2
			std::cout << "memory[SP-1] = (PC highest 8 bits), memory[SP-2] = (PC lowest 8 bits), SP -= 2, PC = 16"; break;
		case 0xd8: //RC
			std::cout << "Return if carry bit = 1"; break;
		case 0xda: //JC
			std::cout << "Jump if carry bit = 1";
			opBytes = 3; break;
		case 0xdc: //CC
			std::cout << "Call if carry bit = 1";
			opBytes = 3; break;
		case 0xdf: //RST 3
			std::cout << "memory[SP-1] = (PC highest 8 bits), memory[SP-2] = (PC lowest 8 bits), SP -= 2, PC = 16"; break;
		case 0xe0: //RPO
			std::cout << "Return if parity bit = 0"; break;
		case 0xe1: //POP H
			std::cout << "L = memory[SP], H = memory[SP+1], SP += 2"; break;
		case 0xe2: //JPO
			std::cout << "Jump if parity bit = 0";
			opBytes = 3; break;
		case 0xe3: //XTHL
			std::cout << "L <-> memory[SP], H <-> memory[SP+1]"; break;
		case 0xe4: //CPO
			std::cout << "Call if parity bit = 0";
			opBytes = 3; break;
		case 0xe5: //PUSH H
			std::cout << "memory[SP-1] = H, memory[SP-2] = L, SP -= 2"; break;
		case 0xe7: //RST 4
			std::cout << "memory[SP-1] = (PC highest 8 bits), memory[SP-2] = (PC lowest 8 bits), SP -= 2, PC = 24"; break;
		case 0xe8: //RPE
			std::cout << "Return if parity bit = 1"; break;
		case 0xe9: //PCHL
			std::cout << "(PC highest 8 bits) = H, (PC lowest 8 bits) = L"; break;
		case 0xea: //JPE
			std::cout << "Jump if parity bit = 1";
			opBytes = 3; break;
		case 0xeb: //XCHG
			std::cout << "H <-> D, L <-> E"; break;
		case 0xec: //CPE
			std::cout << "Call if parity bit = 1";
			opBytes = 3; break;
		case 0xee: //XRI
			std::cout << "XOR with immediate";
			opBytes = 2; break;
		case 0xef: //RST 4
			std::cout << "memory[SP-1] = (PC highest 8 bits), memory[SP-2] = (PC lowest 8 bits), SP -= 2, PC = 32"; break;
		case 0xf0: //RP
			std::cout << "Return if sign bit = 0"; break;
		case 0xf1: //POP PSW
			std::cout << "Take flag values off of stack"; break;
		case 0xf2: //JP
			std::cout << "Jump if sign bit = 0";
			opBytes = 3; break;
		case 0xf3: //DI
			std::cout << "int_enable = 0"; break;
		case 0xf4: //CP
			std::cout << "Call if sign bit = 0";
			opBytes = 3; break;
		case 0xf5: //PUSH PSW
			std::cout << "Push flags to stack"; break;
		case 0xf6: //ORI
			std::cout << "OR immediate";
			opBytes = 2; break;
		case 0xf7: //RST 6
			std::cout << "memory[SP-1] = (PC highest 8 bits), memory[SP-2] = (PC lowest 8 bits), SP -= 2, PC = 48"; break;
		case 0xf8: //RM
			std::cout << "Return if sign bit = 1"; break;
		case 0xf9: //SPHL
			std::cout << "SP = HL"; break;
		case 0xfa: //JM
			std::cout << "Jump if sign bit = 1";
			opBytes = 3; break;
		case 0xfb: //EI
			std::cout << "int_enable = 1"; break;
		case 0xfc: //CM
			std::cout << "Call if sign bit = 1";
			opBytes = 3; break;
		case 0xfe: //CPI
			std::cout << "Compare immediate";
			opBytes = 2; break;
		case 0xff: //RST 7
			std::cout << "memory[SP-1] = (PC highest 8 bits), memory[SP-2] = (PC lowest 8 bits), SP -= 2, PC = 56"; break;
	}

	std::cout << "\n";

	return opBytes;
}
