#ifndef PHYSICS_H
#define PHYSICS_H

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

#include "game.h"

// #define THREAD_TIMING

// struct Color
// {
// 	float r;
// 	float g;
// 	float b;
// 	float a;

// 	Color(float r, float g, float b, float a);
// };

;
// extern b2MouseJoint* m_mouseJoint;


extern b2World * m_world;



extern std::list<b2Body* > rayContacts;
extern std::list<PhysicalObject*> physicalObjects;

void initializePhysics ();

void threadPhysics ();

// void threadGame () ;

void threadGraphics () ;

void maintainMouseJoint (b2Vec2 p);

void destroyMouseJoint ();

bool getMouseJointStatus () ;


void shine (b2Vec2 p1, b2Vec2 p2);
void createJoint(PhysicalObject * a, PhysicalObject * b);

void exampleMenuCallback(void * userData);
int checkClickObjects (b2Vec2 worldClick);


void addToWorld(PhysicalObject * object, b2Vec2 position, float angle);

void deleteFromWorld (PhysicalObject * object);

#endif