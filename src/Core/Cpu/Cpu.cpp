#include <sys/mman.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string.h>
#include <bit>

#include "./Cpu.hpp"
#include "../Memory/Mmu.hpp"

Cpu* cpuInstance = nullptr;

void Cpu::init(uint8_t* memoryBufferPointer)
{
	pc = 0;
	nextPC = 0;
	msr = 0x00000040;
	msrUpdated();
	cr = 0;
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

	cpuInstance = this;
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
};

void Cpu::JITCompile()
{

	std::vector<uint8_t> code;
	cur = pc;
	uint32_t instruction;
	uint32_t opcode;

	do
	{
		instruction = mmu.read32(cur);
		opcode = (instruction & 0b11111100000000000000000000000000) >> 26;
		uintptr_t cia = reinterpret_cast<uintptr_t>(&pc);
		uintptr_t nia = reinterpret_cast<uintptr_t>(&nextPC);

		std::cout << "[InstructionAt][0x" << std::hex << cur << "]: 0x" << std::hex << instruction << std::endl;
		std::cout << "[Opcode]: 0x" << (int)opcode << std::endl;

		switch (opcode)
		{
			case 11:
				{
				//cmpi
				void (*cmpins)(uint32_t);
				cmpins = cmpi;
				uintptr_t cmpiAddr = reinterpret_cast<uintptr_t>(cmpins);

				uint8_t instructionCode[] = {0xbf, U32(instruction), 0x48, 0xba, U64(cmpiAddr), 0xff, 0xd2};
                                code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				NEXT();
				break;
				}
			case 14:
				{
				//addi;
				uintptr_t rD = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 21) & 0x0000001f) ]);
                                uintptr_t rA = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 16) & 0x0000001f) ]);
				int32_t simm = (uint16_t)(instruction & 0x0000ffff);
				if (( (instruction >> 16) & 0x0000001f)==0)
				{
					uint8_t instructionCode[] = {0x48, 0xbf, U64(rD), 0xc7, 0x07, U32(simm)};
                                        code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				} else {
					uint8_t instructionCode[] = {0x48, 0xbf, U64(rD), 0x48, 0xbe, U64(rA), 0xc7, 0x07, U32(simm), 0x8b, 0x36, 0x01, 0x37};
                                	code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				};
				NEXT();
				break;
				}
			case 15:
				{
				//addis;
				uintptr_t rD = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 21) & 0x0000001f) ]);
				uintptr_t rA = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 16) & 0x0000001f) ]);
				int32_t simm = (instruction & 0x0000ffff) << 16;
				if (( (instruction >> 16) & 0x0000001f)==0)
				{
					uint8_t instructionCode[] {0x48, 0xbf, U64(rD), 0xc7, 0x07, U32(simm)};
					code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				} else {
					uint8_t instructionCode[] = {0x48, 0xbf, U64(rD), 0xc7, 0x07, U32(simm), 0x48, 0xbe, U64(rA), 0x8b, 0x06, 0x01, 0x07};
					code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				};
				NEXT();
				break;
				}
			case 16:
				{
				//bcx;
				void (*branch)(uint32_t);
				branch = bcx;
				uintptr_t branchAddr = reinterpret_cast<uintptr_t>(branch);

				uint8_t instructionCode[] = {0x48, 0xba, U64(branchAddr), 0xbf, U32(instruction), 0xff, 0xd2};
				code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				break;
				}
			case 21:
				{
				//rlwinm;
				void (*rlwinmins)(uint32_t);
				rlwinmins = rlwinm;
				uintptr_t rlwinmAddr = reinterpret_cast<uintptr_t>(rlwinmins);
				uint8_t instructionCode[] = {0x48, 0xba, U64(rlwinmAddr), 0xbf, U32(instruction), 0xff, 0xd2};
				code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				NEXT();
				break;
				}
			case 24:
				{
				//ori;
				uintptr_t rS = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 21) & 0x0000001f) ]);
				uintptr_t rA = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 16) & 0x0000001f) ]);
				uint32_t uimm = (instruction & 0x0000ffff);
				uint8_t instructionCode[] = {0x48, 0xbe, U64(rS), 0x8b, 0x06, 0x0d, U32(uimm), 0x48, 0xbf, U64(rA), 0x89, 0x07};
				code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				NEXT();
				break;
				}
			case 18:
				{
				//bl;
				uint32_t li = ((instruction >> 2) & 0x00ffffff) << 2;
				int32_t offset = ((int32_t)(li << 6)) >> 6;
				bool aa = (instruction >> 1) & 0x00000001;
				bool lk = (instruction) & 0x00000001;
				uintptr_t lr = reinterpret_cast<uintptr_t>(&spr[8]);
				if (aa)
				{
					uint8_t instructionCode[] = {0x48, 0xbf, U64(nia), 0xc7, 0x07, U32(offset)};
					code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				} else {

					uint8_t instructionCode[] = {0x48, 0xbf, U64(nia), 0x81, 0x07, U32(offset)};
					code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				};
				if (lk)
				{
					uint8_t _next[] = {0x48, 0xbe, U64(lr), 0xb8, U32(cur+0x4), 0x89, 0x06};
					code.insert(code.end(), std::begin(_next), std::end(_next));
				};
				break;
				}
			case 19:
				{
				//tbd from xo field;
				uint32_t xo = (instruction >> 1) & 0x000003ff;
				switch (xo)
				{
					case 16:
						{
						void (*branch)(uint32_t);
						branch = bclrx;
						uintptr_t branchAddr = reinterpret_cast<uintptr_t>(branch);

						uint8_t instructionCode[] = {0xbf, U32(instruction), 0x48, 0xba, U64(branchAddr), 0xff, 0xd2};
		                                code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
						break;
						}
					default:
						{
						std::cout << "[ERROR!]: Unimplemented extension!" << std::endl;
						break;
						}
				};
				break;
				};
			case 31:
				{
				//tbd from xo field.
				uint32_t xo = (instruction >> 1) & 0x000003ff;
				switch (xo)
				{
					case 339:
						{
						//mfspr;
						uintptr_t rD = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 21) & 0x0000001f) ]);
						uint32_t n = ((instruction >> 11) & 0x0000001f) << 5 | ((instruction >> 16) & 0x0000001f);
						uintptr_t sprN = reinterpret_cast<uintptr_t>(&spr[n]);

						uint8_t instructionCode[] = {0x48, 0xbf, U64(sprN), 0x48, 0xbe, U64(rD), 0x8b, 0x17, 0x89, 0x16};
						code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
						NEXT();
						break;
						}
					case 467:
						{
						//mtspr;
						uintptr_t rS = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 21) & 0x0000001f) ]);
						uint32_t n = ((instruction >> 11) & 0x0000001f) << 5 | ((instruction >> 16) & 0x0000001f);
						uintptr_t sprN = reinterpret_cast<uintptr_t>(&spr[n]);

						uint8_t instructionCode[] = {0x48, 0xbf, U64(sprN), 0x48, 0xbe, U64(rS), 0x8b, 0x36, 0x89, 0x37};
						code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
						NEXT();
						break;
						}
					case 266:
						{
						//add;
						uintptr_t rD = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 21) & 0x0000001f)]);
						uintptr_t rA = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 16) & 0x0000001f)]);
						uintptr_t rB = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 11) & 0x0000001f)]);
						bool oe = (instruction >> 10) & 0x00000001;
						bool rc = (instruction) & 0x00000001;

						uint8_t instructionCode[] = {0x48, 0xbf, U64(rA), 0x48, 0xbe, U64(rB), 0x48, 0xba, U64(rD), 0x8b, 0x3f, 0x8b, 0x36, 0x89, 0x3a, 0x01, 0x32};
						code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
						if (rc)
						{
							void (*update)(uint32_t, uint32_t);
							update = updateCR;
							uintptr_t updateAddr = reinterpret_cast<uintptr_t>(update);
							uint8_t updateCode[] = {0xbf, U32(0x0), 0x8b, 0x3a, 0x48, 0xba, U64(updateAddr), 0xff, 0xd2};
							code.insert(code.end(), std::begin(updateCode), std::end(updateCode));
						};
						NEXT();
						break;
						}
					default:
						std::cout << "[ERROR!]: Unimplimented extension!" << std::endl;
						exit(0);
				};
				break;
				}
			case 32:
				{
				//lwz;
				uintptr_t rD = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 21) & 0x0000001f) ]);
				uintptr_t rA = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 16) & 0x0000001f) ]);
				int32_t d = (int16_t)(instruction & 0x0000ffff);
				uint32_t (*read)(uint32_t);
				read = mmu.read32;
				uintptr_t readAddr = reinterpret_cast<uintptr_t>(read);

				uint8_t instructionCode[] = {0x48, 0xbf, U64(rA), 0x48, 0xba, U64(readAddr), 0x8b, 0x3f, 0x81, 0xc7, U32(d), 0xff, 0xd2, 0x48, 0xbe, U64(rD), 0x89, 0x06};
                                code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				NEXT();
				break;
				}
			case 36:
				{
				//stwu;
				uintptr_t rS = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 21) & 0x0000001f) ]);
				uintptr_t rA = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 16) & 0x0000001f) ]);
				int32_t d = (int16_t)(instruction & 0x0000ffff);
				void (*write)(uint32_t, uint32_t);
				write = mmu.write32;
				uintptr_t writeAddr = reinterpret_cast<uintptr_t>(write);

				uint8_t instructionCode[] = {0x48, 0xbf, U64(rA), 0x48, 0xbe, U64(rS), 0x48, 0xba, U64(writeAddr), 0x8b, 0x3f, 0x8b, 0x36, 0x81, 0xc7, U32(d), 0xff, 0xd2};
				code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				NEXT();
				break;
				}
			case 37:
                                {
                                //stwu;
                                uintptr_t rS = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 21) & 0x0000001f) ]);
                                uintptr_t rA = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 16) & 0x0000001f) ]);
                                int32_t d = (int16_t)(instruction & 0x0000ffff);
                                void (*write)(uint32_t, uint32_t);
                                write = mmu.write32;
                                uintptr_t writeAddr = reinterpret_cast<uintptr_t>(write);

                                uint8_t instructionCode[] = {0x48, 0xbf, U64(rA), 0x48, 0xbe, U64(rS), 0x48, 0xba, U64(writeAddr), 0x81, 0x07, U32(d), 0x8b, 0x3f, 0x8b, 0x36, 0xff, 0xd2};
                                code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
                                NEXT();
                                break;
				}
			case 44:
				{
				//sth;
				uintptr_t rS = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 21) & 0x0000001f) ]);
                                uintptr_t rA = reinterpret_cast<uintptr_t>(&gpr[( (instruction >> 16) & 0x0000001f) ]);
                                int32_t d = (int16_t)(instruction & 0x0000ffff);
				void (*write)(uint32_t, uint16_t);
                                write = mmu.write16;
                                uintptr_t writeAddr = reinterpret_cast<uintptr_t>(write);

				uint8_t instructionCode[] = {0x48, 0xbf, U64(rA), 0x48, 0xbe, U64(rS), 0x48, 0xba, U64(writeAddr), 0x8b, 0x3f, 0x66, 0x8b, 0x36, 0x81, 0xc7, U32(d), 0xff, 0xd2};
				code.insert(code.end(), std::begin(instructionCode), std::end(instructionCode));
				NEXT();
				break;
				}
			default:
				std::cout << "[ERROR!]: Unimplimented instruction!" << std::endl;
				exit(0);
		};

	} while ((opcode != 18) && (opcode != 16) && (opcode != 19));

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

