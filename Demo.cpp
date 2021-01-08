#pragma warning(disable : 4996)
#include <iostream>
#include <CL\cl.hpp>
#include <vector>
#include "ConwayGameOfLifeExecutor.h"

int main() {
	ConwayGameOfLifeExecutor executor;
	executor.simulate();
}