#pragma warning(disable : 4996)
#include "ConwayGameOfLifeExecutor.h"
#include <vector>
#include <CL/cl.hpp>
#include "IMatrix.h"
#include "Matrix.h"
#include <iostream>

ConwayGameOfLifeExecutor::ConwayGameOfLifeExecutor() {

}
ConwayGameOfLifeExecutor::~ConwayGameOfLifeExecutor() {

}

void ConwayGameOfLifeExecutor::simulate() {
	// Allocate host memory
	int columnCount = 20;
	int rowCount = 20;
	Matrix<int> initialMatrix = Matrix<int>(rowCount, columnCount);
	initialMatrix.randomInitialize();

	Matrix<int> newPopulation = Matrix<int>(rowCount, columnCount);

	for (int i = 0; i < 100; ++i)
	{
		std::cout << initialMatrix;
		next_state(initialMatrix, newPopulation, columnCount, rowCount);
		initialMatrix = newPopulation;
		std::cout << std::endl;
	}
	// alocirati memoriju host uredjaja
	// konfigurisati opencl program
	// pozvati opencl program
	// obratiti rezultat

	// Deallocate memory
}

int ConwayGameOfLifeExecutor::sum_neighbours(const Matrix<int>& matrix, int currentRow, int currentCol) {
	int total = 0;
	int left, top, right, bottom;
	left = currentCol - 1;
	right = currentCol + 1;
	top = currentRow - 1;
	bottom = currentRow + 1;

	total += (matrix[{top, left}] != 0);
	total += (matrix[{top, currentCol}] != 0);
	total += (matrix[{top, right}] != 0);
	total += (matrix[{currentRow, left}] != 0);
	total += (matrix[{currentRow, right}] != 0);
	total += (matrix[{bottom, left}] != 0);
	total += (matrix[{bottom, currentCol}] != 0);
	total += (matrix[{bottom, right}] != 0);
	return total;
}

void ConwayGameOfLifeExecutor::next_state(const Matrix<int>& in_state, Matrix<int>& out_state, int colCount, int rowCount) {
	int value, liveNeigboursCount;
	for (int i = 0; i < rowCount; ++i)
		for (int j = 0; j < colCount; ++j)
		{
			if (j == 0 || j == colCount - 1 || i == 0 || i == rowCount - 1)
				continue;
			value = in_state[{i, j}];
			liveNeigboursCount = sum_neighbours(in_state, i, j);
			out_state[{i, j}] = ((value == 1 && liveNeigboursCount == 2) || liveNeigboursCount == 3) ? 1 : 0;//  (liveNeigboursCount == 3 || liveNeigboursCount == 2 && value != 0) ? 1 : 0;
		}
}