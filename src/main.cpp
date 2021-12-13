#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "SDL.h"
#include <vector>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <random>

bool paused;
static const unsigned int nominalFramerate = 60;
static const unsigned int width = 1920;
static const unsigned int height = 1080;
float viewZoom = 10.0f;
float viewZoomSetpoint = 1000.0f;

float viewPanX = 0.0f;
float viewPanY = 0.0f;
float viewPanSetpointX = 0.0f;
float viewPanSetpointY = 0.0f;

float cameraTrackingResponse = 10;

SDL_Window * window;
SDL_GLContext context;

GLuint vs, fs, program;
GLuint vao, vbo;
GLuint IndexBufferId;

static const char * vertex_shader =
    "#version 130\n"
    "in vec2 i_position;\n"
    "in vec4 i_color;\n"
    "out vec4 v_color;\n"
    "uniform mat4 u_projection_matrix;\n"
    "void main() {\n"
    "    v_color = i_color;\n"
    "    gl_Position = u_projection_matrix * vec4( i_position, 0.0, 1.0 );\n"
    "}\n";

static const char * fragment_shader =
    "#version 130\n"
    "in vec4 v_color;\n"
    "out vec4 o_color;\n"
    "void main() {\n"
    "    o_color = v_color;\n"
    "}\n";

typedef enum t_attrib_id
{
	attrib_position,
	attrib_color
} t_attrib_id;


int mouseX;
int mouseY;

#define sizeX 1024
#define sizeY 256

static unsigned int grid[sizeX][sizeY];

#define MATERIAL_VACUUM                1  // 2^0, bit 0
#define MATERIAL_SAND          		  2  // 2^1, bit 1

#define MATERIAL_LIFE          		  4  // 2^1, bit 1

struct color
{
	float r;
	float g;
	float b;
	float a;

	color(float r, float g, float b, float a);
};

color::color(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}


unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

typedef float t_mat4x4[16];
static inline void mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar )
{
#define T(a, b) (a * 4 + b)

	out[T(0, 0)] = 2.0f / (right - left);
	out[T(0, 1)] = 0.0f;
	out[T(0, 2)] = 0.0f;
	out[T(0, 3)] = 0.0f;

	out[T(1, 1)] = 2.0f / (top - bottom);
	out[T(1, 0)] = 0.0f;
	out[T(1, 2)] = 0.0f;
	out[T(1, 3)] = 0.0f;

	out[T(2, 2)] = -2.0f / (zfar - znear);
	out[T(2, 0)] = 0.0f;
	out[T(2, 1)] = 0.0f;
	out[T(2, 3)] = 0.0f;

	out[T(3, 0)] = -(right + left) / (right - left);
	out[T(3, 1)] = -(top + bottom) / (top - bottom);
	out[T(3, 2)] = -(zfar + znear) / (zfar - znear);
	out[T(3, 3)] = 1.0f;

#undef T
}

// The projection matrix efficiently handles all panning, zooming, and rotation.
t_mat4x4 projection_matrix;




color sandColor = color(0.2f, 0.2f, 0.2f, 1.0f);
color emptyColor = color(0.05f, 0.05f, 0.05f, 1.0f);

color lifeColor = color(0.2f, 0.05f, 0.4f, 1.0f);

float RNG()
{
	static std::default_random_engine e;
	static std::uniform_real_distribution<> dis(0, 1);
	return dis(e);
}

void quit ()
{
	SDL_GL_DeleteContext( context );
	SDL_DestroyWindow( window );
	SDL_Quit();
}

void togglePause ()
{
	paused = !paused;
}
void prepareForWorldDraw ()
{
	glUseProgram( program );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glEnableVertexAttribArray( attrib_position );
	glEnableVertexAttribArray( attrib_color );
	glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
	glVertexAttribPointer( attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )(4 * sizeof(float)) );

	// mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar )
	mat4x4_ortho(
	    projection_matrix,
	    (-1 * viewZoom) + viewPanX,
	    (+1 * viewZoom) + viewPanX,
	    (-1 * 0.6 * viewZoom) + viewPanY,
	    (+1 * 0.6 * viewZoom) + viewPanY,
	    -10.0f,
	    +10.0f
	);
	glUniformMatrix4fv( glGetUniformLocation( program, "u_projection_matrix" ), 1, GL_FALSE, projection_matrix );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void cleanupAfterWorldDraw ()
{
	glDisable(GL_BLEND);
	glDisableVertexAttribArray(attrib_position);
	glDisableVertexAttribArray(attrib_color);
}

