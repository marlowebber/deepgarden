#include "deepgarden.h"
#include "deepgarden_graphics.h"

uint numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

struct Material;
struct Cell;
struct Plant;
struct Color;


uint roomTemperature = 294;




enum Phases 
{
	solid,
	liquid,
	gas,
	plasma
};

enum Materials 
{
	water
};

struct Material 
{
	
	Materials material;

	uint melting;
	uint boiling;

	Color color = Color(0.5f, 0.5f, 0.5f, 1.0f);

	Material(Materials material, uint melting, uint boiling, Color color) 
	{
		this->material = material;
		this->melting = melting;
		this->boiling = boiling;
		this->color = color;
	}
};


Material material_water = Material( water, 273, 373, Color(0.0f, 0.1f, 0.6f, 0.5f) );

struct Cell 
{
	Plant * owner;
	Material * material;
	uint temperature;
	Phases phase() 
	{
		if (this->temperature > this->material->melting) 
		{
			if (this->temperature > this->material->boiling) 
			{
				return gas;
			}
			else 
			{
				return liquid;
			}

		}
		else 
		{
			return solid;
		}
	}

	// Color color()
	// {
	// 	// return this->material-color;
	// }

	Cell(Plant * owner, Material * material, uint temperature)
	{
		this->material = material;
		this->owner = owner;
		this->temperature = temperature;


	}
};

struct Plant 
{
	uint energy;
	std::string genes;
	std::vector<Cell> cells;

	uint cursor_string;
	uint genomeSize;
	vec_u2 cursor_grid = vec_u2(0,0);
	vec_u2 prevCursor_grid = vec_u2(0,0);

	float accumulatedRotation;
	float accumulatedRotation_precomputedCos;
	float accumulatedRotation_precomputedSin;

	Plant(std::string genes) 
	{
		this->genes = genes;
		this->energy = 0;
	}

};


std::list <Plant> garden;

// the grid is a two dimensional vector, because it is of constant size, and must be accessed very fast.
// each point is a list of cells occupying that point in the physical space.
// the game is run by scanning across the grid and animating each list of cells in turn.
// this minimises costly neighbour-finding operations, and doesn't really have an impact on the speed of accessing the game model data.

std::vector< std::vector< std::list<Cell*> > > grid;






std::string exampleSentence = std::string("f = fractal, sequence with geometric scaling w = zigzag, length, angle, repetitions v = cone, first variable angle, second variable width, third variable length");



