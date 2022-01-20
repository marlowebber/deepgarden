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


#define sizeX 2048
#define sizeY 256
#define maxGenomeSize 64
#define N_NEIGHBOURS 8                 // this is always 8 on a 2D square grid. 

struct Color
{
	float r;
	float g;
	float b;
	float a;

	Color(float r, float g, float b, float a);
}__attribute__((packed));


#define MATERIAL_VACUUM		 1
#define MATERIAL_PHOTON  	 2       
#define MATERIAL_SEED 		 3
#define MATERIAL_IRON 		 4
#define MATERIAL_STONE 		 5
#define MATERIAL_OXYGEN 	 6
#define MATERIAL_GOLD 		 7
#define MATERIAL_WATER 		 8
#define MATERIAL_QUARTZ 	 9
#define MATERIAL_AMPHIBOLE 	 10
#define MATERIAL_OLIVINE 	 11
#define MATERIAL_FIRE        12

#define PHASE_NULL			 0
#define PHASE_VACUUM 		 1
#define PHASE_SOLID  		 2
#define PHASE_POWDER 		 4
#define PHASE_LIQUID 		 8
#define PHASE_GAS    		 16
#define PHASE_LIGHT  		 32
		 
#define STAGE_NULL   		 0
#define STAGE_BUD    		 1
#define STAGE_FRUIT  		 2
#define STAGE_SEED   		 3
#define STAGE_PHOTON 		 4
#define STAGE_ANIMALSEED	 5
#define STAGE_ANIMAL         6
#define STAGE_PLAYER		 7
#define STAGE_ERODINGRAIN    8

#define ENERGYSOURCE_LIGHT   1
#define ENERGYSOURCE_MINERAL 2
#define ENERGYSOURCE_SEED    4
#define ENERGYSOURCE_PLANT   8
#define ENERGYSOURCE_ANIMAL  16

#define FRAME_A 1
#define FRAME_B 2
#define FRAME_C 3

#define MOVEMENT_ONPOWDER  1
#define MOVEMENT_INWATER   2
#define MOVEMENT_INPLANTS  4
#define MOVEMENT_INAIR     8
#define MOVEMENT_ONSOLID   16

float RNG();

void initialize ();

void toggleErodingRain () ;
void setPointSize (unsigned int pointSize) ;

void thread_graphics () ;
void thread_physics ();
void thread_chemistry ();
void thread_optics ();
void thread_particledrawing ();

void heatEverything () ;
void coolEverything();

void setNeutralTemp () ;

void thread_temperature2 ();


void thread_life();
void thread_seeds();


void sendLifeToBackground ();

void insertRandomSeed();

void toggleEnergyGridDisplay ();


void increaseLampBrightness ();

void decreaseLampBrightness ();


void thread_plantDrawing();

void save();

void load();


void drawAHill();
void drawRandomLandscape();
#endif