#include <cstdint>
#include <string.h>

#include "./Mmu.hpp"
#include "../Endian.hpp"

void Mmu::init(uint8_t* memoryBufferPointer)
{
	this->memoryBufferPointer = memoryBufferPointer;
};

void Mmu::copy(uint32_t address, uint8_t* from, size_t noBytes)
{
	uint32_t offset = 0;
	if (address>0x80000000 && address<0x817fffff)
	{
		offset = address - 0x80000000;
	} else if (address>0xC0000000 && address<0xC17fffff)
	{
		offset = address - 0xC0000000;
	};
	memcpy(memoryBufferPointer+offset, from, noBytes);
};

uint8_t Mmu::readByte(uint32_t address)
{
	uint32_t offset;
	if (address>0x80000000 && address<0x817fffff)
        {
                offset = address - 0x80000000;
        } else if (address>0xC0000000 && address<0xC17fffff)
        {
                offset = address - 0xC0000000;
        };
	return *(memoryBufferPointer+offset);
};

uint32_t Mmu::read32(uint32_t address)
{
        uint32_t offset;
        if (address>0x80000000 && address<0x817fffff)
        {
                offset = address - 0x80000000;
        } else if (address>0xC0000000 && address<0xC17fffff)
        {
                offset = address - 0xC0000000;
        };
	uint32_t result;
	memcpy(&result, (memoryBufferPointer + offset), sizeof(uint32_t));
	result = bigEndian(result);
	return result;
};

void Mmu::fill(uint32_t address, uint8_t value, size_t noBytes)
{
	uint32_t offset;
        if (address>0x80000000 && address<0x817fffff)
        {
                offset = address - 0x80000000;
        } else if (address>0xC0000000 && address<0xC17fffff)
        {
                offset = address - 0xC0000000;
        };
	for (size_t i = 0; i<noBytes; i++)
	{
		*(memoryBufferPointer+offset) = value;
	};
};
