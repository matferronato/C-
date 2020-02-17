#include "cpu6502.h"
#include "Bus.h"


//CONSTRUCTOR DESTRUCTOR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


CPU6502::CPU6502(){
	using a = CPU6502; //just creating an alias to CPU6502 to avoid giant names when calling functions via CPU6502::
	//filing lut instruction struct list according to 6502 datasheet 
	lut =
	{
		{ "BRK", &a::BRK, &a::IMM, 7 },{ "ORA", &a::ORA, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "ORA", &a::ORA, &a::ZP0, 3 },{ "ASL", &a::ASL, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PHP", &a::PHP, &a::IMP, 3 },{ "ORA", &a::ORA, &a::IMM, 2 },{ "ASL", &a::ASL, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ABS, 4 },{ "ASL", &a::ASL, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BPL", &a::BPL, &a::REL, 2 },{ "ORA", &a::ORA, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ZPX, 4 },{ "ASL", &a::ASL, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLC", &a::CLC, &a::IMP, 2 },{ "ORA", &a::ORA, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ABX, 4 },{ "ASL", &a::ASL, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "JSR", &a::JSR, &a::ABS, 6 },{ "AND", &a::AND, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "BIT", &a::BIT, &a::ZP0, 3 },{ "AND", &a::AND, &a::ZP0, 3 },{ "ROL", &a::ROL, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PLP", &a::PLP, &a::IMP, 4 },{ "AND", &a::AND, &a::IMM, 2 },{ "ROL", &a::ROL, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "BIT", &a::BIT, &a::ABS, 4 },{ "AND", &a::AND, &a::ABS, 4 },{ "ROL", &a::ROL, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BMI", &a::BMI, &a::REL, 2 },{ "AND", &a::AND, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "AND", &a::AND, &a::ZPX, 4 },{ "ROL", &a::ROL, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SEC", &a::SEC, &a::IMP, 2 },{ "AND", &a::AND, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "AND", &a::AND, &a::ABX, 4 },{ "ROL", &a::ROL, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "RTI", &a::RTI, &a::IMP, 6 },{ "EOR", &a::EOR, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "EOR", &a::EOR, &a::ZP0, 3 },{ "LSR", &a::LSR, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PHA", &a::PHA, &a::IMP, 3 },{ "EOR", &a::EOR, &a::IMM, 2 },{ "LSR", &a::LSR, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "JMP", &a::JMP, &a::ABS, 3 },{ "EOR", &a::EOR, &a::ABS, 4 },{ "LSR", &a::LSR, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BVC", &a::BVC, &a::REL, 2 },{ "EOR", &a::EOR, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "EOR", &a::EOR, &a::ZPX, 4 },{ "LSR", &a::LSR, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLI", &a::CLI, &a::IMP, 2 },{ "EOR", &a::EOR, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "EOR", &a::EOR, &a::ABX, 4 },{ "LSR", &a::LSR, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "RTS", &a::RTS, &a::IMP, 6 },{ "ADC", &a::ADC, &a::IZX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "ADC", &a::ADC, &a::ZP0, 3 },{ "ROR", &a::ROR, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PLA", &a::PLA, &a::IMP, 4 },{ "ADC", &a::ADC, &a::IMM, 2 },{ "ROR", &a::ROR, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "JMP", &a::JMP, &a::IND, 5 },{ "ADC", &a::ADC, &a::ABS, 4 },{ "ROR", &a::ROR, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BVS", &a::BVS, &a::REL, 2 },{ "ADC", &a::ADC, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "ADC", &a::ADC, &a::ZPX, 4 },{ "ROR", &a::ROR, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SEI", &a::SEI, &a::IMP, 2 },{ "ADC", &a::ADC, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "ADC", &a::ADC, &a::ABX, 4 },{ "ROR", &a::ROR, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "???", &a::NOP, &a::IMP, 2 },{ "STA", &a::STA, &a::IZX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "STY", &a::STY, &a::ZP0, 3 },{ "STA", &a::STA, &a::ZP0, 3 },{ "STX", &a::STX, &a::ZP0, 3 },{ "???", &a::XXX, &a::IMP, 3 },{ "DEY", &a::DEY, &a::IMP, 2 },{ "???", &a::NOP, &a::IMP, 2 },{ "TXA", &a::TXA, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "STY", &a::STY, &a::ABS, 4 },{ "STA", &a::STA, &a::ABS, 4 },{ "STX", &a::STX, &a::ABS, 4 },{ "???", &a::XXX, &a::IMP, 4 },
		{ "BCC", &a::BCC, &a::REL, 2 },{ "STA", &a::STA, &a::IZY, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "STY", &a::STY, &a::ZPX, 4 },{ "STA", &a::STA, &a::ZPX, 4 },{ "STX", &a::STX, &a::ZPY, 4 },{ "???", &a::XXX, &a::IMP, 4 },{ "TYA", &a::TYA, &a::IMP, 2 },{ "STA", &a::STA, &a::ABY, 5 },{ "TXS", &a::TXS, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 5 },{ "???", &a::NOP, &a::IMP, 5 },{ "STA", &a::STA, &a::ABX, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "???", &a::XXX, &a::IMP, 5 },
		{ "LDY", &a::LDY, &a::IMM, 2 },{ "LDA", &a::LDA, &a::IZX, 6 },{ "LDX", &a::LDX, &a::IMM, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "LDY", &a::LDY, &a::ZP0, 3 },{ "LDA", &a::LDA, &a::ZP0, 3 },{ "LDX", &a::LDX, &a::ZP0, 3 },{ "???", &a::XXX, &a::IMP, 3 },{ "TAY", &a::TAY, &a::IMP, 2 },{ "LDA", &a::LDA, &a::IMM, 2 },{ "TAX", &a::TAX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "LDY", &a::LDY, &a::ABS, 4 },{ "LDA", &a::LDA, &a::ABS, 4 },{ "LDX", &a::LDX, &a::ABS, 4 },{ "???", &a::XXX, &a::IMP, 4 },
		{ "BCS", &a::BCS, &a::REL, 2 },{ "LDA", &a::LDA, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 5 },{ "LDY", &a::LDY, &a::ZPX, 4 },{ "LDA", &a::LDA, &a::ZPX, 4 },{ "LDX", &a::LDX, &a::ZPY, 4 },{ "???", &a::XXX, &a::IMP, 4 },{ "CLV", &a::CLV, &a::IMP, 2 },{ "LDA", &a::LDA, &a::ABY, 4 },{ "TSX", &a::TSX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 4 },{ "LDY", &a::LDY, &a::ABX, 4 },{ "LDA", &a::LDA, &a::ABX, 4 },{ "LDX", &a::LDX, &a::ABY, 4 },{ "???", &a::XXX, &a::IMP, 4 },
		{ "CPY", &a::CPY, &a::IMM, 2 },{ "CMP", &a::CMP, &a::IZX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "CPY", &a::CPY, &a::ZP0, 3 },{ "CMP", &a::CMP, &a::ZP0, 3 },{ "DEC", &a::DEC, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "INY", &a::INY, &a::IMP, 2 },{ "CMP", &a::CMP, &a::IMM, 2 },{ "DEX", &a::DEX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "CPY", &a::CPY, &a::ABS, 4 },{ "CMP", &a::CMP, &a::ABS, 4 },{ "DEC", &a::DEC, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BNE", &a::BNE, &a::REL, 2 },{ "CMP", &a::CMP, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "CMP", &a::CMP, &a::ZPX, 4 },{ "DEC", &a::DEC, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLD", &a::CLD, &a::IMP, 2 },{ "CMP", &a::CMP, &a::ABY, 4 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "CMP", &a::CMP, &a::ABX, 4 },{ "DEC", &a::DEC, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "CPX", &a::CPX, &a::IMM, 2 },{ "SBC", &a::SBC, &a::IZX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "CPX", &a::CPX, &a::ZP0, 3 },{ "SBC", &a::SBC, &a::ZP0, 3 },{ "INC", &a::INC, &a::ZP0, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "INX", &a::INX, &a::IMP, 2 },{ "SBC", &a::SBC, &a::IMM, 2 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::SBC, &a::IMP, 2 },{ "CPX", &a::CPX, &a::ABS, 4 },{ "SBC", &a::SBC, &a::ABS, 4 },{ "INC", &a::INC, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BEQ", &a::BEQ, &a::REL, 2 },{ "SBC", &a::SBC, &a::IZY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "SBC", &a::SBC, &a::ZPX, 4 },{ "INC", &a::INC, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SED", &a::SED, &a::IMP, 2 },{ "SBC", &a::SBC, &a::ABY, 4 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "SBC", &a::SBC, &a::ABX, 4 },{ "INC", &a::INC, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
	};
}
CPU6502::~CPU6502() {}

//MEMORY READ WRITE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void CPU6502::writeMemory(uint16_t address, uint8_t data) {
	bus->writeMemory(address, data);
}

