#pragma warning(disable : 4996)
#include "ConwayGameOfLifeExecutor.h"
#include <vector>
#include <CL/cl.hpp>
#include "ConwayMatrix.h"
#include "OpenCLKernels.h"
#include <iostream>
#include <string>

ConwayGameOfLifeExecutor::ConwayGameOfLifeExecutor() {

}
ConwayGameOfLifeExecutor::~ConwayGameOfLifeExecutor() {

}

void ConwayGameOfLifeExecutor::simulate() {
	// Allocate host memory
	int columnCount = 20;
	int rowCount = 20;
	ConwayMatrix initialMatrix = ConwayMatrix(rowCount, columnCount);
	initialMatrix.randomInitialize();
	//initialMatrix.fromImage("Input.pgm");
	ConwayMatrix newPopulation = ConwayMatrix(rowCount, columnCount);
	int* initialVector = initialMatrix.toIntVector();

	/*int columnCount = 20;
		int rowCount = 20;
		ConwayMatrix initialMatrix = ConwayMatrix(rowCount, columnCount);
		initialMatrix.fromImage("Input.pgm");

		ConwayMatrix newPopulation = ConwayMatrix(rowCount, columnCount);

		for (int i = 0; i < 100; ++i)
		{
			std::cout << initialMatrix;
			nextState(initialMatrix, newPopulation, columnCount, rowCount);
			initialMatrix = newPopulation;
			std::cout << std::endl;
		}
		return;*/

#pragma region OpenCL
	cl_int error;
	cl_platform_id platformId;
	cl_device_id deviceId;
	cl_context context;
	cl_command_queue commandQueue;
	cl_program kernelProgram;
	cl_kernel kernel;
	cl_uint platformsCount, devicesCount;
	cl_mem inputScene;
	cl_mem outputScene;

	size_t gameSceneSize = sizeof(int) * rowCount * columnCount;
	error = clGetPlatformIDs(1, &platformId, &platformsCount);
	error = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_CPU, 1, &deviceId, &devicesCount);
	cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platformId,0 };

	context = clCreateContext(properties, 1, &deviceId, NULL, NULL, &error);
	commandQueue = clCreateCommandQueue(context, deviceId, 0, &error);

	char* kernelSource = readKernelSource(PROGRAM_FILENAME);
	kernelProgram = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, 0, &error);
	printIfError(error);

	error = clBuildProgram(kernelProgram, 0, NULL, NULL, NULL, 0);
	printIfError(error);

	kernel = clCreateKernel(kernelProgram, SIMULATE_GAME_KERNEL, &error);

	inputScene = clCreateBuffer(context, CL_MEM_READ_WRITE, gameSceneSize, NULL, &error);
	outputScene = clCreateBuffer(context, CL_MEM_READ_WRITE, gameSceneSize, NULL, &error);
	printIfError(error);

	error = clEnqueueWriteBuffer(commandQueue, inputScene, CL_TRUE, 0, gameSceneSize, initialVector, 0, NULL, NULL);
	printIfError(error);

	size_t localWorkSize[2], globalWorkSize[2];
	localWorkSize[0] = localWorkSize[1] = 4;
	globalWorkSize[0] = rowCount;
	globalWorkSize[1] = columnCount;

	for (int i = 0; i < 500; ++i)
	{
		if (i != 0) {
			cl_mem tmp = inputScene;
			inputScene = outputScene;
			outputScene = tmp;
		}
		clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputScene);
		clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputScene);
		clSetKernelArg(kernel, 2, sizeof(int), (void*)&rowCount);
		clSetKernelArg(kernel, 3, sizeof(int), (void*)&columnCount);

		error = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize,
			localWorkSize, 0, NULL, NULL);
		printIfError(error);

		error = clEnqueueReadBuffer(commandQueue, outputScene, CL_TRUE, 0, gameSceneSize, initialVector, 0, NULL, NULL);
		printIfError(error);

		initialMatrix.fromIntVector(initialVector);
		std::cout << initialMatrix;
		std::cout << std::endl;
	}
	clFinish(commandQueue);
	initialMatrix.writeToImage("prvaSlika.pgm");

	// release OpenCL resources
	clReleaseMemObject(inputScene);
	clReleaseMemObject(outputScene);
	clReleaseProgram(kernelProgram);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);

	//release host memory
	delete[](initialVector);
	delete[](kernelSource);
