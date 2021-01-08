#pragma once
#include <iostream>
#include <string>
#include <CL/cl.hpp>
#include "ConwayMatrix.h"

class ConwayGameOfLifeExecutor
{
public:
	ConwayGameOfLifeExecutor();
	ConwayGameOfLifeExecutor(const ConwayGameOfLifeExecutor&) = delete;
	ConwayGameOfLifeExecutor(ConwayGameOfLifeExecutor&&) = delete;
	ConwayGameOfLifeExecutor& operator=(const ConwayGameOfLifeExecutor&) = delete;
	ConwayGameOfLifeExecutor& operator=(ConwayGameOfLifeExecutor&&) = delete;
	~ConwayGameOfLifeExecutor();

	void simulate();
private:
	int sumNeighbours(const ConwayMatrix& matrix, int currentRow, int currentCol);
	void nextState(const ConwayMatrix& in_state, ConwayMatrix& out_state, int colCount, int rowCount);
	char* readKernelSource(const char* filename);
	void printIfError(cl_int errorCode);
	std::string translateOpenCLError(cl_int errorCode);
};