uint8_t CPU6502::readMemory(uint16_t address, bool readOnly) {
	return bus->readMemory(address, false);
}




//FLAG GET SET
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint8_t CPU6502::getFlag(flags flag){
	//return ((status & flag) > 0) ? 1 : 0;
	//flag is enum in the format (1 << X)
	//if status reg position and (1 << X) != 0 them position X  = 1
	if ((status & flag) > 0) {
		return 1;
	}
	else {
		return 0;
	}

}

void CPU6502::setFlag(flags flag, bool v){
	if (v)
		status |= flag; //sets flag to 1
	else
		status &= ~flag; //reset flags to 0
}

//ADRESS MODE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//adressing modes
// Address Mode: Implied
// There is no additional data required for this instruction. The instruction
// does something very simple like like sets a status bit. However, we will
// target the accumulator, for instructions like PHA
uint8_t CPU6502::IMP(){
	fetchedData = regA;
	return 0;
}

// Address Mode: Immediate
// The instruction expects the next byte to be used as a value, so we'll prep
// the read address to point to the next byte
uint8_t CPU6502::IMM(){
	addressAbsolute = pc++;
	return 0;
}

// Address Mode: Zero Page
// To save program bytes, zero page addressing allows you to absolutely address
// a location in first 0xFF bytes of address range. Clearly this only requires
// one byte instead of the usual two.
uint8_t CPU6502::ZP0(){
	addressAbsolute = readMemory(pc);
	pc++;
	addressAbsolute &= 0x00FF;
	return 0;
}