void initialize ()
{

	memset( grid, 0x00, (sizeof(unsigned int) * sizeX * sizeY) );

	for (unsigned int x = 0; x < sizeX; ++x)
	{
		for (unsigned int y = 0; y < sizeY; ++y)
		{
			if (RNG() > 0.5)
			{
				grid[x][y] = MATERIAL_SAND;
			}
			else
			{
				grid[x][y] = MATERIAL_VACUUM;
			}

			if (RNG() < 0.0005)
			{
				grid[x][y] = MATERIAL_LIFE;
			}

		}
	}
}

void setupGraphics()
{
	/**
	You have to bind and unbind the buffer to copy to it from 'C' and draw with it from openGL. Think of it as locking/unlocking between the program and the graphics.

	So the sequence is:
	create
	bind
	stuff data
	unbind

	bind
	display
	unbind

	https://stackoverflow.com/questions/8599264/why-the-second-call-to-glbindbuffer
	 * */

	vs = glCreateShader( GL_VERTEX_SHADER );
	fs = glCreateShader( GL_FRAGMENT_SHADER );

	int length = strlen( vertex_shader );
	glShaderSource( vs, 1, ( const GLchar ** )&vertex_shader, &length );
	glCompileShader( vs );

	GLint status;
	glGetShaderiv( vs, GL_COMPILE_STATUS, &status );
	if ( status == GL_FALSE )
	{
		fprintf( stderr, "vertex shader compilation failed\n" );
	}

	length = strlen( fragment_shader );
	glShaderSource( fs, 1, ( const GLchar ** )&fragment_shader, &length );
	glCompileShader( fs );

	glGetShaderiv( fs, GL_COMPILE_STATUS, &status );
	if ( status == GL_FALSE )
	{
		fprintf( stderr, "fragment shader compilation failed\n" );
	}

	program = glCreateProgram();
	glAttachShader( program, vs );
	glAttachShader( program, fs );

	glBindAttribLocation( program, attrib_position, "i_position" );
	glBindAttribLocation( program, attrib_color, "i_color" );
	glLinkProgram( program );

	glDisable( GL_DEPTH_TEST );
	glViewport( 0, 0, width, height );

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xffff);

	glGenBuffers(1, &IndexBufferId);
	glGenVertexArrays( 1, &vao );
	glGenBuffers( 1, &vbo );

	glUseProgram( program );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId);
	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );

	glEnableVertexAttribArray( attrib_position );
	glEnableVertexAttribArray( attrib_color );

	glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
	glVertexAttribPointer( attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )(4 * sizeof(float)) );

	glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
}

void vertToBuffer (GLfloat * vertex_buffer_data, unsigned int * cursor, color vert_color, unsigned int x, unsigned int y)
{
	float floatx = x;
	float floaty = y;
	vertex_buffer_data[(*cursor) + 0] = vert_color.r;
	vertex_buffer_data[(*cursor) + 1] = vert_color.g;
	vertex_buffer_data[(*cursor) + 2] = vert_color.b;
	vertex_buffer_data[(*cursor) + 3] = vert_color.a;
	vertex_buffer_data[(*cursor) + 4] = floatx;
	vertex_buffer_data[(*cursor) + 5] = floaty;
	(*cursor) += 6;
}

void advanceIndexBuffers (unsigned int * index_buffer_data, unsigned int * index_buffer_content, unsigned int * index_buffer_cursor)
{
	index_buffer_data[(*index_buffer_cursor)] = (*index_buffer_content);
	(*index_buffer_cursor)++;
	(*index_buffer_content)++;
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	unsigned int nVertsToRenderThisTurn = 4 * sizeX * sizeY;
	unsigned int nIndicesToUseThisTurn 	= 5 * sizeX * sizeY;
	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

	// Create the buffer.
	unsigned int g_vertex_buffer_cursor = 0;

	// printf("fields %u\n", totalNumberOfFields);

	float * vertex_buffer_data  = new float[totalNumberOfFields];

	unsigned int index_buffer_cursor = 0;
	unsigned int index_buffer_content = 0;
	unsigned int index_buffer_data[nIndicesToUseThisTurn];

	prepareForWorldDraw();

	for (unsigned int x = 0; x < sizeX; ++x)
	{
		for (unsigned int y = 0; y < sizeY; ++y)
		{

			color colorToUse = emptyColor;

			if ((grid[x][y] & (MATERIAL_SAND)) == (MATERIAL_SAND))
			{
				colorToUse = sandColor;

			}

			if ((grid[x][y] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
			{
				colorToUse = emptyColor;

			}


				if ((grid[x][y] & (MATERIAL_LIFE)) == (MATERIAL_LIFE))
			{
				colorToUse = lifeColor;

			}



			vertToBuffer ( vertex_buffer_data, &g_vertex_buffer_cursor, colorToUse , x + 1,  y);
			advanceIndexBuffers(index_buffer_data, &index_buffer_content, &index_buffer_cursor);


			vertToBuffer ( vertex_buffer_data, &g_vertex_buffer_cursor, colorToUse , x,  y);
			advanceIndexBuffers(index_buffer_data, &index_buffer_content, &index_buffer_cursor);

			vertToBuffer ( vertex_buffer_data, &g_vertex_buffer_cursor, colorToUse , x + 1,  y + 1);
			advanceIndexBuffers(index_buffer_data, &index_buffer_content, &index_buffer_cursor);


			vertToBuffer ( vertex_buffer_data, &g_vertex_buffer_cursor, colorToUse , x,  y + 1);
			advanceIndexBuffers(index_buffer_data, &index_buffer_content, &index_buffer_cursor);



			index_buffer_data[(index_buffer_cursor)] = 0xffff;
			(index_buffer_cursor)++;




		}
	}

	glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, vertex_buffer_data, GL_DYNAMIC_DRAW );
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int ) * nIndicesToUseThisTurn, index_buffer_data, GL_DYNAMIC_DRAW);

	glDrawElements( GL_TRIANGLE_STRIP, nIndicesToUseThisTurn, GL_UNSIGNED_INT, index_buffer_data );

	cleanupAfterWorldDraw();

	SDL_GL_SwapWindow( window );

	delete [] vertex_buffer_data;
}


