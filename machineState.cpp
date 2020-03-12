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
					memory.push_back(tempchar);
					firstChar = false;
				}
				else {
					memory[memory.size()-1] = memory[memory.size()-1] << 4;
					memory[memory.size()-1] += tempchar;
					firstChar = true;
				}
			}
		}
	}
	else {
		input.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);
		unsigned char tmp;
		bool firstChar = true;
		std::streampos size = input.tellg();
		char* mem = new char[size];
    	input.seekg (0, std::ios::beg);
    	input.read (mem, size);
		while(((tmp = input.get()) != EOF)) {
			// for(unsigned int i = 0; i < tmp.size(); i++) {
			// 	//tempchar takes care of 0 in ascii != 0 in hex
			// 	uint8_t tempchar = tmp[i];
			// 	if(tempchar < 58) tempchar -= 48;
			// 	else tempchar -= 87;

			// 	if(firstChar) {
			// 		memory.push_back(tempchar);
			// 		firstChar = false;
			// 	}
			// 	else {
			// 		memory[memory.size()-1] = memory[memory.size()-1] << 4;
			// 		memory[memory.size()-1] += tempchar;
			// 		firstChar = true;
			// 	}
			// }
			std::cout << std::hex << (int) firstChar << std::endl;
		}
		delete [] mem;
	}
	input.close();

	unsigned int fileSize = memory.size();

	for(unsigned int i = 0; i < fileSize; i++) {
		std::cout << std::hex << (int)this->memory[i] << " ";
		if(i%5 == 4)
			std::cout << "\n";
	}
	std::cout << std::endl;


	//establish initial values
	this->pc = 0;
	this->sp = 0x2000;
	this->a = 0;
	this->b = 0;
	this->c = 0;
	this->d = 0;
	this->e = 0;
	this->h = 0;
	this->l = 0;
	cc.set();
}

