#ifndef machineState_h
#define machineState_h

#include <bitset>
#include <vector>

class MachineState {
public:
	MachineState(const std::string& fileName);
	~MachineState();

	void printState() const;
	void printDisassembled() const;
	bool isDone() const { return pc >= memorySize; }

	void processCommand();

private:
	//Reigsters and other data to store
	uint8_t a, b, c, d, e, h, l;
	uint16_t sp, pc;
	unsigned char* memory;
	uint16_t memorySize;
	uint8_t int_enable;
	uint8_t shift0, shift1, shift_offset;
	/*Condition Code reference
	0 = z = zero
	1 = s = sign
	2 = p = parity
	3 = cy = carry
	4 = ac = auxillary carry
	*/
	std::bitset<5> cc;

	//Helper commands for certian opcodes
	void sub(uint8_t num, uint8_t carry);
	void add(uint8_t num, uint16_t carry);
	void call(bool condition);
	void ret(bool condition);
	void ana(uint8_t num);
	void xra(uint8_t num);
	void ora(uint8_t num);
	void cmp(uint8_t num);
	void dad(uint16_t num);
	void rst(uint8_t num);

	int getOpcode(uint16_t index) const;
	int getOpcodeDescription(uint16_t index) const;
};

#endif