int drawCharacter (Plant * plant)
{
	char c = plant->genes[plant->cursor_string];
	std::list<vec_u2> v;

	switch (c)
	{

	case 'e': // extrude
	{
		plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}
	}

	case 'b': // branch. a sequence that grows at an angle to the main trunk
	{
		plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( plant->genes[plant->cursor_string] ); plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;} }
		float rotation = numberModifier;

		// record trunk rotation
		float trunkRotation = plant->accumulatedRotation;

		// figure out new branch rotation
		plant->accumulatedRotation = plant->accumulatedRotation + (rotation / 26.0f) * 2.0f * 3.14159f;
		plant->accumulatedRotation_precomputedSin = sin(plant->accumulatedRotation);
		plant->accumulatedRotation_precomputedCos = cos(plant->accumulatedRotation);

		// record old cursor position
		vec_u2 old_cursorGrid = plant->cursor_grid;

		// recursion_depth++;
		while (1)
		{
			if ( drawCharacter(plant) < 0)
			{
				break;
			}
		}
		// recursion_depth--;

		// return to normal orientation
		plant->accumulatedRotation = trunkRotation;
		plant->accumulatedRotation_precomputedSin = sin(plant->accumulatedRotation);
		plant->accumulatedRotation_precomputedCos = cos(plant->accumulatedRotation);
		plant->cursor_grid = old_cursorGrid;

		break;
	}

	case 's': // sequence. a motif is repeated serially a number of times.
	{
		plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}

		// get number of times to repeat the sequence
		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( plant->genes[plant->cursor_string] ); plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;} }
		int repeats = numberModifier;

		// the character after that is the next thing to be arrayed
		plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}
		unsigned int sequenceOrigin = plant->cursor_string;

		// recursion_depth ++;
		for ( int i = 0; i < repeats; ++i)
		{
			while (1)
			{
				if ( drawCharacter(plant) < 0)
				{
					break;
				}
			}
			plant->cursor_string = sequenceOrigin;
		}
		// recursion_depth--;
		break;
	}
	case ' ': // break innermost array
	{
		plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}
		return -1;
		break;
	}

	case 'c': // paint a circle at the cursor
	{
		plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( plant->genes[plant->cursor_string] ); plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;} }

		// get the circle radius (the next number in the string.)
		unsigned int radius = numberModifier;

		// define the range of pixels you'll draw- so you don't have to navigate the entire, massive grid.
		unsigned int drawingAreaLowerX = plant->cursor_grid.x - radius;
		unsigned int drawingAreaLowerY = plant->cursor_grid.y - radius;
		unsigned int drawingAreaUpperX = plant->cursor_grid.x + radius;
		unsigned int drawingAreaUpperY = plant->cursor_grid.y + radius;

		// raster a circle
		for (unsigned int i = drawingAreaLowerX; i < drawingAreaUpperX; ++i)
		{
			for (unsigned int j = drawingAreaLowerY; j < drawingAreaUpperY; ++j)
			{
				if (  magnitude_int (  i - plant->cursor_grid.x , j - plant->cursor_grid.y )  < radius )
				{
					v.push_back( vec_u2(i, j) );
				}
			}
		}

		break;
	}

	case 'l':
	{
		// rasters a line, taking into account the accumulated rotation
		plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}

		// set the previous x and y, which will be the start of the line
		plant->prevCursor_grid = plant->cursor_grid;

		bool chosenSignX = false;
		int signX = 1;
		while (!chosenSignX)
		{
			plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}
			signX = alphanumeric( plant->genes[plant->cursor_string]);
			if (signX > 13) { signX = 1; } else { signX = -1; }
			chosenSignX = true;
		}

		bool chosenSignY = false;
		int signY = 1;
		while (!chosenSignY)
		{
			plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}
			signY = alphanumeric( plant->genes[plant->cursor_string]);
			if (signY > 13) { signY = 1; } else { signY = -1; }
			chosenSignY = true;
		}

		unsigned int numberModifier = 0;
		while (!numberModifier)
		{
			plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}
			numberModifier = alphanumeric( plant->genes[plant->cursor_string] );
		}

		bool inRange = true;
		if (plant->cursor_grid.x + numberModifier  > sizeX && signX > 0)
		{
			plant->cursor_grid.x = sizeX - 1;
			inRange = false;
		}
		if (numberModifier > plant->cursor_grid.x && signX < 0 )
		{
			plant->cursor_grid.x = 0;
			inRange = false;
		}
		if (inRange)
		{
			if (signX > 0)
			{
				plant->cursor_grid.x += (numberModifier );
			}
			else
			{
				plant->cursor_grid.x -= (numberModifier );
			}
		}

		numberModifier = 0;
		while (!numberModifier)
		{
			plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}
			numberModifier = alphanumeric( plant->genes[plant->cursor_string] );
		}

		inRange = true;
		if (plant->cursor_grid.y + numberModifier  > sizeY && signY > 0)
		{
			plant->cursor_grid.y = sizeY - 1;
			inRange = false;
		}
		if (numberModifier > plant->cursor_grid.y && signY < 0 )
		{
			plant->cursor_grid.y = 0;
			inRange = false;
		}

		if (inRange)
		{
			if (signY > 0)
			{
				plant->cursor_grid.y += (numberModifier );
			}
			else
			{
				plant->cursor_grid.y -= (numberModifier );
			}

		}

		vec_f2 rotatedPoint = rotatePointPrecomputed( vec_f2(plant->prevCursor_grid.x, plant->prevCursor_grid.y),  plant->accumulatedRotation_precomputedSin,  plant->accumulatedRotation_precomputedCos, vec_f2(  plant->cursor_grid.x, plant->cursor_grid.y));

		if 		(rotatedPoint.x > 0 && rotatedPoint.x < sizeX - 1) 	{ plant->cursor_grid.x = rotatedPoint.x;}
		else if (rotatedPoint.x < 0) 								{ plant->cursor_grid.x = 0;}
		else if (rotatedPoint.x > sizeX - 1) 						{ plant->cursor_grid.x = sizeX - 1; }

		if 		(rotatedPoint.y > 0 && rotatedPoint.y < sizeY - 1) 	{ plant->cursor_grid.y = rotatedPoint.y;}
		else if (rotatedPoint.y < 0) 								{ plant->cursor_grid.y = 0;}
		else if (rotatedPoint.y > sizeY - 1) 						{ plant->cursor_grid.y = sizeY - 1; }

		v = EFLA_E( plant->prevCursor_grid, plant->cursor_grid);

		break;
	}

	default:
	{
		plant->cursor_string++; if (plant->cursor_string > plant->genomeSize) {return -1;}
	}

	}

	// paint in the points with material
	unsigned int n_points = 0;
	for (std::list<vec_u2>::iterator it = v.begin(); it != v.end(); ++it)
	{
		// Cell(Plant * owner, Material * material, uint temperature)
		Cell newcell = Cell(plant, &material_water, roomTemperature);
		plant->cells.push_back( newcell );

		( grid[it->x][it->y] ).push_back(
		 &(plant->cells.back() )
		  );

		n_points ++;
	}

	plant->energy -= n_points;

	return 0;
}




void deepgardenGraphics()
{
	preDraw();

	uint nVertsToRenderThisTurn = 4 * sizeX * sizeY;
	uint nIndicesToUseThisTurn 	= 5 * sizeX * sizeY;
	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

	// Create the buffer.
	uint g_vertex_buffer_cursor = 0;

	float * vertex_buffer_data  = new float[totalNumberOfFields];

	uint index_buffer_cursor = 0;
	uint index_buffer_content = 0;
	uint index_buffer_data[nIndicesToUseThisTurn];


	for (uint x = 0; x < sizeX; ++x)
	{
		for (uint y = 0; y < sizeY; ++y)
		{
			Color colorToUse = Color(0.0f, 0.0f, 0.0f, 1.0f);

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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint ) * nIndicesToUseThisTurn, index_buffer_data, GL_DYNAMIC_DRAW);

	glDrawElements( GL_TRIANGLE_STRIP, nIndicesToUseThisTurn, GL_UNSIGNED_INT, index_buffer_data );

	delete [] vertex_buffer_data;

	postDraw();
}

// insert a viable seed into the world
void instantiatePlant (std::string genes, uint x, uint y) 
{
	// instantiate a plant
	garden.push_back(Plant(genes));

	(garden.back()).cells.push_back(Cell(  &(garden.back()), &material_water, roomTemperature  ));

	grid[x][y].push_back( &( garden.back().cells.back() ) );
}



void initialize ()
{





}



void deepgardenLoop()
{

std::list<Cell*>::iterator cell;

for (int x = 0; x < sizeX; ++x)
{
	for (int y = 0; y < sizeY; ++y)
	{

		for (cell = (grid[x][y]).begin(); cell != (grid[x][y]).end(); ++cell)
		{
			
		}



	}
}


}