void Cpu::bcx(uint32_t instruction)
{
	std::cout << "bcx: 0x" << std::hex << instruction << std::endl;
	uint32_t bo = (instruction >> 21) & 0x0000001f;
	uint32_t bi = (instruction >> 16) & 0x0000001f;
	int32_t bd = (instruction >> 2) & 0x00003fff;
	bool aa = (instruction >> 1) & 0x00000001;
	bool lk = (instruction) & 0x00000001;
	bool cr_bit = (cpuInstance->cr >> (31 - bi)) & 0x1;

	if (!((bo>>2)&0x1))
	{
		cpuInstance->spr[9] -= 1;
	};
	bool ctr_ok = ((bo>>2)&0x1) || ((cpuInstance->spr[9]!=0) ^ ((bo>>1)&0x1));
	bool cond_ok = ((bo>>4)&0x1) || (cr_bit == ((bo>>3)&0x1));
	if (ctr_ok && cond_ok)
	{
		if (aa)
		{
			int32_t exts = (int32_t)(bd << 18) >> 18;
			exts = exts << 2;
			cpuInstance->nextPC = exts;
		} else {
			int32_t exts = (int32_t)(bd << 18) >> 18;
                        exts = exts << 2;
                        cpuInstance->nextPC = cpuInstance->cur + exts;
		};
		if (lk)
		{
			cpuInstance->spr[8] = cpuInstance->cur + 0x4;
		};
	} else {
		cpuInstance->nextPC = cpuInstance->cur + 0x4;
	};
};