// Address Mode: Zero Page with X Offset
uint8_t CPU6502::ZPX(){
	addressAbsolute = (readMemory(pc) + regX);
	pc++;
	addressAbsolute &= 0x00FF;
	return 0;
}

// Address Mode: Zero Page with Y Offset
uint8_t CPU6502::ZPY(){
	addressAbsolute = (readMemory(pc) + regY);
	pc++;
	addressAbsolute &= 0x00FF;
	return 0;
}

// Address Mode: Relative
// This address mode is exclusive to branch instructions. The address
// must reside within -128 to +127 of the branch instruction, i.e.
// you cant directly branch to any address in the addressable range.
uint8_t CPU6502::REL(){
	addressRelative = readMemory(pc);
	pc++;
	if (addressRelative & 0x80)
		addressRelative |= 0xFF00;
	return 0;
}

// Address Mode: Absolute 
// A full 16-bit address is loaded and used
uint8_t CPU6502::ABS(){
	uint16_t lo = readMemory(pc); //low
	pc++;
	uint16_t hi = readMemory(pc); //high
	pc++;

	addressAbsolute = (hi << 8) | lo;

	return 0;
}

// Address Mode: Absolute with X Offset
// Fundamentally the same as absolute addressing, but the contents of the X Register
// is added to the supplied two byte address. If the resulting address changes
// the page, an additional clock cycle is required
uint8_t CPU6502::ABX(){
	uint16_t lo = readMemory(pc);
	pc++;
	uint16_t hi = readMemory(pc);
	pc++;

	addressAbsolute = (hi << 8) | lo;
	addressAbsolute += regX;

	if ((addressAbsolute & 0xFF00) != (hi << 8))
		return 1;
	else
		return 0;
}

// Address Mode: Absolute with Y Offset
// Fundamentally the same as absolute addressing, but the contents of the Y Register
// is added to the supplied two byte address. If the resulting address changes
// the page, an additional clock cycle is required
uint8_t CPU6502::ABY(){
	uint16_t lo = readMemory(pc);
	pc++;
	uint16_t hi = readMemory(pc);
	pc++;

	addressAbsolute = (hi << 8) | lo;
	addressAbsolute += regY;

	if ((addressAbsolute & 0xFF00) != (hi << 8))
		return 1;
	else
		return 0;
}

// Note: The next 3 address modes use indirection (aka Pointers!)
// Address Mode: Indirect
// The supplied 16-bit address is read to get the actual 16-bit address. This is
// instruction is unusual in that it has a bug in the hardware! To emulate its
// function accurately, we also need to emulate this bug. If the low byte of the
// supplied address is 0xFF, then to read the high byte of the actual address
// we need to cross a page boundary. This doesnt actually work on the chip as 
// designed, instead it wraps back around in the same page, yielding an 
// invalid actual address
uint8_t CPU6502::IND(){
	uint16_t ptr_lo = readMemory(pc);
	pc++;
	uint16_t ptr_hi = readMemory(pc);
	pc++;

	uint16_t ptr = (ptr_hi << 8) | ptr_lo;

	if (ptr_lo == 0x00FF) // Simulate page boundary hardware bug
	{
		addressAbsolute = (readMemory(ptr & 0xFF00) << 8) | readMemory(ptr + 0);
	}
	else // Behave normally
	{
		addressAbsolute = (readMemory(ptr + 1) << 8) | readMemory(ptr + 0);
	}

	return 0;
}

// Address Mode: Indirect X
// The supplied 8-bit address is offset by X Register to index
// a location in page 0x00. The actual 16-bit address is read 
// from this location
uint8_t CPU6502::IZX(){
	uint16_t t = readMemory(pc);
	pc++;

	uint16_t lo = readMemory((uint16_t)(t + (uint16_t)regX) & 0x00FF);
	uint16_t hi = readMemory((uint16_t)(t + (uint16_t)regX + 1) & 0x00FF);

	addressAbsolute = (hi << 8) | lo;

	return 0;
}

// Address Mode: Indirect Y
// The supplied 8-bit address indexes a location in page 0x00. From 
// here the actual 16-bit address is read, and the contents of
// Y Register is added to it to offset it. If the offset causes a
// change in page then an additional clock cycle is required.
uint8_t CPU6502::IZY(){
	uint16_t t = readMemory(pc);
	pc++;

	uint16_t lo = readMemory(t & 0x00FF);
	uint16_t hi = readMemory((t + 1) & 0x00FF);

	addressAbsolute = (hi << 8) | lo;
	addressAbsolute += regY;

	if ((addressAbsolute & 0xFF00) != (hi << 8))
		return 1;
	else
		return 0;
}


