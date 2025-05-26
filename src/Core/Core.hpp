#ifndef CORE_HPP
#define CORE_HPP

#include <cstdint>

#include "./Cpu/Cpu.hpp"
#include "./Ipl/Ipl.hpp"
#include "./Dol/Dol.hpp"

#define GAMECUBE_RAM_SIZE 25165824

class Core
{
	private:
		Cpu cpu;
		Ipl ipl;
	public:
		uint8_t* memoryBuffer;

		void init();
		void loadDol(char* path);
};

#endif
