#ifndef DEEPGARDEN_UTILITIES_H
#define DEEPGARDEN_UTILITIES_H

#include "deepgarden.h"

struct vec_u2
{
	unsigned int x;
	unsigned int y;
vec_u2(unsigned int a, unsigned int b);

};

struct vec_i2
{
	int x;
	int y;

vec_i2( int a,  int b);
};

struct vec_f2
{
	float x;
	float y;

vec_f2(float  a, float  b);
};

vec_f2 rotatePointPrecomputed( vec_f2 center, float s, float c, vec_f2 point);
int alphanumeric (char c);

float RNG();

float magnitude_int( int x,  int y);


template <typename U = uint64_t> class RandomizerWithSentinelShift ;

#endif