#pragma once
#include<iostream>
#include <windows.h>

constexpr auto DEAD = 0;
constexpr auto LIVE = 1;
template<class T>
class Matrix
{
private:
	struct Index
	{
		int i;
		int j;
	};
public:
	explicit Matrix(int, int = 0);
	Matrix() = delete;
	Matrix(const Matrix&);
	Matrix(Matrix&&);
	Matrix& operator=(const Matrix&);
	Matrix& operator=(Matrix&&);
	~Matrix();

	T& operator[](const Index&) noexcept(false);
	const T& operator[](const Index&) const noexcept(false);

	void randomInitialize();
	void fromVector(int* originalVector);
	int* toVector();

	template<class A>
	friend std::ostream& operator<<(std::ostream& s, const Matrix<A>& matrix)
	{
		system("cls");
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		for (int i = 0; i < matrix.n; ++i, s << std::endl)
			for (int j = 0; j < matrix.m; ++j) {
				int value = ((int)matrix.mat[i][j] == 0) ? 7 : 254;
				SetConsoleTextAttribute(hConsole, value);
				s << "  ";
				SetConsoleTextAttribute(hConsole, 7);
			}
		return s;
	}

private:
	int n;
	int m;
	T** mat;
	void copy(const Matrix&);
	void move(Matrix&&);
	bool wrong_location(int, int) const;
};
