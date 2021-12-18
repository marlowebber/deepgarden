#ifndef DEEPGARDEN_GRAPHICS_H
#define DEEPGARDEN_GRAPHICS_H

#include "deepgarden.h"

extern float viewZoomSetpoint ;
extern float viewPanSetpointX;
extern float viewPanSetpointY;


void setupGraphics() ;
void shutdownGraphics() ;
void draw( uint ** grid );

void preDraw() ;
void postDraw();


void vertToBuffer (GLfloat * vertex_buffer_data, uint * cursor, Color vert_color, uint x, uint y);


void advanceIndexBuffers (uint * index_buffer_data, uint * index_buffer_content, uint * index_buffer_cursor);




#endif