#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>
#include <errno.h>

#include "./Ipl.hpp"

// bootrom descrambler reversed by segher
// Copyright 2008 Segher Boessenkool <segher@kernel.crashing.org>
void Descrambler(unsigned char* data, int size)
{
	unsigned char acc = 0;
	unsigned char nacc = 0;

	unsigned short t = 0x2953;
	unsigned short u = 0xd9c2;
	unsigned short v = 0x3ff1;

	unsigned char x = 1;
	unsigned int it;
	for (it = 0; it < size; )
	{
		int t0 = t & 1;
		int t1 = (t >> 1) & 1;
		int u0 = u & 1;
		int u1 = (u >> 1) & 1;
		int v0 = v & 1;

		x ^= t1 ^ v0;
		x ^= (u0 | u1);
		x ^= (t0 ^ u1 ^ v0) & (t0 ^ u0);

		if (t0 == u0)
		{
			v >>= 1;
			if (v0)
				v ^= 0xb3d0;
		}

		if (t0 == 0)
		{
			u >>= 1;
			if (u0)
				u ^= 0xfb10;
		}

		t >>= 1;
		if (t0)
			t ^= 0xa740;

		nacc++;
		acc = 2*acc + x;
		if (nacc == 8)
		{
			data[it++] ^= acc;
			nacc = 0;
		}
	}
}


void Ipl::load(const char* path)
{
	std::ifstream file(path, std::ios::binary);

	if (!file.is_open())
	{
		std::cout << "[IPL::ERROR]: Failed to open IPL.bin at path: " << path << std::endl;
		std::cout << strerror(errno) << std::endl;
		exit(-1);
	};

        file.seekg(0, std::ios::end);
        size = file.tellg();
        std::cout << "[IPL::Size]: " << size << std::endl;

	//Read copyright string;
        file.seekg(COPYRIGHT_OFFSET, std::ios::beg);
	char copyright[COPYRIGHT_SIZE];
        file.read((char*)copyright, COPYRIGHT_SIZE);
        std::cout << "[IPL::Copyright]: "<< copyright << std::endl;

	data.resize(size);
	//Decrypt BS1 and BS2 sections;
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(data.data()), size);
	Descrambler(data.data()+0x100, size-0x100);

	file.close();
};

void Ipl::showContents()
{
	std::stringstream hex;
        for (int i = 0; i<size; i++)
        {
                hex << data[i];
        };
        std::cout << hex.str() << std::endl;
};
