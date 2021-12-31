
#ifndef GAME_H
#define GAME_H


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

#include "utilities.h"

#include <box2d.h>

#include "physics.h"
#include "game.h"




#define TYPE_BRANCH  (1<<1)
#define TYPE_TERRAIN (1<<2)



extern float thicknessCursor  		;
extern float lengthCursor 			;
extern float angleCursor 			;
extern b2Color colorCursor 		;
extern b2Vec2 worldPositionCursor 	;


struct Branch;

struct PhysicalObject
{
	b2BodyDef bodyDef;
	b2Body * p_body;
	b2PolygonShape shape;
	b2Fixture * p_fixture;
	float fraction;
	bool flagDelete;
	// bool flagReady;
	b2Color color;

	b2RevoluteJointDef jointDef;
	b2RevoluteJoint * p_joint;

	std::vector<b2Vec2>  vertices;

	Branch * owner;

	PhysicalObject (std::vector<b2Vec2>   vertices, bool flagStatic);
};


struct Tree;
// struct PhysicalObject (std::vector<b2Vec2>   vertices, bool flagStatic) ;

struct Branch
{
	float energyValue;

	b2Color color;
	float naturalAngle;

	std::list<Branch> branches;

	bool seed;

	bool stem;

	// bool ready;

	unsigned int capturedLight;

	std::vector<b2Vec2> vertices =//
	{
		b2Vec2( +1.0f ,  -1.0f), //b2Vec2 rootVertexA =
		b2Vec2( -1.0f ,  -1.0f), // b2Vec2 rootVertexB =
		b2Vec2( -1.0f ,  +1.0f), //b2Vec2 tipVertexA =
		b2Vec2( +1.0f ,  +1.0f) // b2Vec2 tipVertexB =
	};

	PhysicalObject object = PhysicalObject(  vertices, false);


	// Branch();

	Tree * owner;

	bool flagDelete;

	Branch();

};



struct Tree
{

	float energyStored;

	std::string genes;

	unsigned long int lastReproduced;

	// bool ready;

	b2Vec2 sproutPosition;
	PhysicalObject * affixedObject;


	std::list<Branch> branches;


	bool mature;

	bool germinated;

	uint geneCursor;

	bool flagDelete;

	Tree(std::string genes);



};

void initializeGame ();


void rebuildMenus ();


void threadGame();


void gameGraphics() ;

#endif