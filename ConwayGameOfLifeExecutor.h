#pragma once
#include <iostream>
#include <string>
#include <functional>
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
	int* addSubSegment(const ConwayMatrix& initialMatrix, int* subsegment, int positionRow, int positionColumn, int rowCount, int columnCount);
	int* getSubSegment(const ConwayMatrix& initialMatrix, int positionRow, int positionColumn, int rowCount, int columnCount);
	int* manipulateSubSegment(const char* kernelName, const ConwayMatrix& initialMatrix, int* subsegment,
		int positionRow, int positionColumn, int rowCount, int columnCount, int outputSize, bool getSubSegment = false);
};
