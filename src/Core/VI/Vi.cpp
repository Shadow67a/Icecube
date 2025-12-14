#include <cstdint>
#include <iostream>

#include "./Vi.hpp"

uint32_t Vi::read32(uint32_t address)
{
	uint32_t result;
	switch (address)
	{
		case 0xcc00201c:
				{
				result = tfbl;
				break;
				}
		default:
			{
			std::cout << "[VI::read32]: Out of range or unimlplemented register!" << std::endl;
			exit(0);
			}
	};
	return result;
};

void Vi::write16(uint32_t address, uint16_t value)
{
	switch (address)
	{
		default:
			{
			//Not implemented any 16bit registers yet...
			std::cout << "[VI::write16]: Out of range or unimlplemented register!" << std::endl;
			break;
			}
	};
};

void Vi::write32(uint32_t address, uint32_t value)
{
        switch (address)
        {
		case 0xcc00201c:
				{
				tfbl = value;
				break;
				};
                default:
                        {
                        std::cout << "[VI::write32]: Out of range or unimlplemented register!" << std::endl;
			break;
                        }
        };
};