//CLOCK RESET AND INTERRUPTION
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void CPU6502::clock() {
	if (cycles == 0) {
		opcode = readMemory(pc);
#ifdef LOGMODE
		uint16_t log_pc = pc;
#endif
		pc++;
		cycles = lut[opcode].cycles;
		uint8_t needMoreCycles0 = (this->*lut[opcode].addrmode)(); //call address mode
		uint8_t needMoreCycles1 = (this->*lut[opcode].operate)(); //call function
	
		cycles = cycles + (needMoreCycles1 & needMoreCycles0); //cycles increment only if both needMoreCyle return 1
		setFlag(U, true);

#ifdef LOGMODE
		// This logger dumps every cycle the entire processor state for analysis.
		// This can be used for debugging the emulation, but has little utility
		// during emulation. Its also very slow, so only use if you have to.
		if (logfile == nullptr)	logfile = fopen("CPU6502.txt", "wt");
		if (logfile != nullptr)
		{
			fprintf(logfile, "%10d:%02d PC:%04X %s A:%02X X:%02X Y:%02X %s%s%s%s%s%s%s%s STKP:%02X\n",
				clock_count, 0, log_pc, "XXX", a, x, y,
				getFlag(N) ? "N" : ".", getFlag(V) ? "V" : ".", getFlag(U) ? "U" : ".",
				getFlag(B) ? "B" : ".", getFlag(D) ? "D" : ".", getFlag(I) ? "I" : ".",
				getFlag(Z) ? "Z" : ".", getFlag(C) ? "C" : ".", stkp);
		}
#endif


	}
	globalClock++;
	cycles--;
}

void CPU6502::reset() {
	regA = 0x00;
	regX = 0x00;
	regY = 0x00;
	stkp = 0x00;
	status = 0x00 | U;

	addressAbsolute = 0xFFFC;
	uint16_t lo = readMemory(addressAbsolute + 0);
	uint16_t hi = readMemory(addressAbsolute + 1);

	pc = (hi << 8) | lo;
	addressRelative = 0x0000;
	addressAbsolute = 0x0000;
	fetchedData = 0x00;

	cycles = 8;
}

void CPU6502::interupt() {
	if (getFlag(I) == 0) {
		writeMemory(0x0100 + stkp, (pc >> 8) & 0x00FF);
		stkp--;
		writeMemory(0x0100 + stkp, pc & 0x00FF);
		stkp--;

		setFlag(B, 0);
		setFlag(U, 1);
		setFlag(I, 1);
		writeMemory(0x0100 + stkp, status);
		stkp--;

		addressAbsolute = 0xFFFE;
		uint16_t lo = readMemory(addressAbsolute + 0);
		uint16_t hi = readMemory(addressAbsolute + 1);
		pc = (hi << 8) | lo;

		cycles = 7;
	}
}

void CPU6502::nonMaskInt() {

	writeMemory(0x0100 + stkp, (pc >> 8) & 0x00FF);
	stkp--;
	writeMemory(0x0100 + stkp, pc & 0x00FF);
	stkp--;

	setFlag(B, 0);
	setFlag(U, 1);
	setFlag(I, 1);
	writeMemory(0x0100 + stkp, status);
	stkp--;

	addressAbsolute = 0xFFFA;
	uint16_t lo = readMemory(addressAbsolute + 0);
	uint16_t hi = readMemory(addressAbsolute + 1);
	pc = (hi << 8) | lo;

	cycles = 8;
}


//INSTRUCTIONS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint8_t CPU6502::fetchData() {
	if ( !(lut[opcode].addrmode == &CPU6502::IMP) ) {
		fetchedData = readMemory(addressAbsolute);
	}
	return fetchedData;
}

//INSTRUCTIONS

uint8_t CPU6502::ADC(){
	// Grab the data that we are adding to the accumulator
	fetchData();

	// Add is performed in 16-bit domain for emulation to capture any
	// carry bit, which will exist in bit 8 of the 16-bit word
	aux = (uint16_t)regA + (uint16_t)fetchedData + (uint16_t)getFlag(C);

	//setting flags
	// The carry flag out exists in the high byte bit 0
	setFlag(C, aux > 255); 
	setFlag(Z, (aux & 0x00FF) == 0);
	setFlag(V, (~((uint16_t)regA ^ (uint16_t)fetchedData) & ((uint16_t)regA ^ (uint16_t)aux)) & 0x0080);
	setFlag(N, aux & 0x80);

	// Load the result into the accumulator (it's 8-bit dont forget!)
	regA = aux & 0x00FF;

	// This instruction has the potential to require an additional clock cycle
	return 1;
}


uint8_t CPU6502::SBC(){
	fetchData();

	// Operating in 16-bit domain to capture carry out

	// We can invert the bottom 8 bits with bitwise xor
	uint16_t value = ((uint16_t)fetchedData) ^ 0x00FF;

	// Notice this is exactly the same as addition from here!
	aux = (uint16_t)regA + value + (uint16_t)getFlag(C);
	setFlag(C, aux & 0xFF00);
	setFlag(Z, ((aux & 0x00FF) == 0));
	setFlag(V, (aux ^ (uint16_t)regA) & (aux ^ value) & 0x0080);
	setFlag(N, aux & 0x0080);
	regA = aux & 0x00FF;
	return 1;
}


// Instruction: Bitwise Logic AND
// Function:    A = A & M
// Flags Out:   N, Z
uint8_t CPU6502::AND(){
	fetchData();
	regA = regA & fetchedData;
	setFlag(Z, regA == 0x00);
	setFlag(N, regA & 0x80);
	return 1;
}


