#pragma once
#include <iostream>
#include "Matrix.h"

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
	int sum_neighbours(const Matrix<int>& matrix, int currentRow, int currentCol);
	void next_state(const Matrix<int>& in_state, Matrix<int>& out_state,int colCount,int rowCount);
};