void Cpu::bclrx(uint32_t instruction)
{
	std::cout << "bclrx: 0x" << std::hex << instruction << std::endl;
	uint32_t bo = (instruction >> 21) & 0x0000001f;
	uint32_t bi = (instruction >> 16) & 0x0000001f;
	bool lk = (instruction) & 0x00000001;
	bool cr_bit = (cpuInstance->cr >> (31 - bi)) & 0x1;
	if (!((bo>>2)&0x1))
	{
                cpuInstance->spr[9] -= 1;
	};
	bool ctr_ok = ((bo>>2)&0x1) || ((cpuInstance->spr[9]!=0) ^ ((bo>>1)&0x1));
        bool cond_ok = ((bo>>4)&0x1) || (cr_bit == ((bo>>3)&0x1));
	if (ctr_ok && cond_ok)
	{
		cpuInstance->nextPC = (cpuInstance->spr[8] & 0xfffffffc);
		if (lk)
		{
			cpuInstance->spr[8] = cpuInstance->cur + 0x4;
		};
	} else {
		cpuInstance->nextPC = cpuInstance->cur + 0x4;
	};
};

void Cpu::cmpi(uint32_t instruction)
{
	std::cout << "cmpi" << std::endl;
	uint32_t crfD = (instruction >> 23) & 0x00000007;
	bool l = (instruction >> 21) & 0x00000001;
	uint32_t A = (instruction >> 16) & 0x0000001f;
	int32_t simm = (int16_t)(instruction & 0x0000ffff);
	uint8_t c = 0x0;
	if (cpuInstance->gpr[A] < simm)
	{
		c = 0b100;
	} else if (cpuInstance->gpr[A] > simm)
	{
		c = 0b010;
	} else {
		c = 0b001;
	};
	bool so_bit = (cpuInstance->spr[0] >> 30) & 0x00000001;
	uint8_t cr_field = (c << 1) | so_bit;
	uint32_t shift = 28 - 4 * crfD;
	cpuInstance->cr &= ~(0xF << shift);
	cpuInstance->cr |= (cr_field << shift);
};

