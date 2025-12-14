#include <iostream>
#include <cstdint>
#include <string.h>

#include "./Mmu.hpp"
#include "../Endian.hpp"
#include "../VI/Vi.hpp"

Mmu* mmuInstance = nullptr;

void Mmu::init(uint8_t* memoryBufferPointer)
{
	mmuInstance = this;
	this->memoryBufferPointer = memoryBufferPointer;
};

void Mmu::addDevice(Vi* vi)
{
	this->viInstance = vi;
};

void Mmu::copy(uint32_t address, uint8_t* from, size_t noBytes)
{
	uint32_t offset = 0;
	if (address>=0x80000000 && address<=0x817fffff)
	{
		offset = address - 0x80000000;
	} else if (address>=0xC0000000 && address<=0xC17fffff)
	{
		offset = address - 0xC0000000;
	} else {
                std::cout << "[Mmu::copy]: out of range!" << std::endl;
                exit(0);
        };
	memcpy(memoryBufferPointer+offset, from, noBytes);
};

uint8_t Mmu::readByte(uint32_t address)
{
	uint32_t offset = 0;
	if (address>=0x80000000 && address<=0x817fffff)
        {
                offset = address - 0x80000000;
        } else if (address>=0xC0000000 && address<=0xC17fffff)
        {
                offset = address - 0xC0000000;
        } else {
                std::cout << "[Mmu::readByte]: out of range!" << std::endl;
                exit(0);
        };
	return *(memoryBufferPointer+offset);
};

uint32_t Mmu::read32(uint32_t address)
{
	uint32_t result;
        if (address>=0x80000000 && address<=0x817fffff)
        {
                uint32_t offset = address - 0x80000000;
		memcpy(&result, (mmuInstance->memoryBufferPointer + offset), sizeof(uint32_t));
		result = bigEndian(result);
        } else if (address>=0xC0000000 && address<=0xC17fffff)
        {
                uint32_t offset = address - 0xC0000000;
		memcpy(&result, (mmuInstance->memoryBufferPointer + offset), sizeof(uint32_t));
        	result = bigEndian(result);
        } else if (address>=0xCC002000 && address<=0xCC002100)
	{
		result = mmuInstance->viInstance->read32(address);
	} else {
                std::cout << "[Mmu::read32]: out of range! at address: 0x" << std::hex << address << std::endl;
                exit(0);
        };
	return result;
};

void Mmu::write32(uint32_t address, uint32_t value)
{
	std::cout << "[Mmu::write32]: address: 0x" << std::hex << address << " value: 0x" <<  std::hex << value << std::endl;
        if (address>=0x80000000 && address<=0x817fffff)
        {
                uint32_t offset = address - 0x80000000;
	        uint32_t data = bigEndian(value);
	        memcpy((mmuInstance->memoryBufferPointer + offset), &data, sizeof(uint32_t));
        } else if (address>=0xC0000000 && address<=0xC17fffff)
        {
                uint32_t offset = address - 0xC0000000;
                uint32_t data = bigEndian(value);
                memcpy((mmuInstance->memoryBufferPointer + offset), &data, sizeof(uint32_t));
        } else if (address>=0xCC002000 && address<=0xCC002100)
	{
		mmuInstance->viInstance->write32(address, value);
	} else {
                std::cout << "[Mmu::write32]: out of range!" << std::endl;
                exit(0);
        };
};

void Mmu::write16(uint32_t address, uint16_t value)
{
        std::cout << "[Mmu::write16]: address: 0x" << std::hex << address << " value: 0x" <<  std::hex << value << std::endl;
        uint32_t offset = 0;
        if (address>=0x80000000 && address<=0x817fffff)
        {
                offset = address - 0x80000000;
	        uint16_t data = bigEndian(value);
        	memcpy((mmuInstance->memoryBufferPointer + offset), &data, sizeof(uint16_t));
        } else if (address>=0xC0000000 && address<=0xC17fffff)
        {
                offset = address - 0xC0000000;
	        uint16_t data = bigEndian(value);
	        memcpy((mmuInstance->memoryBufferPointer + offset), &data, sizeof(uint16_t));
        } else if (address>=0xCC002000 && address<=0xCC002100)
	{
		mmuInstance->viInstance->write16(address, value);
	} else {
		std::cout << "[Mmu::write16]: out of range!" << std::endl;
		exit(0);
	};
};

void Mmu::fill(uint32_t address, uint8_t value, size_t noBytes)
{
	uint32_t offset = 0;
        if (address>=0x80000000 && address<=0x817fffff)
        {
                offset = address - 0x80000000;
        } else if (address>=0xC0000000 && address<=0xC17fffff)
        {
                offset = address - 0xC0000000;
        }  else {
                std::cout << "[Mmu::fill]: out of range!" << std::endl;
                exit(0);
        };
	for (size_t i = 0; i<noBytes; i++)
	{
		*(memoryBufferPointer+offset) = value;
	};
};
