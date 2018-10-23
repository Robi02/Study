#include "rbmath.h"

int max(int first, int last)
{
	return first > last ? first : last;
}

int min(int first, int last)
{
	return first < last ? first : last;
}

int between(int over, int val, int under)
{
	return max(over, min(val, under));
}