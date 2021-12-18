#include "deepgarden.h"
#include "deepgarden_graphics.h"

uint numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

struct Material;
struct Cell;
struct Plant;
struct Color;


uint roomTemperature = 294;


Color black = Color(0.0f, 0.0f, 0.0f, 1.0f);

enum Phases
{
	solid,
	powder,
	liquid,
	gas,
	plasma,
	photonic
};

enum Materials
{
	water,
	light
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

// Color color_water = Color(0.0f, 0.2f, 1.0f, 1.0f);
Material material_water  = Material( water, 273, 373, Color(0.0f, 0.2f, 1.0f, 1.0f) );
Material material_light  = Material( light, 0, 0, Color(1.0f, 1.0f, 1.0f, 0.2f) );


struct Cell
{
	Plant * owner;
	Material * material;
	uint temperature;
	bool powdered;
	Phases phase()
	{

		if (this->material->boiling == 0) {return photonic;}
		else
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
				if (powdered)
				{
					return powder;

				}
				else
				{

					return solid;

				}

			}

		}


	}

	Color color()
	{
		return this->material->color;
		// return Color(1.0f, 1.0f, 1.0f, 1.0f);
	}

	Cell(Plant * owner, Material * material, uint temperature)
	{
		this->material = material;
		this->owner = owner;
		this->temperature = temperature;

		this->powdered = false;


	}
};

struct Plant
{
	int energy;
	std::string genes;
	std::list<Cell> cells;

	uint cursor_string;
	uint genomeSize;
	vec_u2 cursor_grid = vec_u2(0, 0);
	vec_u2 prevCursor_grid = vec_u2(0, 0);

	float accumulatedRotation;
	float accumulatedRotation_precomputedCos;
	float accumulatedRotation_precomputedSin;

	Plant(std::string genes)
	{
		this->genes = genes;
		this->genomeSize = genes.length();
		this->cursor_string = 0;
		this->energy = 0;
		this->accumulatedRotation = 0;
		this->accumulatedRotation_precomputedSin = sin(this->accumulatedRotation);
		this->accumulatedRotation_precomputedCos = cos(this->accumulatedRotation);
	}

};


std::list <Plant> garden;

// the grid is a two dimensional vector, because it is of constant size, and must be accessed very fast.
// each point is a list of cells occupying that point in the physical space.
// the game is run by scanning across the grid and animating each list of cells in turn.
// this minimises costly neighbour-finding operations, and doesn't really have an impact on the speed of accessing the game model data.

// static std::vector< std::vector< std::list<Cell*>>> grid = std::vector< std::vector< std::list<Cell*>>>();

//https://stackoverflow.com/questions/17663186/initializing-a-two-dimensional-stdvector

// ok instead let's make the grid one dimensional.

// std::vector<std::vector< std::list<Cell*> > > grid(
//     sizeX,
//     std::vector<std::list<Cell*>>(sizeY)); // Defaults to zero initial value


std::vector < std::list<Cell*> > grid ( (sizeX * sizeY));



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

		// printf("nonginging" );
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
	for (std::list<vec_u2>::iterator it = v.begin(); it != v.end(); it++)
	{
		// Cell(Plant * owner, Material * material, uint temperature)
		Cell newcell = Cell(plant, &material_water, roomTemperature);
		plant->cells.push_back( newcell );

		// printf("mingingingin");


		grid[ ((it->y * sizeX) + it->x) ] .push_back(
		    &(plant->cells.back() )
		);

		n_points ++;
	}

	plant->energy -= n_points;

	return 0;
}



uint drawCodon(Plant * plant)
{

	uint energyCost = 0;

	while (  true  )
	{

		int codonEnergy = drawCharacter(plant);

		if (codonEnergy == -1)
		{
			break;
		}
		else
		{
			energyCost += codonEnergy;
		}

	}

	return energyCost;

}



