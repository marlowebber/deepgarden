#ifndef DEEPGARDEN_H
#define DEEPGARDEN_H

#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "SDL.h"
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <random>


// #include "deepgarden.h"
// #include "deepgarden_graphics.h"


#define sizeX 1024
#define sizeY 256

struct color
{
	float r;
	float g;
	float b;
	float a;

	color(float r, float g, float b, float a);
};



#define MATERIAL_VACUUM               1  // 2^0, bit 0
#define MATERIAL_PHOTON          		  2  // 2^1, bit 1

#define MATERIAL_LIFE_SOLID        	  4  // 2^1, bit 1
#define MATERIAL_LIFE_POWDER		  8
#define MATERIAL_LIFE_LIQUID		 16
#define MATERIAL_LIFE_GAS		     32

#define MATERIAL_STONE                64  // 2^1, bit 1
#define MATERIAL_GOLD                128  // 2^1, bit 1


float RNG();

void initialize ();

void deepgardenLoop();
void deepgardenGraphics();

#endif