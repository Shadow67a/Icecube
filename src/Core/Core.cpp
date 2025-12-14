#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <cstdint>

#include "./Core.hpp"
#include "./Dol/Dol.hpp"
#include "./Endian.hpp"

void Core::init()
{
	//Allocate 24 mb of game cube ram;
	memoryBuffer = new uint8_t[GAMECUBE_RAM_SIZE];
	cpu.init(memoryBuffer);
	cpu.mmu.addDevice(&vi);
};

void Core::loadDol(char* path)
{
	Dol dolfile;
	dolfile.load(path);

	for (size_t i = 0; i<dolfile.textSections.size(); i++)
	{
		cpu.mmu.copy(dolfile.textLoadingAddresses[i], dolfile.textSections[i].data.data(), dolfile.textSections[i].data.size());
		std::cout << "[CORE]: .text" << i << " loaded into memoryBuffer;" << std::endl;
	};

	cpu.mmu.fill(dolfile.bssAddress, 0, dolfile.bssSize);
	cpu.pc = dolfile.entryPoint;
//	cpu.nextPC = dolfile.entryPoint;
	while (1)
	{
		cpu.execute();
	};
};
