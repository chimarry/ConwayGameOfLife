#pragma warning(disable : 4996)
#include "ConwayGameOfLifeExecutor.h"
#include <vector>
#include <CL/cl.hpp>
#include "ConwayMatrix.h"
#include <iostream>

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

	ConwayMatrix newPopulation = ConwayMatrix(rowCount, columnCount);
	int* initialVector = initialMatrix.toIntVector();

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
	char deviceName[256];

	size_t gameSceneSize = sizeof(int) * rowCount * columnCount;
	error = clGetPlatformIDs(1, &platformId, &platformsCount);
	error = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_CPU, 1, &deviceId, &devicesCount);
	error = clGetDeviceInfo(deviceId, CL_DEVICE_NAME, 256, deviceName, NULL);
	cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platformId,0 };

	context = clCreateContext(properties, 1, &deviceId, NULL, NULL, &error);
	commandQueue = clCreateCommandQueue(context, deviceId, 0, &error);
	std::cout << "Created CL command queue for device " << deviceName << std::endl;

	char* kernelSource = readKernelSource("ConwayGameOfLife.cl");
	kernelProgram = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, 0, &error);
	std::cout << "Loaded kernel source with status" << error << std::endl;

	error = clBuildProgram(kernelProgram, 0, NULL, NULL, NULL, 0);
	if (error != CL_SUCCESS)
		std::cout << "Error " << error << " while building kernel" << std::endl;

	kernel = clCreateKernel(kernelProgram, "simulateGameOfLife", &error);

	inputScene = clCreateBuffer(context, CL_MEM_READ_WRITE, gameSceneSize, NULL, &error);
	outputScene = clCreateBuffer(context, CL_MEM_READ_WRITE, gameSceneSize, NULL, &error);

	if (error != CL_SUCCESS)
		std::cout << "Error " << error << " while allocating buffers" << std::endl;

	error = clEnqueueWriteBuffer(commandQueue, inputScene, CL_TRUE, 0, gameSceneSize, initialVector, 0, NULL, NULL);
	if (error != CL_SUCCESS)
		std::cout << "Error " << error << " while writing to GPU" << std::endl;

	size_t localWorkSize[2], globalWorkSize[2];
	localWorkSize[0] = localWorkSize[1] = 10;
	globalWorkSize[0] = rowCount;
	globalWorkSize[1] = columnCount;

	for (int i = 0; i < 100; ++i)
	{
		if (i != 0)
			inputScene = outputScene;
		clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputScene);
		clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputScene);
		clSetKernelArg(kernel, 2, sizeof(int), (void*)&rowCount);
		clSetKernelArg(kernel, 3, sizeof(int), (void*)&columnCount);
		error = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize,
			localWorkSize, 0, NULL, NULL);
		if (error != CL_SUCCESS)
			std::cout << "Error " << error << " while running kernel" << std::endl;

		clFinish(commandQueue);

		error = clEnqueueReadBuffer(commandQueue, outputScene, CL_TRUE, 0, gameSceneSize, initialVector, 0, NULL, NULL);
		if (error != CL_SUCCESS)
			std::cout << "Error " << error << " while reading result" << std::endl;
		clFinish(commandQueue);

		initialMatrix.fromIntVector(initialVector);
		std::cout << initialMatrix;
		std::cout << std::endl;
	}
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