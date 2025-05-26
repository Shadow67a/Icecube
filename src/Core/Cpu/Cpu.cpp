#include <sys/mman.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string.h>

#include "./Cpu.hpp"
#include "../Memory/Mmu.hpp"

void Cpu::init(uint8_t* memoryBufferPointer)
{
	pc = 0;
	nextPC = 0;
	msr = 0x00000040;
	msrUpdated();
	gpr.fill(0);
	spr.fill(0);
	fpr.fill(0);
	sr.fill(0);

	spr[SPR_PVR] = 0x00083214;
	spr[SPR_HID1] = 0x80000000;

	//Not sure what this is supposed to do, looks important.... Got it from dolphin source;
	spr[924] = 0x0d96e200;
	spr[925] = 0x1840c00d;
	spr[926] = 0x82bb08e8;

	fpscr = 0;
	spr[SPR_DEC] = 0xffffffff;
	system_exception_vector = 0;

	mmu.init(memoryBufferPointer);
	codeBuffer = mmap(NULL, CODEBUFSIZE, PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	codeSize = 0;
};

void Cpu::msrUpdated()
{
	uint32_t mask = 0b00000000000000000000000001000000;
	bool IP = (msr & mask) << 25;
	if (IP)
	{
		system_exception_vector = 0xfff00000;
	} else
	{
		system_exception_vector = 0;
	};
};

void Cpu::execute()
{
	if (JITBlocks.contains(pc))
	{
		void(*func)() = (void(*)())JITBlocks[pc].code;
		func();
		pc = nextPC;
	} else
	{
		JITCompile();
		void(*func)() = (void(*)())JITBlocks[pc].code;
                func();
                pc = nextPC;
	};
	std::cout << std::hex << (int)pc << std::endl;
};

void Cpu::JITCompile()
{

	std::vector<uint8_t> code;
	uint32_t instruction;
	uint32_t opcode;

	do
	{
		instruction = mmu.read32(nextPC);
		opcode = (instruction & 0b11111100000000000000000000000000) >> 26;
		uintptr_t nia = reinterpret_cast<uintptr_t>(&nextPC);

		std::cout << "[InstructionAt][0x" << std::hex << nextPC << "]: 0x" << std::hex << instruction << std::endl;
		std::cout << "[Opcode]: 0x" << (int)opcode << std::endl;

		switch (opcode)
		{
			case 15:
				{
				//addis;
				uintptr_t rD = reinterpret_cast<uintptr_t>(&gpr[( (instruction & 0b00000011111000000000000000000000) >> 21)]);
				uintptr_t rA = reinterpret_cast<uintptr_t>(&gpr[( (instruction & 0b00000000000111110000000000000000) >> 16)]);
				uint32_t simm = (instruction & 0b00000000000000001111111111111111) << 16;

				uint8_t instructionCode[] = {0x48, 0xbf, U64(rD), 0xc7, 0x07, U32(simm), 0x48, 0xbe, U64(rA), 0x8b, 0x06, 0x01, 0x07};
				code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				NEXT();
				break;
				}
			case 24:
				{
				//ori;
				uintptr_t rS = reinterpret_cast<uintptr_t>(&gpr[( (instruction &0b00000011111000000000000000000000) >> 21)]);
				uintptr_t rA = reinterpret_cast<uintptr_t>(&gpr[( (instruction &0b00000000000111110000000000000000) >> 16)]);
				uint32_t uimm = (instruction & 0b00000000000000001111111111111111);

				uint8_t instructionCode[] = {0x48, 0xbe, U64(rS), 0x8b, 0x06, 0x0d, U32(uimm), 0x48, 0xbf, U64(rA), 0x89, 0x07};
				code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				NEXT();
				break;
				}
			case 18:
				{
				//bl
				uint32_t li = (instruction & 0b00000011111111111111111111111100);
				bool aa = (instruction & 0b000000000000000000000000000000000010) >> 1;
				bool lk = (instruction & 0b000000000000000000000000000000000001);
				uintptr_t lr = reinterpret_cast<uintptr_t>(&spr[8]);

				if (lk)
                                {
                                        uint8_t instructionCode[] = {0x48, 0xbf, U64(nia), 0x48, 0xbe, U64(lr), 0x8b, 0x3f, 0x89, 0x3e, 0x81, 0x06, U32(0x4)};
                                        code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				};
				if (aa)
				{
					uint8_t instructionCode[] = {0x48, 0xbf, U64(nia), 0xc7, 0x07, U32(li)};
					code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));

				} else {
					uint8_t instructionCode[] = {0x48, 0xbf, U64(nia), 0xbe, U32(li), 0x01, 0x37};
                                        code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				};
				break;
				}
			default:
				std::cout << "[ERROR!]: Unimplimented instruction!" << std::endl;
				exit(0);
		};

	} while (opcode != 18);

	code.push_back(0xc3); //ret;
	insertCode(code.data(), code.size());
};

void Cpu::insertCode(uint8_t* code, size_t size)
{
	JITBlock block;
	block.code = reinterpret_cast<uint8_t*>(codeBuffer)+codeSize;
	JITBlocks.insert({pc, block});
	memcpy((reinterpret_cast<uint8_t*>(codeBuffer)+codeSize), code, size);
        codeSize += size;
};
