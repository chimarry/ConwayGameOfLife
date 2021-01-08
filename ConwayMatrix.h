#pragma once
#include <iostream>
#include <windows.h>

class ConwayMatrix
{
private:
	struct Index
	{
		int i;
		int j;
	};
public:
	enum  Cell { DEAD, ALIVE };
	explicit ConwayMatrix(int, int = 0);
	ConwayMatrix() = delete;
	ConwayMatrix(const ConwayMatrix&);
	ConwayMatrix(ConwayMatrix&&) noexcept;
	ConwayMatrix& operator=(const ConwayMatrix&);
	ConwayMatrix& operator=(ConwayMatrix&&) noexcept;
	~ConwayMatrix();

	Cell& operator[](const Index&) noexcept(false);
	const Cell& operator[](const Index&) const noexcept(false);

	void randomInitialize();
	void fromIntVector(int* originalVector);
	int* toIntVector();


	friend std::ostream& operator<<(std::ostream& s, const ConwayMatrix& matrix)
	{
		system("cls");
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		for (int i = 0; i < matrix.n; ++i, s << std::endl)
			for (int j = 0; j < matrix.m; ++j) {
				int value = (matrix.mat[i][j] == ConwayMatrix::Cell::DEAD) ? 7 : 254;
				SetConsoleTextAttribute(hConsole, value);
				s << "  ";
				SetConsoleTextAttribute(hConsole, 7);
			}
		return s;
	}

private:
	int n;
	int m;
	Cell** mat;
	void copy(const ConwayMatrix&);
	void move(ConwayMatrix&&);
	bool wrongLocation(int, int) const;
};

