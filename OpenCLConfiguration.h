#pragma once
#include "CL/cl.hpp"
#include <string>

constexpr auto PROGRAM_FILENAME = "ConwayGameOfLife.cl";
constexpr auto SIMULATE_GAME_KERNEL = "simulateGameOfLife";
constexpr auto ADD_SUB_SEGMENT = "addSubSegment";
constexpr auto GET_SUB_SEGMENT = "getSubSegment";

class OpenCLConfiguration
{
public:
	static void configure(
		cl_context& context,
		cl_command_queue& commandQueue,
		cl_program& kernelProgram,
		cl_kernel& kernel,
		const char* kernelName);
	static void releaseMemory(
		cl_mem& obj1,
		cl_mem& obj2,
		cl_program& program,
		cl_kernel& kernel,
		cl_command_queue& commandQueue,
		cl_context& context);
	static char* readKernelSource(const char* filename);
	static void printIfError(cl_int errorCode);
	static std::string translateOpenCLError(cl_int errorCode);
};

