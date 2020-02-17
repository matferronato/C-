#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <map>
#include <string>

#ifdef LOGMODE
#include <stdio.h>
#endif

class Bus;

class CPU6502
{
public:
	enum flags {
		C = (1 << 0),	// Carry Bit
		Z = (1 << 1),	// Zero
		I = (1 << 2),	// Disable Interrupts
		D = (1 << 3),	// Decimal Mode (unused in this implementation)
		B = (1 << 4),	// Break
		U = (1 << 5),	// Unused
		V = (1 << 6),	// Overflow
		N = (1 << 7),	// Negative
	};

	uint8_t fetchedData = 0x00;
	uint8_t status = 0x00;
	uint8_t opcode = 0x00;
	uint8_t cycles = 0x00;
	uint8_t regX = 0x00;
	uint8_t regY = 0x00;
	uint8_t regA = 0x00;
	uint8_t stkp = 0x00;

	uint16_t addressAbsolute = 0x0000;
	uint16_t addressRelative = 0x0000;
	uint16_t aux = 0x0000;
	uint16_t pc = 0x00;

	uint32_t globalClock = 0;

	//functions regarding addressing mode
	uint8_t IMP();	uint8_t IMM();
	uint8_t ZP0();	uint8_t ZPX();
	uint8_t ZPY();	uint8_t REL();
	uint8_t ABS();	uint8_t ABX();
	uint8_t ABY();	uint8_t IND();
	uint8_t IZX();	uint8_t IZY();

	//opcodes
	uint8_t ADC();	uint8_t AND();	uint8_t ASL();	uint8_t BCC();
	uint8_t BCS();	uint8_t BEQ();	uint8_t BIT();	uint8_t BMI();
	uint8_t BNE();	uint8_t BPL();	uint8_t BRK();	uint8_t BVC();
	uint8_t BVS();	uint8_t CLC();	uint8_t CLD();	uint8_t CLI();
	uint8_t CLV();	uint8_t CMP();	uint8_t CPX();	uint8_t CPY();
	uint8_t DEC();	uint8_t DEX();	uint8_t DEY();	uint8_t EOR();
	uint8_t INC();	uint8_t INX();	uint8_t INY();	uint8_t JMP();
	uint8_t JSR();	uint8_t LDA();	uint8_t LDX();	uint8_t LDY();
	uint8_t LSR();	uint8_t NOP();	uint8_t ORA();	uint8_t PHA();
	uint8_t PHP();	uint8_t PLA();	uint8_t PLP();	uint8_t ROL();
	uint8_t ROR();	uint8_t RTI();	uint8_t RTS();	uint8_t SBC();
	uint8_t SEC();	uint8_t SED();	uint8_t SEI();	uint8_t STA();
	uint8_t STX();	uint8_t STY();	uint8_t TAX();	uint8_t TAY();
	uint8_t TSX();	uint8_t TXA();	uint8_t TXS();	uint8_t TYA();
	uint8_t XXX();

	void clock();
	void reset();
	void interupt();
	void nonMaskInt();

	uint8_t fetchData();
	


private:
	Bus *bus = nullptr;
	void writeMemory(uint16_t address, uint8_t data);
	uint8_t readMemory(uint16_t address, bool readOnly = false);
	uint8_t getFlag(flags flag);
	void setFlag(flags flag, bool v);

	struct Instruction
	{
		std::string name; //neumonico
		uint8_t(CPU6502::* operate)(void) = nullptr; //function pointer do operation
		uint8_t(CPU6502::* addrmode)(void) = nullptr; //function pointer do addr mode
		uint8_t cycles = 0; //number of clock cycles required by instruction
	};
	
	std::vector<Instruction> lut; //lookup table

public:
	CPU6502();
	~CPU6502();

	void connectBus(Bus* n) { bus = n; } //provide bus with correct memory address via pointer n
	bool complete();
	std::map<uint16_t, std::string> disassemble(uint16_t nStart, uint16_t nStop);


#ifdef LOGMODE
private:
	FILE* logfile = nullptr;
#endif
};