void deepgardenLoop()
{
	preDraw();


	uint totalSize = sizeX * sizeY;
	uint nVertsToRenderThisTurn = 4 * totalSize;
	uint nIndicesToUseThisTurn 	= 5 * totalSize;

	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

	// Create the buffer.
	uint g_vertex_buffer_cursor = 0;

	float * vertex_buffer_data  = new float[totalNumberOfFields];

	uint index_buffer_cursor = 0;
	uint index_buffer_content = 0;
	uint index_buffer_data[nIndicesToUseThisTurn];





	std::list<Plant>::iterator plant;

	std::list<Cell>::iterator cell;
	std::list<Cell*>::iterator p_cell;


	Plant gaia = Plant("");


	// // shine the lamp
	for (uint i = 0; i < sizeX; ++i)
	{


		grid[ ((sizeY - 1 )* sizeX) + i  ] .push_back(
		    new Cell (&gaia, &material_light, roomTemperature)
		);

	}




	for (plant = garden.begin(); plant != garden.end(); plant++)
	{
		if (plant->energy >= 0)
		{

			plant->energy -= drawCodon( &(*plant) );
			printf("plant enene %i\n", plant->energy);

		}
	}




	for (uint i = 0; i < totalSize ; i++)
	{

		uint x = i % sizeX;
		uint y = i / sizeX;

		uint startOfThisRow = y * sizeX; // because y is an integer, this will neatly round it to the start of the row.
		uint startOfNextRow = startOfThisRow + 1;

		uint oneRowUp = i + sizeY;
		uint oneRowDown = i - sizeY;











		Color colorToUse = black;


		for (p_cell = grid[i].begin(); p_cell != grid[i].end(); )
		{


			if (*p_cell != nullptr) {colorToUse = (*p_cell)->color();  //Color(0.0f, 0.0f, 0.0f, 1.0f);
}
			

			// get material phase

			Phases cellPhase = powder;//(*p_cell)->phase();
			bool done = false;




			switch (cellPhase)
			{
			case solid:
			{
				break;
			}
			case powder:
			{

				uint neighbours[] = 
				{
					( oneRowDown  + 1 ),
					( oneRowDown     ),
					( oneRowDown  - 1 )
				};

				for (uint i = 0; i < 3; i++)
				{

					if (neighbours[i] > totalSize) {continue;}
					uint neighbourOccupied = grid[neighbours[i]].size();


					if ( !neighbourOccupied)
					{
						grid[neighbours[i]].push_back( (*p_cell) ); //grid[i].remove( *p_cell );
						*p_cell = nullptr;

						done = true;
						break;
					}
				}

				break;
			}
			case liquid:
			{
				uint neighbours[] = {

					( i + 1 ),
					( i - 1 ),
					( oneRowDown  + 1 ),
					( oneRowDown     ),
					( oneRowDown  - 1 )


				};

				for (uint i = 0; i < 5; i++)
				{

					if (neighbours[i] > totalSize) {continue;}
					uint neighbourOccupied = grid[neighbours[i]].size();

					if ( !neighbourOccupied)
					{
						// Cell * temp = grid[neighbour]; grid[neighbour] = grid[i]; grid[i] = temp; // swap two cells
						grid[neighbours[i]].push_back( (*p_cell) ); //grid[i].remove( *p_cell);
						*p_cell = nullptr;
						done = true;
						break;
					}
				}

				break;
			}
			case gas:
			{
				uint neighbours[] = {

					( i + 1 ),
					( i - 1 ),
					( oneRowDown  + 1 ),
					( oneRowDown     ),
					( oneRowDown  - 1 ),
					( oneRowUp    + 1 ),
					( oneRowUp       ),
					( oneRowUp    - 1 ),


				};

				for (uint i = 0; i < 8; i++)
				{

					if (neighbours[i] > totalSize) {continue;}
					uint neighbourOccupied = grid[neighbours[i]].size();

					if ( !neighbourOccupied)
					{
						if (RNG() < 0.1)
						{
							// Cell * temp = grid[neighbour]; grid[neighbour] = grid[i]; grid[i] = temp; // swap two cells
							grid[neighbours[i]].push_back( (*p_cell) );// grid[i].remove( *p_cell);
							*p_cell = nullptr;
							done = true;
							break;
						}
					}
				}
				break;
			}
			case photonic:
			{
				// Cell * temp = grid[oneRowDown]; grid[oneRowDown] = grid[i]; grid[i] = temp; // swap two cells
				grid[oneRowDown].push_back( (*p_cell) ); //grid[i].remove(* p_cell);
				*p_cell = nullptr;
				done = true;
				break;
			}
			}
			if (done) {break;}
			// }

			p_cell++;

			// if (*p_cell == nullptr) 
			// {
				grid[i].remove(nullptr);

			// }





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

	grid[(y * sizeX) + x].push_back( &( garden.back().cells.back() ) );

	garden.back().cursor_grid = vec_u2(x, y);
}



void initialize ()
{

	// uint totalSize = sizeX * sizeY;
	// for (uint i = 0; i < totalSize; i++)
	// {

	// // 	// for (int y = 0; y < sizeY; y++)
	// // 	// {

	// // 	// 	// std::list<Cell*> newListOfBlarghs = std::list<Cell*>;
	// 		grid[i] = *( new std::list<Cell*> );
	// // 	// }

	// }


	instantiatePlant(exampleSentence, 100, 100);



}



