#include "Bus.h"

Bus::Bus() {
	for (auto& address : ram) {
		address = 0x00;
	}
	cpu.connectBus(this);
}

Bus::~Bus() {
}

void Bus::writeMemory(uint16_t address, uint8_t data) {
	if (address >= 0x0000 && address <= 0xFFFF) {
		ram[address] = data;
	}
}

uint8_t Bus::readMemory(uint16_t address, bool readOnly) {
	if (address >= 0x0000 && address <= 0xFFFF) {
		return ram[address];
	}
	return 0x00;
}