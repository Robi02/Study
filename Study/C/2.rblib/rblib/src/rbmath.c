#include "rbmath.h"

int max(int first, int second)
{
    return (first > second ? first : second);
}

int min(int first, int second)
{
    return (first < second ? first : second);
}

int between(int over, int val, int under)
{
	return max(over, min(val, under));
}