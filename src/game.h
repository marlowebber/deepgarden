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
	b2Color color;

	std::vector<b2Vec2>  vertices;

	Branch * owner;

	PhysicalObject (std::vector<b2Vec2>   vertices, bool flagStatic);
};

struct Tree;

struct Branch
{
	float energyValue;

	b2Color color;
	float naturalAngle;

	std::list<Branch> branches;

	bool seed;
	bool stem;

	bool expressed;

	unsigned int capturedLight;

	std::vector<b2Vec2> vertices =
	{
		b2Vec2( +1.0f ,  -1.0f),
		b2Vec2( -1.0f ,  -1.0f), 
		b2Vec2( -1.0f ,  +1.0f),
		b2Vec2( +1.0f ,  +1.0f) 
	};

	PhysicalObject object = PhysicalObject(  vertices, false);

	float rootThickness;
	float tipThickness;
	float length;

	unsigned int geneCursorStartPosition;
	unsigned int geneCursor;

	b2RevoluteJointDef rjointDef;
	b2RevoluteJoint * p_rjoint;

	b2DistanceJointDef djointDef;
	b2DistanceJoint * p_djoint;

	b2WeldJointDef wjointDef;
	b2WeldJoint * p_wjoint;

	Tree * owner;

	bool flagDelete;

	float rootThicknessDelta  		;
	float tipThicknessDelta  		;
	float lengthDelta 			;
	float angleDelta			;
	b2Color colorDelta		;

	Branch(float rootThickness, float tipThickness, float length, float naturalAngle, b2Color color );

};

struct Tree
{
	float energyStored;

	std::string genes;

	unsigned long int lastReproduced;

	b2Vec2 sproutPosition;
	PhysicalObject * affixedObject;

	std::list<Branch> branches;

	bool mature;

	bool germinated;

	bool flagDelete;

	Branch * lastGrownBranch;

	unsigned int geneCursor;
	Tree(std::string genes);
};

void initializeGame ();


void rebuildMenus ();


void threadGame();


void gameGraphics() ;

#endif