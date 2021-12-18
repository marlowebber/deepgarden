
#include "deepgarden_utilities.h"

	vec_u2::vec_u2(uint a, uint b)
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


Color::Color(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}


// lookup table for character alphanumeric values
int alphanumeric (char c)
{
	int val = 0;

	switch (c)
	{
	case 'a': { val = 1; break; }
	case 'b': { val = 2; break; }
	case 'c': { val = 3; break; }
	case 'd': { val = 4; break; }
	case 'e': { val = 5; break; }
	case 'f': { val = 6; break; }
	case 'g': { val = 7; break; }
	case 'h': { val = 8; break; }
	case 'i': { val = 9; break; }
	case 'j': { val = 10; break; }
	case 'k': { val = 11; break; }
	case 'l': { val = 12; break; }
	case 'm': { val = 13; break; }
	case 'n': { val = 14; break; }
	case 'o': { val = 15; break; }
	case 'p': { val = 16; break; }
	case 'q': { val = 17; break; }
	case 'r': { val = 18; break; }
	case 's': { val = 19; break; }
	case 't': { val = 20; break; }
	case 'u': { val = 21; break; }
	case 'v': { val = 22; break; }
	case 'w': { val = 23; break; }
	case 'x': { val = 24; break; }
	case 'y': { val = 25; break; }
	case 'z': { val = 26; break; }

	case 'A': { val = 1; break; }
	case 'B': { val = 2; break; }
	case 'C': { val = 3; break; }
	case 'D': { val = 4; break; }
	case 'E': { val = 5; break; }
	case 'F': { val = 6; break; }
	case 'G': { val = 7; break; }
	case 'H': { val = 8; break; }
	case 'I': { val = 9; break; }
	case 'J': { val = 10; break; }
	case 'K': { val = 11; break; }
	case 'L': { val = 12; break; }
	case 'M': { val = 13; break; }
	case 'N': { val = 14; break; }
	case 'O': { val = 15; break; }
	case 'P': { val = 16; break; }
	case 'Q': { val = 17; break; }
	case 'R': { val = 18; break; }
	case 'S': { val = 19; break; }
	case 'T': { val = 20; break; }
	case 'U': { val = 21; break; }
	case 'V': { val = 22; break; }
	case 'W': { val = 23; break; }
	case 'X': { val = 24; break; }
	case 'Y': { val = 25; break; }
	case 'Z': { val = 26; break; }

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
	case '?': { val = 27; break; }
	}

	return val;
}

float RNG()
{
	static std::default_random_engine e;
	static std::uniform_real_distribution<> dis(0, 1);
	return dis(e);
}

float magnitude_int( int x,  int y)
{
	float mag = sqrt(x * x + y * y);
	return mag;
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

// THE EXTREMELY FAST LINE ALGORITHM Variation E (Addition Fixed Point PreCalc Small Display)
// Small Display (256x256) resolution.
std::list<vec_u2> EFLA_E(vec_u2 start, vec_u2 end)
{
	int x  = start.x;
	int y  = start.y;
	int x2 = end.x;
	int y2 = end.y;

	std::list<vec_u2> v;

	bool yLonger = false;
	int shortLen = y2 - y;
	int longLen = x2 - x;
	if (abs(shortLen) > abs(longLen)) {
		int swap = shortLen;
		shortLen = longLen;
		longLen = swap;
		yLonger = true;
	}
	int decInc;
	if (longLen == 0) decInc = 0;
	else decInc = (shortLen << 8) / longLen;

	if (yLonger) {
		if (longLen > 0) {
			longLen += y;
			for (int j = 0x80 + (x << 8); y <= longLen; ++y) {
				// myPixel(surface,j >> 8,y);
				v.push_back( vec_u2(j >> 8, y) );
				j += decInc;
			}
			return v;
		}
		longLen += y;
		for (int j = 0x80 + (x << 8); y >= longLen; --y) {
			// myPixel(surface,j >> 8,y);
			v.push_back( vec_u2(j >> 8, y) );
			j -= decInc;
		}
		return v;
	}

	if (longLen > 0) {
		longLen += x;
		for (int j = 0x80 + (y << 8); x <= longLen; ++x) {
			// myPixel(surface,x,j >> 8);
			v.push_back( vec_u2(x, j >> 8) );
			j += decInc;
		}
		return v;
	}
	longLen += x;
	for (int j = 0x80 + (y << 8); x >= longLen; --x) {
		// myPixel(surface,x,j >> 8);
		v.push_back( vec_u2(x, j >> 8) );
		j -= decInc;
	}

	return v;
}
