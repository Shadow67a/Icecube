cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)

project(icecube LANGUAGES CXX C VERSION 0.1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_CXX_FLAGS "-g")

add_executable(icecube
	src/main.cpp
	src/Core/Core.cpp
	src/Core/Cpu/Cpu.cpp
	src/Core/Memory/Mmu.cpp
	src/Core/Ipl/Ipl.cpp
	src/Core/Dol/Dol.cpp
	src/Core/Endian.cpp
)