// Instruction: Arithmetic Shift Left
// Function:    A = C <- (A << 1) <- 0
// Flags Out:   N, Z, C
uint8_t CPU6502::ASL(){
	fetchData();
	aux = (uint16_t)fetchedData << 1;
	setFlag(C, (aux & 0xFF00) > 0);
	setFlag(Z, (aux & 0x00FF) == 0x00);
	setFlag(N, aux & 0x80);
	if (lut[opcode].addrmode == &CPU6502::IMP)
		regA = aux & 0x00FF;
	else
		writeMemory(addressAbsolute, aux & 0x00FF);
	return 0;
}


// Instruction: Branch if Carry Clear
// Function:    if(C == 0) pc = address 
uint8_t CPU6502::BCC(){
	if (getFlag(C) == 0)
	{
		cycles++;
		addressAbsolute = pc + addressRelative;

		if ((addressAbsolute & 0xFF00) != (pc & 0xFF00))
			cycles++;

		pc = addressAbsolute;
	}
	return 0;
}


// Instruction: Branch if Carry Set
// Function:    if(C == 1) pc = address
uint8_t CPU6502::BCS(){
	if (getFlag(C) == 1)
	{
		cycles++;
		addressAbsolute = pc + addressRelative;

		if ((addressAbsolute & 0xFF00) != (pc & 0xFF00))
			cycles++;

		pc = addressAbsolute;
	}
	return 0;
}


// Instruction: Branch if Equal
// Function:    if(Z == 1) pc = address
uint8_t CPU6502::BEQ(){
	if (getFlag(Z) == 1)
	{
		cycles++;
		addressAbsolute = pc + addressRelative;

		if ((addressAbsolute & 0xFF00) != (pc & 0xFF00))
			cycles++;

		pc = addressAbsolute;
	}
	return 0;
}

uint8_t CPU6502::BIT(){
	fetchData();
	aux = regA & fetchedData;
	setFlag(Z, (aux & 0x00FF) == 0x00);
	setFlag(N, fetchedData & (1 << 7));
	setFlag(V, fetchedData & (1 << 6));
	return 0;
}


// Instruction: Branch if Negative
// Function:    if(N == 1) pc = address
uint8_t CPU6502::BMI(){
	if (getFlag(N) == 1)
	{
		cycles++;
		addressAbsolute = pc + addressRelative;

		if ((addressAbsolute & 0xFF00) != (pc & 0xFF00))
			cycles++;

		pc = addressAbsolute;
	}
	return 0;
}


// Instruction: Branch if Not Equal
// Function:    if(Z == 0) pc = address
uint8_t CPU6502::BNE(){
	if (getFlag(Z) == 0)
	{
		cycles++;
		addressAbsolute = pc + addressRelative;

		if ((addressAbsolute & 0xFF00) != (pc & 0xFF00))
			cycles++;

		pc = addressAbsolute;
	}
	return 0;
}


// Instruction: Branch if Positive
// Function:    if(N == 0) pc = address
uint8_t CPU6502::BPL(){
	if (getFlag(N) == 0)
	{
		cycles++;
		addressAbsolute = pc + addressRelative;

		if ((addressAbsolute & 0xFF00) != (pc & 0xFF00))
			cycles++;

		pc = addressAbsolute;
	}
	return 0;
}

// Instruction: Break
// Function:    Program Sourced Interrupt
uint8_t CPU6502::BRK(){
	pc++;

	setFlag(I, 1);
	writeMemory(0x0100 + stkp, (pc >> 8) & 0x00FF);
	stkp--;
	writeMemory(0x0100 + stkp, pc & 0x00FF);
	stkp--;

	setFlag(B, 1);
	writeMemory(0x0100 + stkp, status);
	stkp--;
	setFlag(B, 0);

	pc = (uint16_t)readMemory(0xFFFE) | ((uint16_t)readMemory(0xFFFF) << 8);
	return 0;
}


// Instruction: Branch if Overflow Clear
// Function:    if(V == 0) pc = address
uint8_t CPU6502::BVC(){
	if (getFlag(V) == 0)
	{
		cycles++;
		addressAbsolute = pc + addressRelative;

		if ((addressAbsolute & 0xFF00) != (pc & 0xFF00))
			cycles++;

		pc = addressAbsolute;
	}
	return 0;
}


// Instruction: Branch if Overflow Set
// Function:    if(V == 1) pc = address
uint8_t CPU6502::BVS(){
	if (getFlag(V) == 1)
	{
		cycles++;
		addressAbsolute = pc + addressRelative;

		if ((addressAbsolute & 0xFF00) != (pc & 0xFF00))
			cycles++;

		pc = addressAbsolute;
	}
	return 0;
}


// Instruction: Clear Carry Flag
// Function:    C = 0
uint8_t CPU6502::CLC(){
	setFlag(C, false);
	return 0;
}


// Instruction: Clear Decimal Flag
// Function:    D = 0
uint8_t CPU6502::CLD(){
	setFlag(D, false);
	return 0;
}


// Instruction: Disable Interrupts / Clear Interrupt Flag
// Function:    I = 0
uint8_t CPU6502::CLI(){
	setFlag(I, false);
	return 0;
}


// Instruction: Clear Overflow Flag
// Function:    V = 0
uint8_t CPU6502::CLV(){
	setFlag(V, false);
	return 0;
}

