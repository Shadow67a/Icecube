#ifndef IPL_HPP
#define IPL_HPP

#include <stdlib.h>
#include <vector>
#include <cstdint>

#define COPYRIGHT_OFFSET 0x00000000
#define BIOS_OFFSET 0x00000100

#define COPYRIGHT_SIZE 0x00000100
#define BIOS_SIZE 0x001aede8

class Ipl
{
	private:
		std::size_t size;
		std::vector<uint8_t> data;
	public:
		void load(const char* path);
		void showContents();
};

#endif
