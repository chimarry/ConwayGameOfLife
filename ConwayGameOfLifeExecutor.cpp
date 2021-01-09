#pragma warning(disable : 4996)
#include "ConwayGameOfLifeExecutor.h"
#include "ConwayMatrix.h"
#include "OpenCLConfiguration.h"

#include <vector>
#include <CL/cl.hpp>
#include <iostream>
#include <string>

ConwayGameOfLifeExecutor::ConwayGameOfLifeExecutor() {}
ConwayGameOfLifeExecutor::~ConwayGameOfLifeExecutor() {}

void ConwayGameOfLifeExecutor::simulate() {
	// Allocate host memory
	int columnCount = 40;
	int rowCount = 40;
	ConwayMatrix initialMatrix = ConwayMatrix(rowCount, columnCount);
	initialMatrix.randomInitialize();

	ConwayMatrix subSegment = ConwayMatrix(20, 20);
	subSegment.fromImage("Input.pgm");

	std::cout << subSegment << std::endl << "sub segment printed" << std::endl;
	std::cout << initialMatrix << "initial matrix printed" << std::endl;

	int* newData = addSubSegment(initialMatrix, subSegment.toIntVector(), 1, 1, 20, 20);
	initialMatrix.fromIntVector(newData);
	std::cout << initialMatrix << "after adding subsegment" << std::endl;
	int* sub = getSubSegment(initialMatrix, 1, 1, 20, 20);
	subSegment.fromIntVector(sub);
	std::cout << "GET SUBSEGMENT:" << subSegment;
	return;

	int* initialVector = initialMatrix.toIntVector();

	cl_int error;
	cl_context context;
	cl_command_queue commandQueue;
	cl_program kernelProgram;
	cl_kernel kernel;
	cl_uint platformsCount, devicesCount;
	cl_mem inputScene;
	cl_mem outputScene;

	size_t gameSceneSize = sizeof(int) * rowCount * columnCount;
	OpenCLConfiguration::configure(context, commandQueue, kernelProgram, kernel, SIMULATE_GAME_KERNEL);

	inputScene = clCreateBuffer(context, CL_MEM_READ_WRITE, gameSceneSize, NULL, &error);
	outputScene = clCreateBuffer(context, CL_MEM_READ_WRITE, gameSceneSize, NULL, &error);
	OpenCLConfiguration::printIfError(error);

	error = clEnqueueWriteBuffer(commandQueue, inputScene, CL_TRUE, 0, gameSceneSize, initialVector, 0, NULL, NULL);
	OpenCLConfiguration::printIfError(error);

	size_t localWorkSize[2], globalWorkSize[2];
	localWorkSize[0] = localWorkSize[1] = 4;
	globalWorkSize[0] = rowCount;
	globalWorkSize[1] = columnCount;

	for (int i = 0; i < 100; ++i)
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
		OpenCLConfiguration::printIfError(error);

		error = clEnqueueReadBuffer(commandQueue, outputScene, CL_TRUE, 0, gameSceneSize, initialVector, 0, NULL, NULL);
		OpenCLConfiguration::printIfError(error);

		initialMatrix.fromIntVector(initialVector);
		std::cout << initialMatrix;
	}
	clFinish(commandQueue);

	OpenCLConfiguration::releaseMemory(inputScene, outputScene, kernelProgram, kernel, commandQueue, context);
	delete[](initialVector);
}

