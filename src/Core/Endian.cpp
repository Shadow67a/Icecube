#include <cstdint>

#include "./Endian.hpp"

uint32_t bigEndian(uint32_t smallEndian)
{
	return __builtin_bswap32(smallEndian);
};

uint32_t smallEndian(uint32_t bigEndian)
{
	return __builtin_bswap32(bigEndian);
};
