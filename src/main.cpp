#include <iostream>
#include <fstream>

#include "./Core/Core.hpp"

int main()
{
	Core core;
	core.init();
	core.loadDol("./panda.dol");
	return 0;
};
