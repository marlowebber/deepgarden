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

#include "deepgarden_utilities.h"

extern  const unsigned int totalSize;

#define sizeX 8192
#define sizeY 256
#define maxGenomeSize 64
#define N_NEIGHBOURS 8                 // this is always 8 on a 2D square grid. 
#define maxLampBrightness 100

#define sizeAnimalSprite 16
#define squareSizeAnimalSprite 256     // these are here to make lookup fast. so you don't have to calculate them.
#define halfSizeAnimalSprite 8

#define maxAnimalSegments 26
#define maxTimesReproduced 5

#define numberOfFairTurnsPerSegment 8

#define pressureScale 10



#define NUMBER_OF_FRAMES     1

struct Color
{
	float r;
	float g;
	float b;
	float a;

	Color(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f);
} __attribute__((packed));


// Material is what flavor a square can be. It controls color, melting temperature, and phase change rules
extern const unsigned int MATERIAL_VACUUM ;
#define MATERIAL_WATER 1
#define MATERIAL_BLOOD 2
#define MATERIAL_BONE 3

// Phase is what set of physical rules are applied to each square each turn, and is used by the program to determine how to treat the square in many other situations.
#define PHASE_NULL			 0
#define PHASE_VACUUM 		 1
#define PHASE_SOLID  		 2
#define PHASE_POWDER 		 4
#define PHASE_LIQUID 		 8
#define PHASE_GAS    		 16
#define PHASE_LIGHT  		 32

// Stage is like material, but applies to the seed grid instead, which is used for objects that can fall or move in front of the other grids.
#define STAGE_NULL   		 0
#define STAGE_BUD    		 1
#define STAGE_FRUIT  		 2
#define STAGE_SPROUT   		 3
#define STAGE_PHOTON 		 4
#define STAGE_ANIMALSEED	 5
#define STAGE_ANIMAL         6
#define STAGE_PLAYER		 7
#define STAGE_ERODINGRAIN    8

// Energysource is the diet of a plant or animal.
#define ENERGYSOURCE_LIGHT   1
#define ENERGYSOURCE_MINERAL 2
#define ENERGYSOURCE_SEED    4
#define ENERGYSOURCE_PLANT   8
#define ENERGYSOURCE_ANIMAL  16

// Frames are the sprites used to animate animals.
#define FRAME_BODY 0 // the 0th frame is the animal's body and the arrangement of its internal organs.



// Movement controls where an animal is allowed to go. This is a hereditary, mutable quality.
#define MOVEMENT_ONPOWDER  1
#define MOVEMENT_INLIQUID  2
#define MOVEMENT_INPLANTS  4
#define MOVEMENT_INAIR     8
#define MOVEMENT_ONSOLID   16

// Visualizers are what picture is shown on the screen. They can be used for art or for scientific purposes.
#define VISUALIZE_MATERIAL    1
#define VISUALIZE_ENERGY      2
#define VISUALIZE_TEMPERATURE 3
#define VISUALIZE_PHASE       4
#define VISUALIZE_PRESSURE	  5
#define NUMBER_OF_VISUALIZERS 5 // i use a ++ to scroll through the list of visualizers. Please keep the declarations in numeric order and update this if you add more.

// logical conditions used for crystallisation.
#define CONDITION_GREATERTHAN      0
#define CONDITION_EQUAL            1
#define CONDITION_LESSTHAN         2
#define CONDITION_EVENNUMBER       3
#define CONDITION_ODDNUMBER        4
#define CONDITION_CORNER           5
#define CONDITION_EDGE             6
#define CONDITION_ROW              7
#define CONDITION_LEFTN            8
#define CONDITION_NOTLEFTRIGHTN    9
#define CONDITION_NOTLRNEIGHBOURS  10

#define NUMBER_OF_CONDITIONS 12

// animal personalities make them respond to situations differently
#define PERSONALITY_AGGRESSIVE 1  // always attack other animals.
#define PERSONALITY_COWARDLY   2  // run away from other animals all the time.
#define PERSONALITY_FLOCKING   4  // copy direction from nearby animals.
#define PERSONALITY_FRIENDLY   8  // run toward nearby animals when not hungry.

// animal organs are polygons of different colors that provide different functionality.
#define ORGAN_NOTHING 0 // empty
#define ORGAN_EYE	  1	// used for the vision radius
#define ORGAN_MUSCLE  2	// used for movement
#define ORGAN_MOUTH   4  // used to eat and attack
#define ORGAN_LIVER	  8  // used for hit points and energy storage
#define ORGAN_BONE	  16	// used for defence and is left behind when the animal is killed
#define ORGAN_VACUOLE 32 // an empty organ that is removed to leave behind shaped voids.
// #define ORGAN_CHEMOSENSOR 64 // an organ that can detect smells
#define ORGAN_HEART   64 // allows the animal to move more often
#define ORGAN_WEAPON   128 // allows the animal to move more often

#define ORGAN_MARKER_A 1024 // used for the polygon filling algorithm. Do not use in gene codes! must be different to all the other organ codes!




// worlds are specifications for sets of materials and environment conditions.

#define WORLD_EARTH 1
#define WORLD_RANDOM 2


void createWorld( unsigned int world);
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

void setExtremeTempPoint (unsigned int x , unsigned  int y);
void sendLifeToBackground ();

void insertRandomSeed();

void insertRandomAnimal ();

void toggleEnergyGridDisplay ();

void increaseLampBrightness ();

void decreaseLampBrightness ();

void thread_plantDrawing();

void thread_weather();

void thread_animals();

void save();

void load();
void clearGrids() ;

void setEverythingHot();

void manualErode();

void eraseFallenSeeds();

void drawAHill();
void drawRandomLandscape();

void createRandomWorld();

void clearGases();
void clearLiquids();

void dropAllSeeds();

void eraseAllLife();


void animalCrudOps(unsigned int i);
bool isAnimal(unsigned int i);

#endif