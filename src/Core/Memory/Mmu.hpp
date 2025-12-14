#ifndef MMU_HPP
#define MMU_HPP

#include <cstdint>
#include <stdlib.h>

#include "../VI/Vi.hpp"

class Mmu
{
	private:
		uint8_t* memoryBufferPointer;
		Vi* viInstance;
	public:
		void init(uint8_t* memoryBufferPointer);
		void addDevice(Vi* vi);
		void copy(uint32_t address, uint8_t* from, size_t noBytes);
		uint8_t readByte(uint32_t address);
		static uint32_t read32(uint32_t address);
		static void write32(uint32_t address, uint32_t value);
		static void write16(uint32_t address, uint16_t value);
		void fill(uint32_t address, uint8_t value, size_t noBytes);
};

extern Mmu* mmuInstance;

#endif