void deepgardenLoop()
{
	for (unsigned int x = 1; x < sizeX - 1; ++x)
	{
		for (unsigned int y = 1; y < sizeY - 1; ++y)
		{
			if ((grid[x][y] & (MATERIAL_SAND)) == (MATERIAL_SAND))
			{

				if ((grid[x][y - 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x][y - 1] = MATERIAL_SAND;
					grid[x][y] = MATERIAL_VACUUM;
				}

				else if ((grid[x - 1][y - 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x - 1][y - 1] = MATERIAL_SAND;
					grid[x][y] = MATERIAL_VACUUM;
				}

				else if ((grid[x + 1][y - 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x + 1][y - 1] = MATERIAL_SAND;
					grid[x][y] = MATERIAL_VACUUM;
				}
			}

			if ((grid[x][y] & (MATERIAL_LIFE)) == (MATERIAL_LIFE))
			{


				if ((grid[x][y - 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x][y - 1]  = MATERIAL_LIFE;
				}
				// else if ((grid[x ][y + 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				// {
				// 	grid[x ][y + 1]  = MATERIAL_LIFE;
				// }
				else if ((grid[x + 1][y ] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x + 1][y ]  = MATERIAL_LIFE;
				}
				else if ((grid[x - 1][y] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x - 1][y ]  = MATERIAL_LIFE;
				}
			

			}

		}
	}
}

int main( int argc, char * argv[] )
{
	// Setup the game window with SDL2
	SDL_Init( SDL_INIT_VIDEO );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	window = SDL_CreateWindow( "", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	context = SDL_GL_CreateContext( window );


	initialize();
	setupGraphics();

	for ( ;; )
	{
		SDL_Event event;
		while ( SDL_PollEvent( &event ) )
		{
			switch ( event.type )
			{

			case SDL_KEYDOWN:
			{
				switch ( event.key.keysym.sym )
				{

				case SDLK_LEFT:
					viewPanSetpointX = viewPanSetpointX - (1.0f * viewZoom  );
					break;
				case SDLK_RIGHT:
					viewPanSetpointX = viewPanSetpointX + (1.0f * viewZoom  );
					break;
				case SDLK_UP:
					viewPanSetpointY = viewPanSetpointY + (1.0f * viewZoom  );
					break;
				case SDLK_DOWN:
					viewPanSetpointY = viewPanSetpointY - (1.0f * viewZoom  );
					break;
				case SDLK_EQUALS:
					viewZoomSetpoint = viewZoomSetpoint * 0.9f;
					break;
				case SDLK_MINUS:
					viewZoomSetpoint = viewZoomSetpoint * 1.1f;
					break;
				case SDLK_ESCAPE:
					quit();
				}
				break;
			}

			case SDL_MOUSEBUTTONDOWN:
			{
				switch (event.button.button)
				{
				case SDL_BUTTON_LEFT:
				{
					break;
				}

				}
				break;
			}
			}
		}

		if (!paused)
		{
			// the gameplay loop
			deepgardenLoop();
		}

		// graphics loop
		draw();

		float zoomDifference = (viewZoom - viewZoomSetpoint);
		float zoomResponse = (zoomDifference * -1) / cameraTrackingResponse;
		viewZoom = viewZoom + zoomResponse;

		float panDifferenceX = (viewPanX - viewPanSetpointX);
		float panResponseX = (panDifferenceX * -1) / cameraTrackingResponse;
		float panDifferenceY = (viewPanY - viewPanSetpointY);
		float panResponseY = (panDifferenceY * -1) / cameraTrackingResponse;
		viewPanX +=  panResponseX ;
		viewPanY += panResponseY ;

	}

	return 0;
}
