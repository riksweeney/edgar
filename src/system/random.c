#include "../headers.h"

static long rSeed;

void setSeed(long seed)
{
    rSeed = seed;
}

long prand()
{
    long k1;
    long ix = rSeed;

    k1 = ix / 127773;
    ix = 16807 * (ix - k1 * 127773) - k1 * 2836;

    if (ix < 0)
    {
        ix += 2147483647;
	}

    rSeed = ix;

    return rSeed;
}