// Instruction: Compare Accumulator
// Function:    C <- A >= M      Z <- (A - M) == 0
// Flags Out:   N, C, Z
uint8_t CPU6502::CMP(){
	fetchData();
	aux = (uint16_t)regA - (uint16_t)fetchedData;
	setFlag(C, regA >= fetchedData);
	setFlag(Z, (aux & 0x00FF) == 0x0000);
	setFlag(N, aux & 0x0080);
	return 1;
}


// Instruction: Compare X Register
// Function:    C <- X >= M      Z <- (X - M) == 0
// Flags Out:   N, C, Z
uint8_t CPU6502::CPX(){
	fetchData();
	aux = (uint16_t)regX - (uint16_t)fetchedData;
	setFlag(C, regX >= fetchedData);
	setFlag(Z, (aux & 0x00FF) == 0x0000);
	setFlag(N, aux & 0x0080);
	return 0;
}


// Instruction: Compare Y Register
// Function:    C <- Y >= M      Z <- (Y - M) == 0
// Flags Out:   N, C, Z
uint8_t CPU6502::CPY(){
	fetchData();
	aux = (uint16_t)regY - (uint16_t)fetchedData;
	setFlag(C, regY >= fetchedData);
	setFlag(Z, (aux & 0x00FF) == 0x0000);
	setFlag(N, aux & 0x0080);
	return 0;
}


// Instruction: Decrement Value at Memory Location
// Function:    M = M - 1
// Flags Out:   N, Z
uint8_t CPU6502::DEC(){
	fetchData();
	aux = fetchedData - 1;
	writeMemory(addressAbsolute, aux & 0x00FF);
	setFlag(Z, (aux & 0x00FF) == 0x0000);
	setFlag(N, aux & 0x0080);
	return 0;
}


// Instruction: Decrement X Register
// Function:    X = X - 1
// Flags Out:   N, Z
uint8_t CPU6502::DEX(){
	regX--;
	setFlag(Z, regX == 0x00);
	setFlag(N, regX & 0x80);
	return 0;
}


// Instruction: Decrement Y Register
// Function:    Y = Y - 1
// Flags Out:   N, Z
uint8_t CPU6502::DEY(){
	regY--;
	setFlag(Z, regY == 0x00);
	setFlag(N, regY & 0x80);
	return 0;
}


// Instruction: Bitwise Logic XOR
// Function:    A = A xor M
// Flags Out:   N, Z
uint8_t CPU6502::EOR(){
	fetchData();
	regA = regA ^ fetchedData;
	setFlag(Z, regA == 0x00);
	setFlag(N, regA & 0x80);
	return 1;
}


// Instruction: Increment Value at Memory Location
// Function:    M = M + 1
// Flags Out:   N, Z
uint8_t CPU6502::INC(){
	fetchData();
	aux = fetchedData + 1;
	writeMemory(addressAbsolute, aux & 0x00FF);
	setFlag(Z, (aux & 0x00FF) == 0x0000);
	setFlag(N, aux & 0x0080);
	return 0;
}


// Instruction: Increment X Register
// Function:    X = X + 1
// Flags Out:   N, Z
uint8_t CPU6502::INX(){
	regX++;
	setFlag(Z, regX == 0x00);
	setFlag(N, regX & 0x80);
	return 0;
}


// Instruction: Increment Y Register
// Function:    Y = Y + 1
// Flags Out:   N, Z
uint8_t CPU6502::INY(){
	regY++;
	setFlag(Z, regY == 0x00);
	setFlag(N, regY & 0x80);
	return 0;
}


// Instruction: Jump To Location
// Function:    pc = address
uint8_t CPU6502::JMP(){
	pc = addressAbsolute;
	return 0;
}


// Instruction: Jump To Sub-Routine
// Function:    Push current pc to stack, pc = address
uint8_t CPU6502::JSR(){
	pc--;

	writeMemory(0x0100 + stkp, (pc >> 8) & 0x00FF);
	stkp--;
	writeMemory(0x0100 + stkp, pc & 0x00FF);
	stkp--;

	pc = addressAbsolute;
	return 0;
}


// Instruction: Load The Accumulator
// Function:    A = M
// Flags Out:   N, Z
uint8_t CPU6502::LDA(){
	fetchData();
	regA = fetchedData;
	setFlag(Z, regA == 0x00);
	setFlag(N, regA & 0x80);
	return 1;
}


// Instruction: Load The X Register
// Function:    X = M
// Flags Out:   N, Z
uint8_t CPU6502::LDX(){
	fetchData();
	regX = fetchedData;
	setFlag(Z, regX == 0x00);
	setFlag(N, regX & 0x80);
	return 1;
}


// Instruction: Load The Y Register
// Function:    Y = M
// Flags Out:   N, Z
uint8_t CPU6502::LDY(){
	fetchData();
	regY = fetchedData;
	setFlag(Z, regY == 0x00);
	setFlag(N, regY & 0x80);
	return 1;
}

uint8_t CPU6502::LSR(){
	fetchData();
	setFlag(C, fetchedData & 0x0001);
	aux = fetchedData >> 1;
	setFlag(Z, (aux & 0x00FF) == 0x0000);
	setFlag(N, aux & 0x0080);
	if (lut[opcode].addrmode == &CPU6502::IMP)
		regA = aux & 0x00FF;
	else
		writeMemory(addressAbsolute, aux & 0x00FF);
	return 0;
}