void Cpu::rlwinm(uint32_t instruction)
{
	std::cout << "rlwinm" << std::endl;
	uint32_t S = (instruction >> 21) & 0x0000001f;
	uint32_t A = (instruction >> 16) & 0x0000001f;
	uint32_t sh = (instruction >> 11) & 0x0000001f;
	uint32_t mb = (instruction >> 6) & 0x0000001f;
	uint32_t me = (instruction >> 1) & 0x0000001f;
	bool rc = (instruction) & 0x00000001;

	uint32_t result = std::rotl(cpuInstance->gpr[S], sh);
	uint32_t mask = 0;
	if (mb <= me) {
    		mask = ((1u << (me - mb + 1)) - 1) << (31 - me);
	} else {
    		mask = ~(((1u << (mb - me - 1)) - 1) << (me + 1));
	};
	result &= mask;
	cpuInstance->gpr[A] = result;
	if (rc)
	{
		cpuInstance->updateCR(0, result);
	};
};

void Cpu::updateCR(uint32_t crfD, uint32_t result)
{
	std::cout << "updateCR: crfd: 0x" << std::hex << crfD << " result: " << std::hex << result << std::endl;
	bool so = (cpuInstance->spr[0] >> 30) & 1;
        uint8_t c = 0;

        if ((int32_t)result < 0)
        {
                c |= 0b100;
        };
        if ((int32_t)result > 0)
        {
                c |= 0b010;
        };
        if ((int32_t)result == 0)
        {
                c |= 0b001;
        };
        uint32_t cr_field = (c << 1) & so;
	uint32_t shift = 28 - 4 * crfD;
        cpuInstance->cr &= ~((uint32_t)(0xf) << shift);
        cpuInstance->cr |= ((uint32_t)(cr_field) << shift);
};