#pragma endregion
}

int ConwayGameOfLifeExecutor::sumNeighbours(const ConwayMatrix& matrix, int currentRow, int currentCol) {
	int total = 0;
	int left, top, right, bottom;
	left = currentCol - 1;
	right = currentCol + 1;
	top = currentRow - 1;
	bottom = currentRow + 1;

	total += (matrix[{top, left}] != ConwayMatrix::DEAD);
	total += (matrix[{top, currentCol}] != ConwayMatrix::DEAD);
	total += (matrix[{top, right}] != ConwayMatrix::DEAD);
	total += (matrix[{currentRow, left}] != ConwayMatrix::DEAD);
	total += (matrix[{currentRow, right}] != ConwayMatrix::DEAD);
	total += (matrix[{bottom, left}] != ConwayMatrix::DEAD);
	total += (matrix[{bottom, currentCol}] != ConwayMatrix::DEAD);
	total += (matrix[{bottom, right}] != ConwayMatrix::DEAD);
	return total;
}

void ConwayGameOfLifeExecutor::nextState(const ConwayMatrix& in_state, ConwayMatrix& out_state, int colCount, int rowCount) {
	ConwayMatrix::Cell value;
	int liveNeigboursCount;

	for (int i = 0; i < rowCount; ++i)
		for (int j = 0; j < colCount; ++j)
		{
			if (j == 0 || j == colCount - 1 || i == 0 || i == rowCount - 1)
				continue;
			value = in_state[{i, j}];
			liveNeigboursCount = sumNeighbours(in_state, i, j);
			out_state[{i, j}] = (value == ConwayMatrix::Cell::ALIVE && liveNeigboursCount == 2 || liveNeigboursCount == 3) ? ConwayMatrix::ALIVE : ConwayMatrix::DEAD;
		}
}

char* ConwayGameOfLifeExecutor::readKernelSource(const char* filename)
{
	char* kernelSource = nullptr;
	long length;
	FILE* f = fopen(filename, "r");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		kernelSource = (char*)calloc(length, sizeof(char));
		if (kernelSource)
			fread(kernelSource, 1, length, f);
		fclose(f);
	}
	return kernelSource;
}
void ConwayGameOfLifeExecutor::printIfError(cl_int error) {
	if (error != CL_SUCCESS)
		std::cerr << error;
}

