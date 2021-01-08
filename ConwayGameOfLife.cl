__kernel void simulateGameOfLife(__global int* originalScene, __global int* newScene, int rowCount, int colCount)
{
	int left, right, bottom, top, col, row;
    int dead = 0;
	int total = 0;

	row = get_global_id(0);
	col = get_global_id(1);
	if(row >= rowCount || col >= colCount)
		return;
   if (row == 0 || col == colCount - 1 || col == 0 || row == rowCount - 1)
	    return;
	left = col - 1;
	right = col + 1;
	top = (row + 1);
	bottom = (row - 1);
	row *= colCount;
	top *= colCount;
	bottom *= colCount;

	total += (originalScene[top + col] != dead);
	total += (originalScene[top + right] != dead);
	total += (originalScene[top + left] != dead);

	total += (originalScene[row + left] != dead);
	total += (originalScene[row + right] != dead);

	total += (originalScene[bottom + left] != dead);
	total += (originalScene[bottom + col] != dead);
	total += (originalScene[bottom + right] != dead);

	int value = originalScene[row + col];
	value = (value == 1 && total == 2 || total == 3) ? 1 : 0;
	newScene[row + col] = value;
}