#pragma once

//========================================================================
// Math_Random.cpp -  a useful random number generator class
//========================================================================

#include "Common/CommonStd.h"
#include <time.h>
#include "Math.h"

// --------------------------------------------------------------------------

NvRandom::NvRandom(void)
{
	rseed = 1;
	// safe0 start
	rseed_sp = 0;
	mti = CMATH_N + 1;
	// safe0 end
}

// Returns a number from 0 to n (excluding n)
unsigned int NvRandom::Random(unsigned int n)
{
	unsigned long y;
	static unsigned long mag01[2] = { 0x0, CMATH_MATRIX_A };

	if (n == 0)
		return(0);

	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= CMATH_N) { /* generate N words at one time */
		int kk;

		if (mti == CMATH_N + 1)   /* if sgenrand() has not been called, */
			SetRandomSeed(4357); /* a default initial seed is used   */

		for (kk = 0; kk < CMATH_N - CMATH_M; kk++) {
			y = (mt[kk] & CMATH_UPPER_MASK) | (mt[kk + 1] & CMATH_LOWER_MASK);
			mt[kk] = mt[kk + CMATH_M] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		for (; kk < CMATH_N - 1; kk++) {
			y = (mt[kk] & CMATH_UPPER_MASK) | (mt[kk + 1] & CMATH_LOWER_MASK);
			mt[kk] = mt[kk + (CMATH_M - CMATH_N)] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		y = (mt[CMATH_N - 1] & CMATH_UPPER_MASK) | (mt[0] & CMATH_LOWER_MASK);
		mt[CMATH_N - 1] = mt[CMATH_M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

		mti = 0;
	}

	y = mt[mti++];
	y ^= CMATH_TEMPERING_SHIFT_U(y);
	y ^= CMATH_TEMPERING_SHIFT_S(y) & CMATH_TEMPERING_MASK_B;
	y ^= CMATH_TEMPERING_SHIFT_T(y) & CMATH_TEMPERING_MASK_C;
	y ^= CMATH_TEMPERING_SHIFT_L(y);

	// ET - old engine added one to the result.
	// We almost NEVER wanted to use this function
	// like this.  So, removed the +1 to return a 
	// range from 0 to n (not including n).
	return (y%n);
}

float NvRandom::Random()
{
	float r = (float)Random(MAXINT);
	float divisor = (float)MAXINT;
	return (r / divisor);
}

void NvRandom::SetRandomSeed(unsigned int n)
{
	/* setting initial seeds to mt[n] using				*/
	/* the generator Line 25 of Table 1 in				*/
	/* [KNUTH 1981, The Art of Computer Programming     */
	/*     Vol. 2 (2nd Edition), pp102]					*/
	mt[0] = n & 0xffffffff;
	for (mti = 1; mti < CMATH_N; mti++)
		mt[mti] = (69069 * mt[mti - 1]) & 0xffffffff;

	rseed = n;
}

unsigned int NvRandom::GetRandomSeed(void)
{
	return(rseed);
}

void NvRandom::Randomize(void)
{
	SetRandomSeed((unsigned int)time(nullptr));
}

// ------------------------------------------------------------------------