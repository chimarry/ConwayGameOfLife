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

	for (int i = 0; i < 100; ++i)
	{
		std::cout << initialMatrix;
		nextState(initialMatrix, newPopulation, columnCount, rowCount);
		initialMatrix = newPopulation;
		std::cout << std::endl;
	}
	// alocirati memoriju host uredjaja
	// konfigurisati opencl program
	// pozvati opencl program
	// obratiti rezultat

	// Deallocate memory
}

int ConwayGameOfLifeExecutor::sumNeighbours(const ConwayMatrix& matrix, int currentRow, int currentCol) {
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

void ConwayGameOfLifeExecutor::nextState(const ConwayMatrix& in_state, ConwayMatrix& out_state, int colCount, int rowCount) {
	int value, liveNeigboursCount;
	for (int i = 0; i < rowCount; ++i)
		for (int j = 0; j < colCount; ++j)
		{
			if (j == 0 || j == colCount - 1 || i == 0 || i == rowCount - 1)
				continue;
			value = in_state[{i, j}];
			liveNeigboursCount = sumNeighbours(in_state, i, j);
			out_state[{i, j}] = ((value == 1 && liveNeigboursCount == 2) || liveNeigboursCount == 3) ? ConwayMatrix::ALIVE : ConwayMatrix::DEAD;//  (liveNeigboursCount == 3 || liveNeigboursCount == 2 && value != 0) ? 1 : 0;
		}
}