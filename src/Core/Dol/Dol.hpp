#ifndef DOL_HPP
#define DOL_HPP

#include <cstdint>
#include <array>
#include <vector>

#define TEXT_OFFSET_START 0x0
#define TEXT_OFFSET_END 0x1b
#define TEXT_OFFSET_SIZE 28

#define TEXT_LOADING_ADDRESS_START 0x48
#define TEXT_LOADING_ADDRESS_END 0x64
#define TEXT_LOADING_ADDRESS_SIZE 28

#define TEXT_SECTION_SIZE_START 0x90
#define TEXT_SECTION_SIZE_END 0xab
#define TEXT_SECTION_SIZE_SIZE 28

#define DATA_OFFSET_START 0x1c
#define DATA_OFFSET_END 0x47
#define DATA_OFFSET_SIZE 44

#define DATA_LOADING_ADDRESS_START 0x65
#define DATA_LOADING_ADDRESS_END 0x8f
#define DATA_LOADING_ADDRESS_SIZE 44

#define DATA_SECTION_SIZE_START 0xac
#define DATA_SECTION_SIZE_END 0xd7
#define DATA_SECTION_SIZE_SIZE 44

#define BSS_ADDRESS_START 0xd8
#define BSS_ADDRESS_END 0xdb
#define BSS_ADDRESS_SIZE 4

#define BSS_SIZE_START 0xdc
#define BSS_SIZE_END 0xdf
#define BSS_SIZE_SIZE 4

#define ENTRYPOINT_START 0xe0
#define ENTRYPOINT_END 0xe3
#define ENTRYPOINT_SIZE 4

struct textSection
{
	std::vector<uint8_t> data;
};

struct dataSection
{
	std::vector<uint8_t> data;
};

class Dol
{
	public:
		std::array<uint32_t, 7> textFileOffsets;
		std::array<uint32_t, 7> textSectionSizes;
                std::array<uint32_t, 7> textLoadingAddresses;
		std::vector<textSection> textSections;

		std::array<uint32_t, 11> dataFileOffsets;
		std::array<uint32_t, 11> dataSectionSizes;
		std::array<uint32_t, 11> dataLoadingAddresses;
		std::vector<dataSection> dataSections;

		uint32_t bssAddress;
		uint32_t bssSize;

		uint32_t entryPoint;

		void load(const char* path);
};

#endif
