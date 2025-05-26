#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <array>
#include <vector>
#include <stdlib.h>
#include <unordered_map>

#include "../Memory/Mmu.hpp"

#define GECKO_BOOT_OFFSET 0x100

#define SPR_HID0 1008
#define SPR_HID1 1009
#define SPR_HID2 920
#define SPR_PVR 287
#define SPR_DEC 22

#define CODEBUFSIZE 67108864

#define U64(x) (x & 0x00000000000000ff), ((x & 0x000000000000ff00) >> 8), ((x & 0x0000000000ff0000) >> 16), ((x & 0x00000000ff000000) >> 24), ((x & 0x000000ff00000000) >> 32), ((x & 0x0000ff0000000000) >> 40), ((x & 0x00ff000000000000) >> 48), ((x & 0xff00000000000000) >> 56)
#define U32(x) (x & 0x00000000000000ff), ((x & 0x000000000000ff00) >> 8), ((x & 0x0000000000ff0000) >> 16), ((x & 0x00000000ff000000) >> 24)

#define NEXT() \
    do { \
	nextPC += 4; \
        uint8_t _next[] = {0x48, 0xba, U64(nia), 0x81, 0x02, U32(0x4)}; \
        code.insert(code.end(), std::begin(_next), std::end(_next)); \
    } while (0)

struct JITBlock
{
	void* code;
//	uint32_t nextPC;
};

class Cpu
{
	public:
		uint32_t pc;
		uint32_t nextPC;
		uint32_t msr;
		uint32_t fpscr;
                uint32_t system_exception_vector;

		std::array<uint32_t, 32> gpr;
		std::array<uint32_t, 1024> spr;
		std::array<uint32_t, 16> sr;
		std::array<uint32_t, 32> fpr;

		Mmu mmu;

		void* codeBuffer;
		size_t codeSize;
		std::unordered_map<uint32_t, JITBlock> JITBlocks;

		void JITCompile();
		void insertCode(uint8_t* code, size_t size);
		void msrUpdated();
		void init(uint8_t* memoryBufferPointer);
		void execute();
};

#endif
