#ifndef MMU_HPP
#define MMU_HPP

#include <cstdint>
#include <stdlib.h>

class Mmu
{
	private:
		uint8_t* memoryBufferPointer;
	public:
		void init(uint8_t* memoryBufferPointer);
		void copy(uint32_t address, uint8_t* from, size_t noBytes);
		uint8_t readByte(uint32_t address);
		uint32_t read32(uint32_t address);
		void fill(uint32_t address, uint8_t value, size_t noBytes);
};

#endif
