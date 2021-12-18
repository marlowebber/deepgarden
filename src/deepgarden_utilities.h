#ifndef DEEPGARDEN_UTILITIES_H
#define DEEPGARDEN_UTILITIES_H
#include <list>

#include <random>



# include "deepgarden.h"
struct vec_u2
{
	uint x;
	uint y;

	vec_u2(uint a, uint b);
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

	vec_f2( float a,  float b);
};


struct Color
{
	float r;
	float g;
	float b;
	float a;

	Color(float r, float g, float b, float a);
};

float RNG();

float magnitude_int( int x,  int y);


vec_f2 rotatePointPrecomputed( vec_f2 center, float s, float c, vec_f2 point);

// lookup table for character alphanumeric values
int alphanumeric (char c);

std::list<vec_u2> EFLA_E(vec_u2 start, vec_u2 end);

#endif