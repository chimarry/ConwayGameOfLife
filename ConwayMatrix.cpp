#pragma once

#include "ConwayMatrix.h"
#include<exception>
#include<stdexcept>

ConwayMatrix::ConwayMatrix(int n, int mm) :n(n), m(mm ? mm : n), mat(new ConwayMatrix::Cell* [n])
{
	for (int i = 0; i < n; ++i) {
		mat[i] = new Cell[m];
		std::fill_n(mat[i], m, ConwayMatrix::DEAD);
	}
}

ConwayMatrix::ConwayMatrix(const ConwayMatrix& matrix)
{
	copy(matrix);
}

ConwayMatrix::ConwayMatrix(ConwayMatrix&& matrix)
{
	move(std::move(matrix));
}

ConwayMatrix& ConwayMatrix::operator=(const ConwayMatrix& matrix)
{
	if (this != &matrix) {
		this->~ConwayMatrix();
		copy(matrix);
	}
	return *this;
}

ConwayMatrix& ConwayMatrix::operator=(ConwayMatrix&& matrix)
{
	if (this != &matrix) {
		this->~ConwayMatrix();
		move(std::move(matrix));
	}
	return *this;
}

ConwayMatrix::~ConwayMatrix()
{
	for (int i = 0; i < n; ++i) {
		delete[] mat[i];
		mat[i] = nullptr;
	}
	delete[] mat;
	mat = nullptr;
	m = n = 0;
}

ConwayMatrix::Cell& ConwayMatrix::operator[](const Index& index) noexcept(false)
{
	return wrongLocation(index.i, index.j) ? throw(std::out_of_range("Wrong")) : mat[index.i][index.j];
}

const ConwayMatrix::Cell& ConwayMatrix::operator[](const Index& index) const noexcept(false)
{
	return wrongLocation(index.i, index.j) ? throw(std::out_of_range("Wrong")) : mat[index.i][index.j];
}

void ConwayMatrix::fromIntVector(int* vector) {
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < m; ++j)
			mat[i][j] = static_cast<ConwayMatrix::Cell>(vector[i + j]);
}

void ConwayMatrix::randomInitialize() {
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < m; ++j)
			mat[i][j] = (rand() % 100) > 50 ? ConwayMatrix::DEAD : ConwayMatrix::ALIVE;

}

int* ConwayMatrix::toIntVector() {
	int* vector = new int[m * n];
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < m; ++j)
			vector[i + j * m] = mat[i][j];
	return vector;
}

void ConwayMatrix::copy(const ConwayMatrix& matrix)
{
	n = matrix.n;
	m = matrix.m;
	mat = new ConwayMatrix::Cell * [n];
	for (int i = 0; i < n; i++)
	{
		mat[i] = new ConwayMatrix::Cell[m];
		std::copy(matrix.mat[i], matrix.mat[i] + m, mat[i]);
	}
}

void ConwayMatrix::move(ConwayMatrix&& matrix)
{
	n = matrix.n;
	m = matrix.m;
	mat = matrix.mat;
	matrix.mat = nullptr;
}

bool ConwayMatrix::wrongLocation(int i, int j) const
{
	return (i < 0 || i >= n || j < 0 || j >= m) ? true : false;
}