std::string ConwayGameOfLifeExecutor::translateOpenCLError(cl_int errorCode)
{
	switch (errorCode)
	{
	case CL_SUCCESS:                            return "CL_SUCCESS";
	case CL_DEVICE_NOT_FOUND:                   return "CL_DEVICE_NOT_FOUND";
	case CL_DEVICE_NOT_AVAILABLE:               return "CL_DEVICE_NOT_AVAILABLE";
	case CL_COMPILER_NOT_AVAILABLE:             return "CL_COMPILER_NOT_AVAILABLE";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case CL_OUT_OF_RESOURCES:                   return "CL_OUT_OF_RESOURCES";
	case CL_OUT_OF_HOST_MEMORY:                 return "CL_OUT_OF_HOST_MEMORY";
	case CL_PROFILING_INFO_NOT_AVAILABLE:       return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case CL_MEM_COPY_OVERLAP:                   return "CL_MEM_COPY_OVERLAP";
	case CL_IMAGE_FORMAT_MISMATCH:              return "CL_IMAGE_FORMAT_MISMATCH";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case CL_BUILD_PROGRAM_FAILURE:              return "CL_BUILD_PROGRAM_FAILURE";
	case CL_MAP_FAILURE:                        return "CL_MAP_FAILURE";
	case CL_MISALIGNED_SUB_BUFFER_OFFSET:       return "CL_MISALIGNED_SUB_BUFFER_OFFSET";                          //-13
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:    return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";   //-14
	case CL_COMPILE_PROGRAM_FAILURE:            return "CL_COMPILE_PROGRAM_FAILURE";                               //-15
	case CL_LINKER_NOT_AVAILABLE:               return "CL_LINKER_NOT_AVAILABLE";                                  //-16
	case CL_LINK_PROGRAM_FAILURE:               return "CL_LINK_PROGRAM_FAILURE";                                  //-17
	case CL_DEVICE_PARTITION_FAILED:            return "CL_DEVICE_PARTITION_FAILED";                               //-18
	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:      return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";                         //-19
	case CL_INVALID_VALUE:                      return "CL_INVALID_VALUE";
	case CL_INVALID_DEVICE_TYPE:                return "CL_INVALID_DEVICE_TYPE";
	case CL_INVALID_PLATFORM:                   return "CL_INVALID_PLATFORM";
	case CL_INVALID_DEVICE:                     return "CL_INVALID_DEVICE";
	case CL_INVALID_CONTEXT:                    return "CL_INVALID_CONTEXT";
	case CL_INVALID_QUEUE_PROPERTIES:           return "CL_INVALID_QUEUE_PROPERTIES";
	case CL_INVALID_COMMAND_QUEUE:              return "CL_INVALID_COMMAND_QUEUE";
	case CL_INVALID_HOST_PTR:                   return "CL_INVALID_HOST_PTR";
	case CL_INVALID_MEM_OBJECT:                 return "CL_INVALID_MEM_OBJECT";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case CL_INVALID_IMAGE_SIZE:                 return "CL_INVALID_IMAGE_SIZE";
	case CL_INVALID_SAMPLER:                    return "CL_INVALID_SAMPLER";
	case CL_INVALID_BINARY:                     return "CL_INVALID_BINARY";
	case CL_INVALID_BUILD_OPTIONS:              return "CL_INVALID_BUILD_OPTIONS";
	case CL_INVALID_PROGRAM:                    return "CL_INVALID_PROGRAM";
	case CL_INVALID_PROGRAM_EXECUTABLE:         return "CL_INVALID_PROGRAM_EXECUTABLE";
	case CL_INVALID_KERNEL_NAME:                return "CL_INVALID_KERNEL_NAME";
	case CL_INVALID_KERNEL_DEFINITION:          return "CL_INVALID_KERNEL_DEFINITION";
	case CL_INVALID_KERNEL:                     return "CL_INVALID_KERNEL";
	case CL_INVALID_ARG_INDEX:                  return "CL_INVALID_ARG_INDEX";
	case CL_INVALID_ARG_VALUE:                  return "CL_INVALID_ARG_VALUE";
	case CL_INVALID_ARG_SIZE:                   return "CL_INVALID_ARG_SIZE";
	case CL_INVALID_KERNEL_ARGS:                return "CL_INVALID_KERNEL_ARGS";
	case CL_INVALID_WORK_DIMENSION:             return "CL_INVALID_WORK_DIMENSION";
	case CL_INVALID_WORK_GROUP_SIZE:            return "CL_INVALID_WORK_GROUP_SIZE";
	case CL_INVALID_WORK_ITEM_SIZE:             return "CL_INVALID_WORK_ITEM_SIZE";
	case CL_INVALID_GLOBAL_OFFSET:              return "CL_INVALID_GLOBAL_OFFSET";
	case CL_INVALID_EVENT_WAIT_LIST:            return "CL_INVALID_EVENT_WAIT_LIST";
	case CL_INVALID_EVENT:                      return "CL_INVALID_EVENT";
	case CL_INVALID_OPERATION:                  return "CL_INVALID_OPERATION";
	case CL_INVALID_GL_OBJECT:                  return "CL_INVALID_GL_OBJECT";
	case CL_INVALID_BUFFER_SIZE:                return "CL_INVALID_BUFFER_SIZE";
	case CL_INVALID_MIP_LEVEL:                  return "CL_INVALID_MIP_LEVEL";
	case CL_INVALID_GLOBAL_WORK_SIZE:           return "CL_INVALID_GLOBAL_WORK_SIZE";                           //-63
	case CL_INVALID_PROPERTY:                   return "CL_INVALID_PROPERTY";                                   //-64
	case CL_INVALID_IMAGE_DESCRIPTOR:           return "CL_INVALID_IMAGE_DESCRIPTOR";                           //-65
	case CL_INVALID_COMPILER_OPTIONS:           return "CL_INVALID_COMPILER_OPTIONS";                           //-66
	case CL_INVALID_LINKER_OPTIONS:             return "CL_INVALID_LINKER_OPTIONS";                             //-67
	case CL_INVALID_DEVICE_PARTITION_COUNT:     return "CL_INVALID_DEVICE_PARTITION_COUNT";                     //-68
	default:
		return "UNKNOWN ERROR CODE";
	}
}