#ifndef DEEPGARDEN_H
#define DEEPGARDEN_H

#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "SDL.h"
#include <random>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "deepgarden_utilities.h"

using namespace glm;




// #include "deepgarden.h"
// #include "deepgarden_graphics.h"

#define sizeX 1024
#define sizeY 256

void initialize ();
// void deepgardenLoop();
void deepgardenLoop();

#endif