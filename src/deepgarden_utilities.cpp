#include "deepgarden.h"

//https://github.com/edrosten/8bit_rng/blob/master/rng-4261412736.c
uint32_t x, y, z, a;

vec_u2::vec_u2(unsigned int a, unsigned int b)
{
	this->x = a;
	this->y = b;
}

vec_i2::vec_i2( int a,  int b)
{
	this->x = a;
	this->y = b;
}

vec_f2::vec_f2( float a,  float b)
{
	this->x = a;
	this->y = b;
}

vec_f2 rotatePointPrecomputed( vec_f2 center, float s, float c, vec_f2 point)
{
	// translate point back to origin:
	point.x -= center.x;
	point.y -= center.y;

	// rotate point
	float xnew = point.x * c - point.y * s;
	float ynew = point.x * s + point.y * c;

	// translate point back:
	point.x = xnew + center.x;
	point.y = ynew + center.y;
	return vec_f2(point.x, point.y);
};

float magnitude_int( int x,  int y)
{
	float mag = sqrt(x * x + y * y);
	return mag;
}

int distanceBetweenPoints( vec_i2 a, vec_i2 b )
{
	int diffX = abs(a.x - b.x);
	int diffY = abs(a.y - b.y);
	return magnitude_int( diffX,  diffY);
}

// lookup table for character alphanumeric values
int alphanumeric (char c)
{
	int val = 0;

	switch (c)
	{
	case 'a': { val = 0; break; }
	case 'b': { val = 1; break; }
	case 'c': { val = 2; break; }
	case 'd': { val = 3; break; }
	case 'e': { val = 4; break; }
	case 'f': { val = 5; break; }
	case 'g': { val = 6; break; }
	case 'h': { val = 7; break; }
	case 'i': { val = 8; break; }
	case 'j': { val = 9; break; }
	case 'k': { val = 10; break; }
	case 'l': { val = 11; break; }
	case 'm': { val = 12; break; }
	case 'n': { val = 13; break; }
	case 'o': { val = 14; break; }
	case 'p': { val = 15; break; }
	case 'q': { val = 16; break; }
	case 'r': { val = 17; break; }
	case 's': { val = 18; break; }
	case 't': { val = 19; break; }
	case 'u': { val = 20; break; }
	case 'v': { val = 21; break; }
	case 'w': { val = 22; break; }
	case 'x': { val = 23; break; }
	case 'y': { val = 24; break; }
	case 'z': { val = 25; break; }

	case 'A': { val = 0; break; }
	case 'B': { val = 1; break; }
	case 'C': { val = 2; break; }
	case 'D': { val = 3; break; }
	case 'E': { val = 4; break; }
	case 'F': { val = 5; break; }
	case 'G': { val = 6; break; }
	case 'H': { val = 7; break; }
	case 'I': { val = 8; break; }
	case 'J': { val = 9; break; }
	case 'K': { val = 10; break; }
	case 'L': { val = 11; break; }
	case 'M': { val = 12; break; }
	case 'N': { val = 13; break; }
	case 'O': { val = 14; break; }
	case 'P': { val = 15; break; }
	case 'Q': { val = 16; break; }
	case 'R': { val = 17; break; }
	case 'S': { val = 18; break; }
	case 'T': { val = 19; break; }
	case 'U': { val = 20; break; }
	case 'V': { val = 21; break; }
	case 'W': { val = 22; break; }
	case 'X': { val = 23; break; }
	case 'Y': { val = 24; break; }
	case 'Z': { val = 25; break; }

	case '1': { val = 1; break; }
	case '2': { val = 2; break; }
	case '3': { val = 3; break; }
	case '4': { val = 4; break; }
	case '5': { val = 5; break; }
	case '6': { val = 6; break; }
	case '7': { val = 7; break; }
	case '8': { val = 8; break; }
	case '9': { val = 9; break; }
	// case '0': { val = 0; break; }

	case '`': { val = 1; break; }
	case '~': { val = 2; break; }
	case '!': { val = 3; break; }
	case '@': { val = 4; break; }
	case '#': { val = 5; break; }
	case '$': { val = 6; break; }
	case '%': { val = 7; break; }
	case '^': { val = 8; break; }
	case '&': { val = 9; break; }
	case '*': { val = 10; break; }
	case '(': { val = 11; break; }
	case ')': { val = 12; break; }
	case '-': { val = 13; break; }
	case '+': { val = 14; break; }
	case '[': { val = 15; break; }
	case ']': { val = 16; break; }
	case '{': { val = 17; break; }
	case '}': { val = 18; break; }
	case '|': { val = 19; break; }
	case ';': { val = 20; break; }
	case ':': { val = 21; break; }
	case '<': { val = 22; break; }
	case '>': { val = 23; break; }
	case ',': { val = 24; break; }
	case '.': { val = 25; break; }
	case '/': { val = 26; break; }
	case '?': { val = 0; break; }
	}
	return val;
}

// produce a letter that would correspond to a particular value.
char numeralphabetic (int i)
{
	if ( i >= 26) {return 'z';}
	switch (i)
	{
	case 1: {  return 'a'; break;  }
	case 2: {  return 'b'; break;  }
	case 3: {  return 'c'; break;  }
	case 4: {  return 'd'; break;  }
	case 5: {  return 'e'; break;  }
	case 6: {  return 'f'; break;  }
	case 7: {  return 'g'; break;  }
	case 8: {  return 'h'; break;  }
	case 9: {  return 'i'; break;  }
	case 10: {  return 'j'; break;  }
	case 11: {  return 'k'; break;  }
	case 12: {  return 'l'; break;  }
	case 13: {  return 'm'; break;  }
	case 14: {  return 'n'; break;  }
	case 15: {  return 'o'; break;  }
	case 16: {  return 'p'; break;  }
	case 17: {  return 'q'; break;  }
	case 18: {  return 'r'; break;  }
	case 19: {  return 's'; break;  }
	case 20: {  return 't'; break;  }
	case 21: {  return 'u'; break;  }
	case 22: {  return 'v'; break;  }
	case 23: {  return 'w'; break;  }
	case 24: {  return 'x'; break;  }
	case 25: {  return 'y'; break;  }
	}
	return 'a';
}

float RNG()
{
	static std::default_random_engine e;
	e.seed(std::chrono::system_clock::now().time_since_epoch().count());
	static std::uniform_real_distribution<> dis(0, 1);
	return dis(e);
}

inline uint32_t extremelyFastRandomByte()
{
	// it used to be an actual byte, but that makes it eventually run out of randomness and always choose the same number!!
	// mask off the top 8 if you really need a byte.
	uint32_t t = x ^ (x << 8);
	x = y;
	y = z;
	z = a;
	a = z ^ t ^ ( z >> 1) ^ (t << 1);

	return a;
}

uint32_t extremelyFastNumberInRange(uint32_t from, uint32_t to)
{
	return from + ( extremelyFastRandomByte() % ( to - from + 1 ) );
}

uint32_t extremelyFastNumberFromZeroTo( uint32_t to)
{
	return ( extremelyFastRandomByte() % ( to + 1 ) );
}

void seedExtremelyFastNumberGenerators()
{
	// x = !z;
	// y = !y;
	// z = !x;
	// a = 1;

	if (RNG() < 0.5)  {x = 1;}
	if (RNG() < 0.5)  {y = 1;}
	if (RNG() < 0.5)  {z = 1;}
}