uint8_t CPU6502::NOP(){
	switch (opcode) {
	case 0x1C:
	case 0x3C:
	case 0x5C:
	case 0x7C:
	case 0xDC:
	case 0xFC:
		return 1;
		break;
	}
	return 0;
}


// Instruction: Bitwise Logic OR
// Function:    A = A | M
// Flags Out:   N, Z
uint8_t CPU6502::ORA(){
	fetchData();
	regA = regA | fetchedData;
	setFlag(Z, regA == 0x00);
	setFlag(N, regA & 0x80);
	return 1;
}


// Instruction: Push Accumulator to Stack
// Function:    A -> stack
uint8_t CPU6502::PHA(){
	writeMemory(0x0100 + stkp, regA);
	stkp--;
	return 0;
}


// Instruction: Push Status Register to Stack
// Function:    status -> stack
// Note:        Break flag is set to 1 before push
uint8_t CPU6502::PHP(){
	writeMemory(0x0100 + stkp, status | B | U);
	setFlag(B, 0);
	setFlag(U, 0);
	stkp--;
	return 0;
}


// Instruction: Pop Accumulator off Stack
// Function:    A <- stack
// Flags Out:   N, Z
uint8_t CPU6502::PLA(){
	stkp++;
	regA = readMemory(0x0100 + stkp);
	setFlag(Z, regA == 0x00);
	setFlag(N, regA & 0x80);
	return 0;
}


// Instruction: Pop Status Register off Stack
// Function:    Status <- stack
uint8_t CPU6502::PLP(){
	stkp++;
	status = readMemory(0x0100 + stkp);
	setFlag(U, 1);
	return 0;
}

uint8_t CPU6502::ROL(){
	fetchData();
	aux = (uint16_t)(fetchedData << 1) | getFlag(C);
	setFlag(C, aux & 0xFF00);
	setFlag(Z, (aux & 0x00FF) == 0x0000);
	setFlag(N, aux & 0x0080);
	if (lut[opcode].addrmode == &CPU6502::IMP)
		regA = aux & 0x00FF;
	else
		writeMemory(addressAbsolute, aux & 0x00FF);
	return 0;
}

uint8_t CPU6502::ROR(){
	fetchData();
	aux = (uint16_t)(getFlag(C) << 7) | (fetchedData >> 1);
	setFlag(C, fetchedData & 0x01);
	setFlag(Z, (aux & 0x00FF) == 0x00);
	setFlag(N, aux & 0x0080);
	if (lut[opcode].addrmode == &CPU6502::IMP)
		regA = aux & 0x00FF;
	else
		writeMemory(addressAbsolute, aux & 0x00FF);
	return 0;
}

//return from interruption
uint8_t CPU6502::RTI(){
	stkp++;
	status = readMemory(0x0100 + stkp);
	status &= ~B;
	status &= ~U;

	stkp++;
	pc = (uint16_t)readMemory(0x0100 + stkp);
	stkp++;
	pc |= (uint16_t)readMemory(0x0100 + stkp) << 8;
	return 0;
}

uint8_t CPU6502::RTS(){
	stkp++;
	pc = (uint16_t)readMemory(0x0100 + stkp);
	stkp++;
	pc |= (uint16_t)readMemory(0x0100 + stkp) << 8;

	pc++;
	return 0;
}

// Instruction: Set Carry Flag
// Function:    C = 1
uint8_t CPU6502::SEC(){
	setFlag(C, true);
	return 0;
}

// Instruction: Set Decimal Flag
// Function:    D = 1
uint8_t CPU6502::SED(){
	setFlag(D, true);
	return 0;
}

// Instruction: Set Interrupt Flag / Enable Interrupts
// Function:    I = 1
uint8_t CPU6502::SEI(){
	setFlag(I, true);
	return 0;
}

// Instruction: Store Accumulator at Address
// Function:    M = A
uint8_t CPU6502::STA(){
	writeMemory(addressAbsolute, regA);
	return 0;
}

// Instruction: Store X Register at Address
// Function:    M = X
uint8_t CPU6502::STX(){
	writeMemory(addressAbsolute, regX);
	return 0;
}

// Instruction: Store Y Register at Address
// Function:    M = Y
uint8_t CPU6502::STY(){
	writeMemory(addressAbsolute, regY);
	return 0;
}

// Instruction: Transfer Accumulator to X Register
// Function:    X = A
// Flags Out:   N, Z
uint8_t CPU6502::TAX(){
	regX = regA;
	setFlag(Z, regX == 0x00);
	setFlag(N, regX & 0x80);
	return 0;
}

// Instruction: Transfer Accumulator to Y Register
// Function:    Y = A
// Flags Out:   N, Z
uint8_t CPU6502::TAY(){
	regY = regA;
	setFlag(Z, regY == 0x00);
	setFlag(N, regY & 0x80);
	return 0;
}

// Instruction: Transfer Stack Pointer to X Register
// Function:    X = stack pointer
// Flags Out:   N, Z
uint8_t CPU6502::TSX(){
	regX = stkp;
	setFlag(Z, regX == 0x00);
	setFlag(N, regX & 0x80);
	return 0;
}

// Instruction: Transfer X Register to Accumulator
// Function:    A = X
// Flags Out:   N, Z
uint8_t CPU6502::TXA(){
	regA = regX;
	setFlag(Z, regA == 0x00);
	setFlag(N, regA & 0x80);
	return 0;
}