int* ConwayGameOfLifeExecutor::addSubSegment(const ConwayMatrix& original, int* subSegmentData, int positionRow, int positionColumn, int rowCount, int columnCount)
{
	cl_int error;
	cl_context context;
	cl_command_queue commandQueue;
	cl_program kernelProgram;
	cl_kernel kernel;
	cl_mem inputScene;
	cl_mem subSegment;
	OpenCLConfiguration::configure(context, commandQueue, kernelProgram, kernel, ADD_SUB_SEGMENT);

	inputScene = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * original.getSize(), NULL, &error);
	subSegment = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * (size_t)rowCount * columnCount, NULL, &error);
	OpenCLConfiguration::printIfError(error);

	int* originalData = original.toIntVector();
	error = clEnqueueWriteBuffer(commandQueue, inputScene, CL_TRUE, 0, sizeof(int) * original.getSize(), originalData, 0, NULL, NULL);
	error = clEnqueueWriteBuffer(commandQueue, subSegment, CL_TRUE, 0, sizeof(int) * (size_t)rowCount * columnCount, subSegmentData, 0, NULL, NULL);
	OpenCLConfiguration::printIfError(error);

	size_t localWorkSize[2], globalWorkSize[2];
	localWorkSize[0] = localWorkSize[1] = 4;
	globalWorkSize[0] = original.getSize() / original.getColumnCount();
	globalWorkSize[1] = original.getColumnCount();

	int originalColumnCount = original.getColumnCount();
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputScene);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &subSegment);
	clSetKernelArg(kernel, 2, sizeof(int), (void*)&originalColumnCount);
	clSetKernelArg(kernel, 3, sizeof(int), (void*)&columnCount);
	clSetKernelArg(kernel, 4, sizeof(int), (void*)&rowCount);
	clSetKernelArg(kernel, 5, sizeof(int), (void*)&positionRow);
	clSetKernelArg(kernel, 6, sizeof(int), (void*)&positionColumn);

	error = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	OpenCLConfiguration::printIfError(error);

	int* result = new int[original.getSize()];

	error = clEnqueueReadBuffer(commandQueue, inputScene, CL_TRUE, 0, sizeof(int) * original.getSize(), result, 0, NULL, NULL);
	OpenCLConfiguration::printIfError(error);

	clFinish(commandQueue);

	OpenCLConfiguration::releaseMemory(inputScene, subSegment, kernelProgram, kernel, commandQueue, context);
	return result;
}

int* ConwayGameOfLifeExecutor::getSubSegment(const ConwayMatrix& original, int positionRow, int positionColumn, int rowCount, int columnCount)
{
	cl_int error;
	cl_context context;
	cl_command_queue commandQueue;
	cl_program kernelProgram;
	cl_kernel kernel;
	cl_mem inputScene;
	cl_mem subSegment;
	OpenCLConfiguration::configure(context, commandQueue, kernelProgram, kernel, GET_SUB_SEGMENT);

	int* subSegmentData = new int[(size_t)rowCount * columnCount];

	inputScene = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * original.getSize(), NULL, &error);
	subSegment = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * (size_t)rowCount * columnCount, NULL, &error);
	OpenCLConfiguration::printIfError(error);

	int* originalData = original.toIntVector();
	error = clEnqueueWriteBuffer(commandQueue, inputScene, CL_TRUE, 0, sizeof(int) * original.getSize(), originalData, 0, NULL, NULL);
	error = clEnqueueWriteBuffer(commandQueue, subSegment, CL_TRUE, 0, sizeof(int) * (size_t)rowCount * columnCount, subSegmentData, 0, NULL, NULL);
	OpenCLConfiguration::printIfError(error);

	size_t localWorkSize[2], globalWorkSize[2];
	localWorkSize[0] = localWorkSize[1] = 4;
	globalWorkSize[0] = original.getSize() / original.getColumnCount();
	globalWorkSize[1] = original.getColumnCount();

	int originalColumnCount = original.getColumnCount();
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputScene);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &subSegment);
	clSetKernelArg(kernel, 2, sizeof(int), (void*)&originalColumnCount);
	clSetKernelArg(kernel, 3, sizeof(int), (void*)&columnCount);
	clSetKernelArg(kernel, 4, sizeof(int), (void*)&rowCount);
	clSetKernelArg(kernel, 5, sizeof(int), (void*)&positionRow);
	clSetKernelArg(kernel, 6, sizeof(int), (void*)&positionColumn);

	error = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	OpenCLConfiguration::printIfError(error);

	int* result = new int[(size_t)rowCount * columnCount];

	error = clEnqueueReadBuffer(commandQueue, subSegment, CL_TRUE, 0, sizeof(int) * rowCount * columnCount, result, 0, NULL, NULL);
	OpenCLConfiguration::printIfError(error);

	clFinish(commandQueue);

	OpenCLConfiguration::releaseMemory(inputScene, subSegment, kernelProgram, kernel, commandQueue, context);
	return result;
}
