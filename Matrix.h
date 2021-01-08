#pragma once
#include "IMatrix.h"
#include<exception>
#include<stdexcept>

constexpr auto DEFAULT_VALUE = 0;

template<class T>
Matrix<T>::Matrix(int n, int mm) :n(n), m(mm ? mm : n), mat(new T* [n])
{
	for (int i = 0; i < n; ++i) {
		mat[i] = new T[m];
		std::fill_n(mat[i], m, DEFAULT_VALUE);
	}
}

template<class T>
inline Matrix<T>::Matrix(const Matrix& matrix)
{
	copy(matrix);
}

template<class T>
inline Matrix<T>::Matrix(Matrix&& matrix)
{
	move(std::move(matrix));
}

template<class T>
inline Matrix<T>& Matrix<T>::operator=(const Matrix<T>& matrix)
{
	if (this != &matrix) {
		this->~Matrix();
		copy(matrix);
	}
	return *this;
}

template<class T>
inline Matrix<T>& Matrix<T>::operator=(Matrix<T>&& matrix)
{
	if (this != &matrix) {
		this->~Matrix();
		move(std::move(matrix));
	}
	return *this;
}

template<class T>
inline Matrix<T>::~Matrix()
{
	for (int i = 0; i < n; ++i) {
		delete[] mat[i];
		mat[i] = nullptr;
	}
	delete[] mat;
	mat = nullptr;
	m = n = 0;
}

template<class T>
inline T& Matrix<T>::operator[](const Index& index) noexcept(false)
{
	return wrong_location(index.i, index.j) ? throw(std::out_of_range("Wrong")) : mat[index.i][index.j];
}

template<class T>
inline const T& Matrix<T>::operator[](const Index& index) const noexcept(false)
{
	return wrong_location(index.i, index.j) ? throw(std::out_of_range("Wrong")) : mat[index.i][index.j];
}

template<class T>
inline void Matrix<T>::fromVector(int* vector) {
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < m; ++j)
			mat[i][j] = vector[i + j];
}

template<class T>
inline void Matrix<T>::randomInitialize() {
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < m; ++j)
			mat[i][j] = (rand() % 100) > 50 ? 0 : 1;

}

template<class T>
inline int* Matrix<T>::toVector() {
	int* vector = new int[m * n];
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < m; ++j)
			vector[i + j * m] = mat[i][j];
	return vector;
}

template<class T>
inline void Matrix<T>::copy(const Matrix& matrix)
{
	n = matrix.n;
	m = matrix.m;
	mat = new T * [n];
	for (int i = 0; i < n; i++)
	{
		mat[i] = new T[m];
		std::copy(matrix.mat[i], matrix.mat[i] + m, mat[i]);
	}
}

template<class T>
inline void Matrix<T>::move(Matrix&& matrix)
{
	n = matrix.n;
	m = matrix.m;
	mat = matrix.mat;
	matrix.mat = nullptr;
}

template<class T>
inline bool Matrix<T>::wrong_location(int i, int j) const
{
	return (i < 0 || i >= n || j < 0 || j >= m) ? true : false;
}

#pragma once