void MachineState::printState() const {
	std::cout << "pc,sp: " << std::hex << std::setw(4) << std::setfill('0') << +this->pc << "," << +this->sp << "\n";
	std::cout << "a\tb c\td e\th l\n";
	std::cout << std::setw(2) << std::setfill('0') << +this->a << "\t";
	std::cout << std::setw(2) << std::setfill('0') << +this->b;
	std::cout << std::setw(2) << std::setfill('0') << +this->c << "\t";
	std::cout << std::setw(2) << std::setfill('0') << +this->d;
	std::cout << std::setw(2) << std::setfill('0') << +this->e << "\t";
	std::cout << std::setw(2) << std::setfill('0') << +this->h;
	std::cout << std::setw(2) << std::setfill('0') << +this->l << "\n";
	std::cout << "z,s,p,cy,ac: " << +this->cc[0] << "," << +this->cc[1] << "," << +this->cc[2] << ","
								<< +this->cc[3] << "," << +this->cc[4] << "\n" << std::endl;
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
			this->pc += 2;
			break;
		case 0x02: //STAX   B
			this->memory[(this->b<<8) | (this->c)] = this->a;
			break;
		case 0x03: //INX    B
			temp16 = (this->b<<8) | (this->c);
			temp16++;
			this->b = temp16 >> 8;
			this->c = temp16 & 0xff;
			break;
		case 0x04: //INR    B
			this->cc[4] = ((this->b & 0x0f) + 1) > 0x0f;
			this->b++;
			this->cc[0] = ((this->b & 0xff) == 0);
			this->cc[1] = ((this->b & 0x80) != 0);
			this->cc[2] = Parity(this->b);
			break;
		case 0x05: //DCR    B
			this->cc[4] = ((this->b & 0x0f) + 0x0f) > 0x0f;
			this->b--;
			this->cc[0] = ((this->b & 0xff) == 0);
			this->cc[1] = ((this->b & 0x80) != 0);
			this->cc[2] = Parity(this->b);
			break;
		case 0x06: //MVI    B
			this->b = this->memory[this->pc+1];
			this->pc++;
			break;
		case 0x07: //RLC
			this->cc[3] = this->a & 0x80;
			this->a = (this->a<<1) | this->cc[3];
			break;
		case 0x08: //NOP
			break;
		case 0x09: //DAD    B
			dad((this->b<<8)|this->c);
			break;
		case 0x0a: //LDAX   B
			this->a = this->memory[(this->b<<8) | (this->c)];
			break;
		case 0x0b: //DCX    B
			temp16 = (this->b<<8) | this->c;
			this->b = temp16>>8;
			this->c = temp16&0xff;
			break;
		case 0x0c: //INR    C
			this->cc[4] = ((this->c & 0x0f) + 1) > 0x0f;
			this->c++;
			this->cc[0] = ((this->c & 0xff) == 0);
			this->cc[1] = ((this->c & 0x80) != 0);
			this->cc[2] = Parity(this->c);
			break;
		case 0x0d: //DCR    C
			this->cc[4] = ((this->c & 0x0f) + 0x0f) > 0x0f;
			this->c--;
			this->cc[0] = ((this->c & 0xff) == 0);
			this->cc[1] = ((this->c & 0x80) != 0);
			this->cc[2] = Parity(this->c);
			break;
		case 0x0e: //MVI    C
			this->c = this->memory[this->pc+1];
			this->pc++;
			break;
		case 0x0f: //RRC
			this->cc[3] = this->a & 0x01;
			this->a = (this->a<<1) | (this->cc[3]<<7);
			break;
		case 0x10: //NOP
			break;
		case 0x11: //LXI    D,word
			this->e = this->memory[this->pc+1];
			this->d = this->memory[this->pc+2];
			this->pc += 2;
			break;
		case 0x12:  //STAX   D
			this->memory[(this->d<<8) | (this->e)] = this->a;
			break;
		case 0x13: //INX    D
			temp16 = (this->d<<8) | (this->e);
			temp16++;
			this->d = temp16 >> 8;
			this->e = temp16 & 0xff;
			break;
		case 0x14: //INR    D
			this->cc[4] = ((this->d & 0x0f) + 1) > 0x0f;
			this->d++;
			this->cc[0] = ((this->d & 0xff) == 0);
			this->cc[1] = ((this->d & 0x80) != 0);
			this->cc[2] = Parity(this->d);
			break;
		case 0x15: //DCR    D
			this->cc[4] = ((this->d & 0x0f) + 0x0f) > 0x0f;
			this->d--;
			this->cc[0] = ((this->d & 0xff) == 0);
			this->cc[1] = ((this->d & 0x80) != 0);
			this->cc[2] = Parity(this->d);
			break;
		case 0x16: //MVI    D
			this->d = this->memory[this->pc+1];
			this->pc++;
			break;
		case 0x17: //RAL
			temp8 = this->cc[3];
			this->cc[3] = this->a & 0x80;
			this->a = (this->a<<1) | temp8;
			break;
		case 0x18: //NOP
			break;
		case 0x19: //DAD    D
			dad((this->d<<8)|this->e);
			break;
		case 0x1a: //LDAX   D
			this->a = this->memory[(this->d<<8) | (this->e)];
			break;
		case 0x1b: //DCX    D
			temp16 = (this->d<<8) | this->e;
			this->d = temp16>>8;
			this->e = temp16&0xff;
			break;
		case 0x1c: //INR    E
			this->cc[4] = ((this->e & 0x0f) + 1) > 0x0f;
			this->e++;
			this->cc[0] = ((this->e & 0xff) == 0);
			this->cc[1] = ((this->e & 0x80) != 0);
			this->cc[2] = Parity(this->e);
			break;
		case 0x1d: //DCR    E
			this->cc[4] = ((this->e & 0x0f) + 0x0f) > 0x0f;
			this->e--;
			this->cc[0] = ((this->e & 0xff) == 0);
			this->cc[1] = ((this->e & 0x80) != 0);
			this->cc[2] = Parity(this->e);
			break;
		case 0x1e: //MVI    E
			this->e = this->memory[this->pc+1];
			this->pc++;
			break;
		case 0x1f: //RAR
			temp8 = this->cc[3];
			this->cc[3] = this->a & 0x01;
			this->a = (this->a<<1) | (temp8<<7);
			break;
		case 0x20: //NOP
			break;
		case 0x21: //LXI    H,word
			this->l = this->memory[this->pc+1];
			this->h = this->memory[this->pc+2];
			this->pc += 2;
			break;
		case 0x22: //SHLD
			temp16 = (this->memory[this->pc+2]<<8) | this->memory[this->pc+1];
			this->memory[temp16] = this->l;
			this->memory[temp16+1] = this->h;
			this->pc += 2;
			break;
		case 0x23: //INX    H
			temp16 = (this->h<<8) | (this->l);
			temp16++;
			this->h = temp16 >> 8;
			this->l = temp16 & 0xff;
			break;
		case 0x24: //INR    H
			this->cc[4] = ((this->h & 0x0f) + 1) > 0x0f;
			this->h++;
			this->cc[0] = ((this->h & 0xff) == 0);
			this->cc[1] = ((this->h & 0x80) != 0);
			this->cc[2] = Parity(this->h);
			break;
		case 0x25: //DCR    H
			this->cc[4] = ((this->h & 0x0f) + 0x0f) > 0x0f;
			this->h--;
			this->cc[0] = ((this->h & 0xff) == 0);
			this->cc[1] = ((this->h & 0x80) != 0);
			this->cc[2] = Parity(this->h);
			break;
		case 0x26: //MVI    H
			this->h = this->memory[this->pc+1];
			this->pc++;
			break;
		case 0x27: //DAA
			if(this->cc[4] || ((this->a & 0xf) > 9)) {
				temp8 = (this->a & 0xf) + 1;
				this->cc[4] = (temp8 > 0xf);
				this->a += 6;
			}
			if((this->a>>4) > 9) {
				temp8 = (this->a>>4) + 1;
				this->cc[3] = (temp8 > 0xf);
				this->a = (this->a&0xf) | (temp8<<4);
			}
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a);
			break;
		case 0x28: //NOP
			break;
		case 0x29: //DAD    H
			dad((this->h<<8)|this->l);
			break;
		case 0x2a: //LHLD
			temp16 = (this->memory[this->pc+2]<<8) | this->memory[this->pc+1];
			this->l = this->memory[temp16];
			this->h = this->memory[temp16+1];
			break;
		case 0x2b: //DCX    H
			temp16 = (this->h<<8) | this->l;
			this->h = temp16>>8;
			this->l = temp16&0xff;
			break;
		case 0x2c:  //INR    L
			this->cc[4] = ((this->l & 0x0f) + 1) > 0x0f;
			this->l++;
			this->cc[0] = ((this->l & 0xff) == 0);
			this->cc[1] = ((this->l & 0x80) != 0);
			this->cc[2] = Parity(this->l);
			break;
		case 0x2d: //DCR    L
			this->cc[4] = ((this->l & 0x0f) + 0x0f) > 0x0f;
			this->l--;
			this->cc[0] = ((this->l & 0xff) == 0);
			this->cc[1] = ((this->l & 0x80) != 0);
			this->cc[2] = Parity(this->l);
			break;
		case 0x2e: //MVI    L
			this->l = this->memory[this->pc+1];
			this->pc++;
			break;
		case 0x2f: //CMA
			this->a = ~this->a;
			break;
		case 0x30: //NOP
			break;
		case 0x31: //LXI    SP,word
			this->sp = (this->memory[this->pc+2]<<8) | this->memory[this->pc+1];
			this->pc += 2;
			break;
		case 0x32: //STA
			this->memory[this->memory[this->pc+2]<<8 | this->memory[this->pc+1]] = this->a;
			this->pc += 2;
			break;
		case 0x33:  //INX    SP
			this->sp++;
			break;
		case 0x34:  //INR    M
			this->cc[4] = ((this->memory[(this->h<<8) | (this->l)] & 0x0f) + 1) > 0x0f;
			this->memory[(this->h<<8) | (this->l)]++;
			this->cc[0] = ((this->memory[(this->h<<8) | (this->l)] & 0xff) == 0);
			this->cc[1] = ((this->memory[(this->h<<8) | (this->l)] & 0x80) != 0);
			this->cc[2] = Parity(this->memory[(this->h<<8) | (this->l)]);
			break;
		case 0x35: //DCR    M
			this->cc[4] = ((this->memory[(this->h<<8) | (this->l)] & 0x0f) + 0x0f) > 0x0f;
			this->memory[(this->h<<8) | (this->l)]--;
			this->cc[0] = ((this->memory[(this->h<<8) | (this->l)] & 0xff) == 0);
			this->cc[1] = ((this->memory[(this->h<<8) | (this->l)] & 0x80) != 0);
			this->cc[2] = Parity(this->memory[(this->h<<8) | (this->l)]);
			break;
		case 0x36: //MVI    M
			this->memory[(this->h<<8) | (this->l)] = this->memory[this->pc+1];
			this->pc++;
			break;
		case 0x37: //STC
			this->cc[3] = 1;
			break;
		case 0x38: //NOP
			break;
		case 0x39: //DAD    SP
			dad(this->sp);
			break;
		case 0x3a: //LDA
			this->a = this->memory[this->memory[this->pc+2]<<8 | this->memory[this->pc+1]];
			this->pc += 2;
			break;
		case 0x3b: //DCX    SP
			this->sp--;
			break;
		case 0x3c: //INR    A
			this->cc[4] = ((this->a & 0x0f) + 1) > 0x0f;
			this->a++;
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a);
			break;
		case 0x3d: //DCR    A
			this->cc[4] = ((this->a & 0x0f) + 0x0f) > 0x0f;
			this->a--;
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a);
			break;
		case 0x3e: //MVI    A
			this->a = this->memory[this->pc+1];
			this->pc++;
			break;
		case 0x3f: //CMC
			this->cc[3] = !this->cc[3];
			break;
		case 0x40: //MOV    B,B
			this->b = this->b;
			break;
		case 0x41: //MOV    B,C
			this->b = this->c;
			break;
		case 0x42: //MOV    B,D
			this->b = this->d;
			break;
		case 0x43: //MOV    B,E
			this->b = this->e;
			break;
		case 0x44: //MOV    B,H
			this->b = this->h;
			break;
		case 0x45: //MOV    B,L
			this->b = this->l;
			break;
		case 0x46: //MOV    B,M
			this->b = this->memory[(this->h<<8) | (this->l)];
			break;
		case 0x47: //MOV    B,A
			this->b = this->a;
			break;
		case 0x48: //MOV    C,B
			this->c = this->b;
			break;
		case 0x49: //MOV    C,C
			this->c = this->c;
			break;
		case 0x4a: //MOV    C,D
			this->c = this->d;
			break;
		case 0x4b: //MOV    C,E
			this->c = this->e;
			break;
		case 0x4c: //MOV    C,H
			this->c = this->h;
			break;
		case 0x4d: //MOV    C,L
			this->c = this->l;
			break;
		case 0x4e: //MOV    C,M
			this->c = this->memory[(this->h<<8) | (this->l)];
			break;
		case 0x4f: //MOV    C,A
			this->c = this->a;
			break;
		case 0x50: //MOV    D,B
			this->d = this->b;
			break;
		case 0x51: //MOV    D,C
			this->d = this->c;
			break;
		case 0x52: //MOV    D,D
			this->d = this->d;
			break;
		case 0x53: //MOV    D,E
			this->d = this->e;
			break;
		case 0x54: //MOV    D,H
			this->d = this->h;
			break;
		case 0x55: //MOV    D,L
			this->d = this->l;
			break;
		case 0x56: //MOV    D,M
			this->d = this->memory[(this->h<<8) | (this->l)];
			break;
		case 0x57: //MOV    D,A
			this->d = this->a;
			break;
		case 0x58: //MOV    E,B
			this->e = this->b;
			break;
		case 0x59: //MOV    E,C
			this->e = this->c;
			break;
		case 0x5a: //MOV    E,D
			this->e = this->d;
			break;
		case 0x5b: //MOV    E,E
			this->e = this->e;
			break;
		case 0x5c: //MOV    E,H
			this->e = this->h;
			break;
		case 0x5d: //MOV    E,L
			this->e = this->l;
			break;
		case 0x5e: //MOV    E,M
			this->e = this->memory[(this->h<<8) | (this->l)];
			break;
		case 0x5f: //MOV    E,A
			this->e = this->a;
			break;
		case 0x60: //MOV    H,B
			this->h = this->b;
			break;
		case 0x61: //MOV    H,C
			this->h = this->c;
			break;
		case 0x62: //MOV    H,D
			this->h = this->d;
			break;
		case 0x63: //MOV    H,E
			this->h = this->e;
			break;
		case 0x64: //MOV    H,H
			this->h = this->h;
			break;
		case 0x65: //MOV    H,L
			this->h = this->l;
			break;
		case 0x66: //MOV    H,M
			this->h = this->memory[(this->h<<8) | (this->l)];
			break;
		case 0x67: //MOV    H,A
			this->h = this->a;
			break;
		case 0x68: //MOV    L,B
			this->l = this->b;
			break;
		case 0x69: //MOV    L,C
			this->l = this->c;
			break;
		case 0x6a: //MOV    L,D
			this->l = this->d;
			break;
		case 0x6b: //MOV    L,E
			this->l = this->e;
			break;
		case 0x6c: //MOV    L,H
			this->l = this->h;
			break;
		case 0x6d: //MOV    L,L
			this->l = this->l;
			break;
		case 0x6e: //MOV    L,M
			this->l = this->memory[(this->h<<8) | (this->l)];
			break;
		case 0x6f: //MOV    L,A
			this->l = this->a;
			break;
		case 0x70: //MOV    M,B
			this->memory[(this->h<<8) | (this->l)] = this->b;
			break;
		case 0x71: //MOV    M,C
			this->memory[(this->h<<8) | (this->l)] = this->c;
			break;
		case 0x72: //MOV    M,D
			this->memory[(this->h<<8) | (this->l)] = this->d;
			break;
		case 0x73: //MOV    M,E
			this->memory[(this->h<<8) | (this->l)] = this->e;
			break;
		case 0x74: //MOV    M,H
			this->memory[(this->h<<8) | (this->l)] = this->h;
			break;
		case 0x75: //MOV    M,L
			this->memory[(this->h<<8) | (this->l)] = this->l;
			break;
		case 0x76: //HLT
			exit(0);
			break;
		case 0x77: //MOV    M,A
			this->memory[(this->h<<8) | (this->l)] = this->a;
			break;
		case 0x78: //MOV    A,B
			this->a = this->b;
			break;
		case 0x79: //MOV    A,C
			this->a = this->c;
			break;
		case 0x7a: //MOV    A,D
			this->a = this->d;
			break;
		case 0x7b: //MOV    A,E
			this->a = this->e;
			break;
		case 0x7c: //MOV    A,H
			this->a = this->h;
			break;
		case 0x7d: //MOV    A,L
			this->a = this->l;
			break;
		case 0x7e: //MOV    A,M
			this->a = this->memory[(this->h<<8) | (this->l)];
			break;
		case 0x7f: //MOV    A,A
			this->a = this->a;
			break;
		case 0x80: //ADD    B
			add(this->b, 0);
			break;
		case 0x81: //ADD    C
			add(this->c, 0);
			break;
		case 0x82: //ADD    D
			add(this->d, 0);
			break;
		case 0x83: //ADD    E
			add(this->e, 0);
			break;
		case 0x84: //ADD    H
			add(this->h, 0);
			break;
		case 0x85: //ADD    L
			add(this->l, 0);
			break;
		case 0x86: //ADD    M
			add(this->memory[(this->h<<8) | (this->l)], 0);
			break;
		case 0x87: //ADD    A
			add(this->a, 0);
			break;
		case 0x88: //ADC    B
			add(this->b, this->cc[3]);
			break;
		case 0x89: //ADC    C
			add(this->c, this->cc[3]);
			break;
		case 0x8a: //ADC    D
			add(this->d, this->cc[3]);
			break;
		case 0x8b: //ADC    E
			add(this->e, this->cc[3]);
			break;
		case 0x8c: //ADC    H
			add(this->h, this->cc[3]);
			break;
		case 0x8d: //ADC    L
			add(this->l, this->cc[3]);
			break;
		case 0x8e: //ADC    M
			add(this->memory[(this->h<<8) | (this->l)], this->cc[3]);
			break;
		case 0x8f: //ADC    A
			add(this->a, this->cc[3]);
			break;
		case 0x90: //SUB    B
			add(~this->b + 1, this->cc[3]);
			break;
		case 0x91: //SUB    C
			add(~this->c + 1, this->cc[3]);
			break;
		case 0x92: //SUB    D
			add(~this->d + 1, this->cc[3]);
			break;
		case 0x93: //SUB    E
			add(~this->e + 1, this->cc[3]);
			break;
		case 0x94: //SUB    H
			add(~this->h + 1, this->cc[3]);
			break;
		case 0x95: //SUB    L
			add(~this->l + 1, this->cc[3]);
			break;
		case 0x96: //SUB    M
			add(~this->memory[(this->h<<8) | (this->l)] + 1, this->cc[3]);
			break;
		case 0x97: //SUB    A
			add(~this->a + 1, this->cc[3]);
			break;
		case 0x98: //SBB    B
			add(~(1+this->b) + 1, this->cc[3]);
			break;
		case 0x99: //SBB    C
			add(~(1+this->c) + 1, this->cc[3]);
			break;
		case 0x9a: //SBB    D
			add(~(1+this->d) + 1, this->cc[3]);
			break;
		case 0x9b: //SBB    E
			add(~(1+this->e) + 1, this->cc[3]);
			break;
		case 0x9c: //SBB    H
			add(~(1+this->h) + 1, this->cc[3]);
			break;
		case 0x9d: //SBB    L
			add(~(1+this->l) + 1, this->cc[3]);
			break;
		case 0x9e: //SBB    M
			add(~(1+this->memory[(this->h<<8) | (this->l)]) + 1, this->cc[3]);
			break;
		case 0x9f: //SBB    A
			add(~(1+this->a) + 1, this->cc[3]);
			break;
		case 0xa0: //ANA    B
			ana(this->b);
			break;
		case 0xa1: //ANA    C
			ana(this->c);
			break;
		case 0xa2: //ANA    D
			ana(this->d);
			break;
		case 0xa3: //ANA    E
			ana(this->e);
			break;
		case 0xa4: //ANA    H
			ana(this->h);
			break;
		case 0xa5: //ANA    L
			ana(this->l);
			break;
		case 0xa6: //ANA    M
			ana(this->memory[(this->h<<8) | (this->l)]);
			break;
		case 0xa7: //ANA    A
			ana(this->a);
			break;
		case 0xa8: //XRA    B
			xra(this->b);
			break;
		case 0xa9: //XRA    C
			xra(this->c);
			break;
		case 0xaa: //XRA    D
			xra(this->d);
			break;
		case 0xab: //XRA    E
			xra(this->e);
			break;
		case 0xac: //XRA    H
			xra(this->h);
			break;
		case 0xad: //XRA    L
			xra(this->l);
			break;
		case 0xae: //XRA    M
			xra(this->memory[(this->h<<8) | (this->l)]);
			break;
		case 0xaf: //XRA    A
			xra(this->a);
			break;
		case 0xb0: //ORA    B
			ora(this->b);
			break;
		case 0xb1: //ORA    C
			ora(this->c);
			break;
		case 0xb2: //ORA    D
			ora(this->d);
			break;
		case 0xb3: //ORA    E
			ora(this->e);
			break;
		case 0xb4: //ORA    H
			ora(this->h);
			break;
		case 0xb5: //ORA    L
			ora(this->l);
			break;
		case 0xb6: //ORA    M
			ora(this->memory[(this->h<<8) | (this->l)]);
			break;
		case 0xb7: //ORA    A
			ora(this->a);
			break;
		case 0xb8: //CMP    B
			cmp(this->b);
			break;
		case 0xb9: //CMP    C
			cmp(this->c);
			break;
		case 0xba: //CMP    D
			cmp(this->d);
			break;
		case 0xbb: //CMP    E
			cmp(this->e);
			break;
		case 0xbc: //CMP    H
			cmp(this->h);
			break;
		case 0xbd: //CMP    L
			cmp(this->l);
			break;
		case 0xbe: //CMP    M
			cmp(this->memory[(this->h<<8) | (this->l)]);
			break;
		case 0xbf: //CMP    A
			cmp(this->a);
			break;
		case 0xc0: //RNZ
			ret(!this->cc[0]);
			break;
		case 0xc1: //POP    B
			this->c = this->memory[this->sp];
			this->b = this->memory[this->sp+1];
			this->sp += 2;
			break;
		case 0xc2: //JNZ
			if(!this->cc[0])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xc3: //JMP
			this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			break;
		case 0xc4: //CNZ
			call(!this->cc[0]);
			break;
		case 0xc5: //PUSH   B
			this->memory[this->sp-1] = this->b;    
            this->memory[this->sp-2] = this->c;    
            this->sp -= 2;
			break;
		case 0xc6: //ADI
			add(this->memory[this->pc+1], 0);
			this->pc++;
			break;
		case 0xc7: //RST    0
			rst(0);
			break;
		case 0xc8: //RZ
			ret(this->cc[0]);
			break;
		case 0xc9: //RET
			ret(true);
			break;
		case 0xca: //JZ
			if(this->cc[0])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xcb: //NOP
			break;
		case 0xcc: //CZ
			call(this->cc[0]);
			break;
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
			break;
		case 0xcf: //RST    1
			rst(1);
			break;
		case 0xd0: //RNC
			ret(!this->cc[3]);
			break;
		case 0xd1: //POP    D
			this->d = this->memory[this->sp];
			this->e = this->memory[this->sp+1];
			this->sp += 2;
			break;
		case 0xd2: //JNC
			if(!this->cc[3])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xd3: //OUT
			this->pc++;
			break;
		case 0xd4: //CNC
			call(!this->cc[3]);
			break;
		case 0xd5: //PUSH   D
			this->memory[this->sp-1] = this->d;    
            this->memory[this->sp-2] = this->e;    
            this->sp -= 2;
			break;
		case 0xd6: //SUI
			add(~this->memory[this->pc+1] + 1, 0);
			this->pc++;
			break;
		case 0xd7: //RST    2
			rst(2);
			break;
		case 0xd8: //RC
			ret(this->cc[3]);
			break;
		case 0xd9: //NOP
			break;
		case 0xda: //JC
			if(this->cc[3])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xdb: //IN
    	    // this->a = MachineIN(this->memory[this->pc+1]);
    	    this->pc++;
			break;
		case 0xdc: //CC
			call(this->cc[3]);
			break;
		case 0xdd: //NOP
			break;
		case 0xde: //SBI
			add(~(this->memory[this->pc+1]+1) + 1, 0);
			break;
		case 0xdf: //RST    3
			rst(3);
			break;
		case 0xe0: //RPO
			ret(!this->cc[2]);
			break;
		case 0xe1: //POP    H
			this->h = this->memory[this->sp];
			this->l = this->memory[this->sp+1];
			this->sp += 2;
			break;
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
			this->memory[this->sp+1] = temp8;
			break;
		case 0xe4: //CPO
			call(!this->cc[2]);
			break;
		case 0xe5: //PUSH   H
			this->memory[this->sp-1] = this->h;    
            this->memory[this->sp-2] = this->l;    
            this->sp -= 2;
			break;
		case 0xe6: //ANI
			this->a = this->a & this->memory[this->pc+1];
			this->cc[3] = 0;
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a & 0xff);
			this->pc++;
			break;
		case 0xe7: //RST    4
			rst(4);
			break;
		case 0xe8: //RPE
			ret(this->cc[2]);
			break;
		case 0xe9: //PCHL
			this->pc = ((this->h<<8) | (this->l)) - 1;
			break;
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
			this->l = temp8;
			break;
		case 0xec: //CPE
			call(this->cc[2]);
			break;
		case 0xed: //NOP
			break;
		case 0xee: //XRI
			this->a = this->a ^ this->memory[this->pc+1];
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a & 0xff);
			this->cc[3] = 0;
			this->pc++;
			break;
		case 0xef: //RST    5
			rst(5);
			break;
		case 0xf0: //RP
			ret(!this->cc[1]);
			break;
		case 0xf1: //POP    PSW
			this->a = this->memory[this->sp+1];  
            this->cc[0]  = (0x01 == (this->memory[this->sp] & 0x01));    
            this->cc[1]  = (0x02 == (this->memory[this->sp] & 0x02));    
            this->cc[2]  = (0x04 == (this->memory[this->sp] & 0x04));    
            this->cc[3] = (0x05 == (this->memory[this->sp] & 0x08));    
            this->cc[4] = (0x10 == (this->memory[this->sp] & 0x10));    
            this->sp += 2; 
			break;
		case 0xf2: //JP
			if(!this->cc[1])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xf3: //DI
			this->int_enable = 0;
			break;
		case 0xf4: //CP
			call(!this->cc[1]);
			break;
		case 0xf5: //PUSH   PSW
			this->memory[this->sp-1] = this->a;
            this->memory[this->sp-2] = (this->cc[0] |
                            this->cc[1] << 1 |
                            this->cc[2] << 2 |
                            this->cc[3] << 3 |
                            this->cc[4] << 4 );
            this->sp -= 2;
			break;
		case 0xf6: //ORI
			this->a = this->a | this->memory[this->pc+1];
			this->cc[0] = ((this->a & 0xff) == 0);
			this->cc[1] = ((this->a & 0x80) != 0);
			this->cc[2] = Parity(this->a & 0xff);
			this->cc[3] = 0;
			this->pc++;
			break;
		case 0xf7: //RST    6
			rst(6);
			break;
		case 0xf8: //RM
			ret(this->cc[1]);
			break;
		case 0xf9: //SPHL
			this->sp = (this->h << 8) | this->l;
			break;
		case 0xfa: //JM
			if(this->cc[1])
				this->pc = ((this->memory[this->pc+2] << 8) | this->memory[this->pc+1]) - 1;
			else
				this->pc += 2;
			break;
		case 0xfb: //EI
			this->int_enable = 1;
			break;
		case 0xfc: //CM
			call(this->cc[1]);
			break;
		case 0xfd: //NOP
			break;
		case 0xfe: //CPI
			cmp(this->memory[this->pc+1]);
			this->pc++;
			break;
		case 0xff: //RST    7
			rst(7);
			break;
	}
	this->pc++;
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
		uint16_t ret = this->pc + 2;
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
	else
		this->pc += 2;
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
	uint16_t answer = (uint16_t) this->a + (uint16_t) tmp + 1;
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