// Instruction: Transfer X Register to Stack Pointer
// Function:    stack pointer = X
uint8_t CPU6502::TXS(){
	stkp = regX;
	return 0;
}

// Instruction: Transfer Y Register to Accumulator
// Function:    A = Y
// Flags Out:   N, Z
uint8_t CPU6502::TYA(){
	regA = regY;
	setFlag(Z, regA == 0x00);
	setFlag(N, regA & 0x80);
	return 0;
}

// This function captures illegal opcodes
uint8_t CPU6502::XXX(){
	return 0;
}



//DEBUG
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CPU6502::complete()
{
	return cycles == 0;
}

// This is the disassembly function. Its workings are not required for emulation.
// It is merely a convenience function to turn the binary instruction code into
// human readable form. Its included as part of the emulator because it can take
// advantage of many of the CPUs internal operations to do this.
std::map<uint16_t, std::string> CPU6502::disassemble(uint16_t nStart, uint16_t nStop)
{
	uint32_t addr = nStart;
	uint8_t value = 0x00, lo = 0x00, hi = 0x00;
	std::map<uint16_t, std::string> mapLines;
	uint16_t line_addr = 0;

	// A convenient utility to convert variables into
	// hex strings because "modern C++"'s method with 
	// streams is atrocious
	auto hex = [](uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	// Starting at the specified address we read an instruction
	// byte, which in turn yields information from the lookup table
	// as to how many additional bytes we need to read and what the
	// addressing mode is. I need this info to assemble human readable
	// syntax, which is different depending upon the addressing mode

	// As the instruction is decoded, a std::string is assembled
	// with the readable output
	while (addr <= (uint32_t)nStop)
	{
		line_addr = addr;

		// Prefix line with instruction address
		std::string sInst = "$" + hex(addr, 4) + ": ";

		// Read instruction, and get its readable name
		uint8_t opcode = bus->readMemory(addr, true); addr++;
		sInst += lut[opcode].name + " ";

		// Get oprands from desired locations, and form the
		// instruction based upon its addressing mode. These
		// routines mimmick the actual fetch routine of the
		// 6502 in order to get accurate data as part of the
		// instruction
		if (lut[opcode].addrmode == &CPU6502::IMP)
		{
			sInst += " {IMP}";
		}
		else if (lut[opcode].addrmode == &CPU6502::IMM)
		{
			value = bus->readMemory(addr, true); addr++;
			sInst += "#$" + hex(value, 2) + " {IMM}";
		}
		else if (lut[opcode].addrmode == &CPU6502::ZP0)
		{
			lo = bus->readMemory(addr, true); addr++;
			hi = 0x00;
			sInst += "$" + hex(lo, 2) + " {ZP0}";
		}
		else if (lut[opcode].addrmode == &CPU6502::ZPX)
		{
			lo = bus->readMemory(addr, true); addr++;
			hi = 0x00;
			sInst += "$" + hex(lo, 2) + ", X {ZPX}";
		}
		else if (lut[opcode].addrmode == &CPU6502::ZPY)
		{
			lo = bus->readMemory(addr, true); addr++;
			hi = 0x00;
			sInst += "$" + hex(lo, 2) + ", Y {ZPY}";
		}
		else if (lut[opcode].addrmode == &CPU6502::IZX)
		{
			lo = bus->readMemory(addr, true); addr++;
			hi = 0x00;
			sInst += "($" + hex(lo, 2) + ", X) {IZX}";
		}
		else if (lut[opcode].addrmode == &CPU6502::IZY)
		{
			lo = bus->readMemory(addr, true); addr++;
			hi = 0x00;
			sInst += "($" + hex(lo, 2) + "), Y {IZY}";
		}
		else if (lut[opcode].addrmode == &CPU6502::ABS)
		{
			lo = bus->readMemory(addr, true); addr++;
			hi = bus->readMemory(addr, true); addr++;
			sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + " {ABS}";
		}
		else if (lut[opcode].addrmode == &CPU6502::ABX)
		{
			lo = bus->readMemory(addr, true); addr++;
			hi = bus->readMemory(addr, true); addr++;
			sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", X {ABX}";
		}
		else if (lut[opcode].addrmode == &CPU6502::ABY)
		{
			lo = bus->readMemory(addr, true); addr++;
			hi = bus->readMemory(addr, true); addr++;
			sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", Y {ABY}";
		}
		else if (lut[opcode].addrmode == &CPU6502::IND)
		{
			lo = bus->readMemory(addr, true); addr++;
			hi = bus->readMemory(addr, true); addr++;
			sInst += "($" + hex((uint16_t)(hi << 8) | lo, 4) + ") {IND}";
		}
		else if (lut[opcode].addrmode == &CPU6502::REL)
		{
			value = bus->readMemory(addr, true); addr++;
			sInst += "$" + hex(value, 2) + " [$" + hex(addr + value, 4) + "] {REL}";
		}

		// Add the formed string to a std::map, using the instruction's
		// address as the key. This makes it convenient to look for later
		// as the instructions are variable in length, so a straight up
		// incremental index is not sufficient.
		mapLines[line_addr] = sInst;
	}

	return mapLines;
}
