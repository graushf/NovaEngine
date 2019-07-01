#pragma once

// -------------------------------------------------------------------
// Includes
// -------------------------------------------------------------------

//#include "Common/CommonStd.h"
#include "Common/types.h"
#include <vector>

// -------------------------------------------------------------------
// Defines
// -------------------------------------------------------------------

// -------------------------------------------------------------------

/* Period paramters */
#define CMATH_N 624
#define CMATH_M 397
#define CMATH_MATRIX_A 0x9908b0df   /* constant vector a */

#define CMATH_UPPER_MASK 0x80000000 /* most significant w-r bits */
#define CMATH_LOWER_MASK 0x7fffffff /* least significant r bits */

#define CMATH_TEMPERING_MASK_B 0x9d2c5680
#define CMATH_TEMPERING_MASK_C 0xefc60000
#define CMATH_TEMPERING_SHIFT_U(y)  (y >> 11)
#define CMATH_TEMPERING_SHIFT_S(y)  (y << 7)
#define CMATH_TEMPERING_SHIFT_T(y)  (y << 15)
#define CMATH_TEMPERING_SHIFT_L(y)  (y >> 18)

#define RADIANS_TO_DEGREES(x) ((x) * 180.0f / Nv_PI)
#define DEGREES_TO_RADIANS(x) ((x) * Nv_PI / 180.0f)

class NvRandom
{
private:
	// DATA
	unsigned int rseed;
	unsigned int rseed_sp;
	unsigned long mt[CMATH_N];  /* the array for the state vector */
	int mti;					/* mti==N+1 means mt[N] is not initialized */

	// FUNCTIONS
public:
	NvRandom(void);

	unsigned int		Random(unsigned int n);
	float				Random();
	void				SetRandomSeed(unsigned int n);
	unsigned int		GetRandomSeed(void);
	void				Randomize(void);
};

/*
struct Point
{ 
	int x, y;
	Point() { x = y = 0; }
	Point(int _x, int _y) { x = _x; y = _y; }
*/

typedef std::vector<Point> Poly;

class Math
{
	// DATA
private:

public:
	static NvRandom random;

	// FUNCTIONS
public:

	static bool PointInPoly(Point const& test, const Poly& polygon);
	static bool PointInPoly(int const x, int const y, int const * const vertex, int const nvertex);
};