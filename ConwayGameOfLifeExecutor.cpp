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