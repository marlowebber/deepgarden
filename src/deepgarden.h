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

#include <boost/thread.hpp>


// #include "deepgarden.h"
#include "deepgarden_utilities.h"


#define sizeX 8096
#define sizeY 256

struct Color
{
	float r;
	float g;
	float b;
	float a;

	Color(float r, float g, float b, float a);
}__attribute__((packed));


#define MATERIAL_VACUUM		1
#define MATERIAL_PHOTON  	2       
#define MATERIAL_SEED 		3
#define MATERIAL_IRON 		4
#define MATERIAL_STONE 		5
#define MATERIAL_OXYGEN 	6
#define MATERIAL_GOLD 		7
#define MATERIAL_WATER 		8
#define MATERIAL_QUARTZ 	9
#define MATERIAL_AMPHIBOLE 	10
#define MATERIAL_OLIVINE 	11


#define PHASE_VACUUM 1
#define PHASE_SOLID 2
#define PHASE_POWDER 4
#define PHASE_LIQUID 8
#define PHASE_GAS    16
#define PHASE_LIGHT 32

#define DARK 0
#define LIGHT 1

float RNG();

void initialize ();


void setPointSize (unsigned int pointSize) ;

void thread_graphics () ;
void thread_physics ();
void thread_chemistry ();
void thread_optics ();
void thread_particledrawing ();

void heatEverything () ;
void coolEverything();

// void thread_temperature ();

void setNeutralTemp () ;

void thread_temperature2 ();


void thread_life();
void thread_seeds();
#endif