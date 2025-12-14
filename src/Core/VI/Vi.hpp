#ifndef VI_HPP
#define VI_HPP

#include <cstdint>

class Vi
{
	private:
		uint32_t tfbl;
	public:
		uint32_t read32(uint32_t address);
		void write16(uint32_t address, uint16_t value);
		void write32(uint32_t address, uint32_t value);
};

#endif
