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
#include "deepgarden_utilities.h"


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


#define MATERIAL_VACUUM	1
#define MATERIAL_PHOTON          		  2  // 2^1, bit 1

#define MATERIAL_IRON 4
#define MATERIAL_STONE 8
#define MATERIAL_OXYGEN 16


#define PHASE_VACUUM 1
#define PHASE_SOLID 2
#define PHASE_POWDER 4
#define PHASE_LIQUID 8
#define PHASE_GAS    16
#define PHASE_LIGHT 32

#define DARK 1
#define LIGHT 2

float RNG();

void initialize ();

// void deepgardenLoop();
void deepgardenLoop();

#endif