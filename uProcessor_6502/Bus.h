#pragma once
#include <cstdint>
#include <array>
#include "cpu6502.h"

class Bus
{

private:

public:
	Bus();
	~Bus();
	void writeMemory(uint16_t address, uint8_t data);
	uint8_t readMemory(uint16_t address, bool readOnly = false);

	CPU6502 cpu;
	std::array<uint8_t, 64 * 1024> ram;


};

