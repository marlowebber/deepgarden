#include "deepgarden.h"
#include "deepgarden_graphics.h"

#include <ctime>
#include <chrono>
#include <iostream>

//#define THREAD_TIMING
#define RENDERING_THREADS 4

const unsigned int totalSize = sizeX * sizeY;
const unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

float colorGrid[ totalSize * numberOfFieldsPerVertex ];
float lifeColorGrid[totalSize * numberOfFieldsPerVertex ];
float seedColorGrid[totalSize * numberOfFieldsPerVertex];


struct Particle
{
	uint8_t material;
	uint8_t phase;
	uint16_t temperature;
};

struct LifeParticle
{
	std::string genes;
	uint16_t identity;
	float energy;
};

struct SeedParticle
{
	uint16_t parentIdentity;
	std::string genes;
	float energy;
	uint8_t stage;
};

Particle grid[totalSize];
LifeParticle lifeGrid[totalSize];
SeedParticle seedGrid[totalSize];

std::string exampleSentence = "su lmmmm sd lmmmmblmmmmff ";

void mutateSentence ( std::string * genes )
{
	size_t genesLength = genes->length();

	for (int i = 0; i < genesLength; ++i)
	{
		// swap a letter
		if (extremelyFastNumberFromZeroTo(100) == 0)
		{
			// https://stackoverflow.com/questions/20132650/how-to-select-random-letters-in-c
			(*genes)[i] = (char)('a' + rand() % 26);
		}

		// add a letter
		if (extremelyFastNumberFromZeroTo(100) == 0)
		{
			char randomCharacter = (char)('a' + rand() % 26);
			genes->insert(    extremelyFastNumberFromZeroTo( (genes->length() - 1) )  ,  &randomCharacter  );
		}

		// delete a letter
		if (extremelyFastNumberFromZeroTo(100) == 0)
		{
			genes->erase (  extremelyFastNumberFromZeroTo( (genes->length() - 1) )  , 1);
		}
	}
}

const Color color_lightblue = Color( 0.1f, 0.3f, 0.65f, 1.0f );
const Color color_yellow    = Color( 1.0f, 1.0f, 0.0f, 1.0f );
const Color color_lightgrey = Color( 0.75f, 0.75f, 0.75f, 1.0f );
const Color color_grey      = Color( 0.50f, 0.50f, 0.50f, 1.0f );
const Color color_darkgrey  = Color( 0.25f, 0.25f, 0.25f, 1.0f );
const Color color_black     = Color( 0.0f, 0.0f, 0.0f, 1.0f );
const Color color_white     = Color( 1.0f, 1.0f, 1.0f, 1.0f );
const Color color_purple     = Color( 0.8f, 0.0f, 0.8f, 1.0f );

int wind = 0;

int defaultTemperature = 300;

unsigned int recursion_level = 0;
const unsigned int recursion_limit = 3;

unsigned int drawActions = 0;
const unsigned int drawActionlimit = 100;

// variables keep track of the sequence and rotation states.
float accumulatedRotation = 0.0f;
float accumulatedRotation_precomputedSin = sin(accumulatedRotation);
float accumulatedRotation_precomputedCos = cos(accumulatedRotation);

vec_u2 cursor_grid = vec_u2(0, 0);
vec_u2 prevCursor_grid = vec_u2(0, 0);
unsigned int cursor_string = 0;
vec_u2 origin = vec_u2(0, 0);
Color cursor_color = Color(0.1f, 0.1f, 0.1f, 1.0f);

float maximumDisplayEnergy = 1;

std::list<unsigned int> identities;

Color materialColor ( unsigned int material )
{
	if ((material == MATERIAL_VACUUM))      {    return color_black;      }
	if ((material == MATERIAL_WATER))       {    return color_lightblue;  }
	if ((material == MATERIAL_QUARTZ))      {    return color_lightgrey;  }
	if ((material == MATERIAL_AMPHIBOLE))   {    return color_grey;       }
	if ((material == MATERIAL_OLIVINE))     {    return color_darkgrey;   }
	if ((material == MATERIAL_GOLD))        {    return color_yellow;     }
	if ((material == MATERIAL_SEED))        {    return color_yellow;     }
	return color_black;
}

void setParticle(unsigned int material, unsigned int i)
{
	grid[i].temperature = defaultTemperature;
	grid[i].material = material;
	grid[i].phase = PHASE_POWDER;
	Color temp_color = materialColor(material);
	unsigned int a_offset = (i * numberOfFieldsPerVertex);
	memcpy( &colorGrid[ a_offset ], &temp_color, 16 );
}

void swapParticle (unsigned int a, unsigned int b)
{
	float temp_color[4];
	unsigned int a_offset = (a * numberOfFieldsPerVertex);
	unsigned int b_offset = (b * numberOfFieldsPerVertex);
	memcpy( temp_color, &colorGrid[ b_offset ] , 16 ); // 4x floats of 4 bytes each
	memcpy( &colorGrid[ b_offset], &colorGrid[ a_offset] , 16 );
	memcpy( &colorGrid[ a_offset ], temp_color, 16 );
	Particle tempParticle = grid[b];
	grid[b] = grid[a];
	grid[a] = tempParticle;
}

void setLifeParticle( std::string genes, unsigned int identity, unsigned int i)
{
	lifeGrid[i].identity = identity;
	lifeGrid[i].genes  = genes;
	lifeGrid[i].energy = 0.0f;

	memcpy( (&lifeColorGrid[i * numberOfFieldsPerVertex]),  &(cursor_color),  sizeof(Color) );
}

void swapLifeParticle(unsigned int a, unsigned int b)
{
	LifeParticle tempLife;
	memcpy( &tempLife,  &(lifeGrid[a]), 	 sizeof(LifeParticle) );
	memcpy( &(lifeGrid[a]),  &(lifeGrid[b]), sizeof(LifeParticle) );
	memcpy( &(lifeGrid[b]),  &tempLife, 	 sizeof(LifeParticle) );

	float temp_color[4];
	unsigned int a_offset = (a * numberOfFieldsPerVertex)  ;
	unsigned int b_offset = (b * numberOfFieldsPerVertex)  ;
	memcpy( temp_color, &lifeColorGrid[ b_offset ] , sizeof(Color) ); // 4x floats of 4 bytes each
	memcpy( &lifeColorGrid[ b_offset], &lifeColorGrid[ a_offset] , sizeof(Color) );
	memcpy( &lifeColorGrid[ a_offset ], temp_color, sizeof(Color) );
}

void setSeedParticle( std::string genes, unsigned int parentIdentity, float energyDebt, unsigned int i)
{
	seedGrid[i].genes = genes;
	seedGrid[i].parentIdentity = parentIdentity;
	seedGrid[i].stage = STAGE_BUD;
	seedGrid[i].energy = energyDebt;
	memcpy( (&seedColorGrid[i * numberOfFieldsPerVertex]) ,  &(color_yellow),  sizeof(Color) );
}

void setPhoton(  unsigned int i)
{
	seedGrid[i].stage = STAGE_PHOTON;
	memcpy( (&seedColorGrid[i * numberOfFieldsPerVertex]) ,  &(color_white),  sizeof(Color) );
}

void clearSeedParticle( unsigned int i)
{
	seedGrid[i].stage = 0x00;
	memset( &(seedColorGrid[ i * numberOfFieldsPerVertex ]) , 0x00, 16 );
}

void swapSeedParticle(unsigned int a, unsigned int b)
{
	SeedParticle tempSeed = seedGrid[b];
	seedGrid[b] = seedGrid[a];
	seedGrid[a] = tempSeed;
	float temp_color[4];
	unsigned int a_offset = (a * numberOfFieldsPerVertex) ;
	unsigned int b_offset = (b * numberOfFieldsPerVertex) ;
	memcpy( temp_color, 				&seedColorGrid[ b_offset ] , 	sizeof(Color) ); // 4x floats of 4 bytes each
	memcpy( &seedColorGrid[ b_offset], 	&seedColorGrid[ a_offset] , 	sizeof(Color) );
	memcpy( &seedColorGrid[ a_offset ], temp_color, 					sizeof(Color) );
}

unsigned int newIdentity ()
{
	unsigned int identityCursor = 1; // zero is reserved
	while (true)
	{
		bool used = false;
		for (std::list<unsigned int>::iterator it = identities.begin(); it != identities.end(); ++it)
		{
			if (identityCursor == *it)
			{
				used = true;
			}
		}

		if (!used)
		{
			identities.push_back(identityCursor);
			printf("claimed new ID: %u\n", identityCursor);
			return identityCursor;
		}
		identityCursor++;
	}
	return identityCursor;
}

void retireIdentity (unsigned int identityToRetire)
{
	identities.remove (identityToRetire);
}

void initialize ()
{
	setupExtremelyFastNumberGenerators();
	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	srand((unsigned int)time(NULL));

	memset ( colorGrid, 	0x00, sizeof(float ) * numberOfFieldsPerVertex * totalSize );
	memset ( lifeColorGrid, 0x00, sizeof(float ) * numberOfFieldsPerVertex * totalSize );
	memset ( seedColorGrid, 0x00, sizeof(float ) * numberOfFieldsPerVertex * totalSize );

	memset(grid, 0x00, sizeof(Particle) * ( totalSize));

	// // setup the x and y positions in the color grid. these never change so you can just calculate them once.
	unsigned int x = 0;
	unsigned int y = 0;
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		x = i % sizeX;
		if (!x) { y = i / sizeX; }
		float fx = x;
		float fy = y;

		grid[i].temperature = defaultTemperature;
		grid[i].phase = PHASE_VACUUM;

		// RGBA color occupies the first 4 places.
		// also, initialize the color alpha to 1.
		colorGrid[ (i * numberOfFieldsPerVertex) + 3] = 1.0f;
		colorGrid[ (i * numberOfFieldsPerVertex) + 4] = fx;
		colorGrid[ (i * numberOfFieldsPerVertex) + 5] = fy;

		lifeColorGrid[ (i * numberOfFieldsPerVertex) + 3] = 0.0f;
		lifeColorGrid[ (i * numberOfFieldsPerVertex) + 4] = fx;
		lifeColorGrid[ (i * numberOfFieldsPerVertex) + 5] = fy;

		seedColorGrid[ (i * numberOfFieldsPerVertex) + 3] = 0.0f;
		seedColorGrid[ (i * numberOfFieldsPerVertex) + 4] = fx;
		seedColorGrid[ (i * numberOfFieldsPerVertex) + 5] = fy;

		if (false)
		{
			// olivine cube

			if (x > 100 && x < 200)
			{
				if (y > 100 && y < 200)
				{
					setParticle( MATERIAL_OLIVINE, i);
				}
			}
		}

		if (true)
		{
			// sprinkle some material on it to make a default scene.
			if (i > (10 * sizeX) && i < (50 * sizeX))
			{
				if (RNG() < 0.5)
				{
					setParticle( MATERIAL_OLIVINE, i);
				}
			}

			// sprinkle some material on it to make a default scene.
			if (i > (50 * sizeX) && i < (75 * sizeX))
			{
				if (RNG() < 0.5)
				{
					setParticle( MATERIAL_AMPHIBOLE, i);
				}
			}

			// sprinkle some material on it to make a default scene.
			if (i > (75 * sizeX) && i < (100 * sizeX))
			{
				if (RNG() < 0.5)
				{
					setParticle( MATERIAL_QUARTZ, i);
				}
			}

			// sprinkle some material on it to make a default scene.
			if (i > (100 * sizeX) && i < (125 * sizeX))
			{
				if (RNG() < 0.5)
				{
					setParticle( MATERIAL_WATER, i);
				}
			}
		}

		if (x == 500 && y == 100)
		{

			setSeedParticle(exampleSentence, newIdentity() , 0, i);
			seedGrid[i].stage = STAGE_FRUIT;
		}
	}
}

void heatEverything ()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		grid[i].temperature += 25;
	}
}

void coolEverything ()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		grid[i].temperature -= 25;
	}
}

void setNeutralTemp ()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		grid[i].temperature = 300;
	}
}

void thread_temperature2 ()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	for (unsigned int i = (sizeX + 1); i < (totalSize - (sizeX + 1)); ++i)
	{
		// MATERIAL_WATER
		if (grid[i].material == MATERIAL_WATER)
		{
			if (grid[i].phase == PHASE_POWDER)
			{
				if (grid[i].temperature > 273)
				{
					grid[i].phase = PHASE_LIQUID;
				}
			}
			else if (grid[i].phase == PHASE_LIQUID)
			{
				if (grid[i].temperature < 273)
				{
					grid[i].phase = PHASE_POWDER;
				}
				else if (grid[i].temperature > 373)
				{
					grid[i].phase = PHASE_GAS;
				}
			}
			else if (grid[i].phase == PHASE_GAS)
			{
				if (grid[i].temperature < 373)
				{
					grid[i].phase = PHASE_LIQUID;
				}
			}
		}

		// MATERIAL_QUARTZ
		else if (grid[i].material == MATERIAL_QUARTZ)
		{
			if (grid[i].phase == PHASE_POWDER)
			{
				if (grid[i].temperature > 600)
				{
					grid[i].phase = PHASE_LIQUID;
				}
			}
			else if (grid[i].phase == PHASE_LIQUID)
			{
				if (grid[i].temperature < 600)
				{
					grid[i].phase = PHASE_POWDER;
				}
				else if (grid[i].temperature > 600 && grid[i].temperature < 700)
				{

					unsigned int squareBelow = i - sizeX;
					unsigned int squareAbove = i + sizeX;
					unsigned int neighbours[] =
					{
						//squareBelow - 1,
						squareBelow,
						//	squareBelow + 1,
						i - 1,
						i + 1,
						//	squareAbove - 1,
						squareAbove,
						//	squareAbove + 1
					};
					unsigned int nSolidNeighbours = 0;
					for (int i = 0; i < 4; ++i)
					{
						if ( grid[neighbours[i]].material == MATERIAL_QUARTZ )
						{
							if (grid[neighbours[i]].phase == PHASE_SOLID )
							{
								nSolidNeighbours++;
							}
						}
					}
					if (nSolidNeighbours > 2)
					{
						grid[i].phase = PHASE_SOLID;
					}
					else if (extremelyFastNumberFromZeroTo(1000) == 0)
					{
						grid[i].phase = PHASE_SOLID;
					}
				}
				else if (grid[i].temperature > 2330)
				{
					grid[i].phase = PHASE_GAS;
				}
			}
			else if (grid[i].phase == PHASE_GAS)
			{
				if (grid[i].temperature < 2330)
				{
					grid[i].phase = PHASE_LIQUID;
				}
			}
			else if (grid[i].phase == PHASE_SOLID )
			{
				if (grid[i].temperature > 2330)
				{
					unsigned int squareBelow = i - sizeX;
					unsigned int squareAbove = i + sizeX;
					unsigned int neighbours[] =
					{
						squareBelow - 1,
						squareBelow,
						squareBelow + 1,
						i - 1,
						i + 1,
						squareAbove - 1,
						squareAbove,
						squareAbove + 1
					};
					unsigned int nGaseousNeighbours = 0;
					for (int i = 0; i < 8; ++i)
					{
						if ( grid[neighbours[i]].material == MATERIAL_QUARTZ )
						{
							if (grid[neighbours[i]].phase == PHASE_GAS )
							{
								nGaseousNeighbours++;
							}
						}
					}
					if (nGaseousNeighbours > 3)
					{
						grid[i].phase = PHASE_GAS;
					}
					else if (extremelyFastNumberFromZeroTo(1000) == 0)
					{
						grid[i].phase = PHASE_GAS;
					}
				}

			}

		}





		else if (grid[i].material == MATERIAL_AMPHIBOLE)
		{
			if (grid[i].phase == PHASE_POWDER)
			{
				if (grid[i].temperature > 800)
				{
					grid[i].phase = PHASE_LIQUID;
				}
			}
			else if (grid[i].phase == PHASE_LIQUID)
			{
				if (grid[i].temperature < 800)
				{
					grid[i].phase = PHASE_POWDER;
				}
				else if (grid[i].temperature > 800 && grid[i].temperature < 900)
				{
					unsigned int squareBelow = i - sizeX;
					unsigned int squareAbove = i + sizeX;
					unsigned int neighbours[] =
					{
						squareBelow - 1,
						//squareBelow,
						squareBelow + 1,
						//i - 1,
						//i + 1,
						squareAbove - 1,
						//squareAbove,
						squareAbove + 1
					};
					unsigned int nSolidNeighbours = 0;
					for (int i = 0; i < 4; ++i)
					{
						if ( grid[neighbours[i]].material == MATERIAL_AMPHIBOLE )
						{
							if (grid[neighbours[i]].phase == PHASE_SOLID )
							{
								nSolidNeighbours++;
							}
						}
					}
					if (nSolidNeighbours > 2)
					{
						grid[i].phase = PHASE_SOLID;
					}
					else if (extremelyFastNumberFromZeroTo(1000) == 0)
					{
						grid[i].phase = PHASE_SOLID;
					}

				}
				else if (grid[i].temperature > 2330)
				{
					grid[i].phase = PHASE_GAS;
				}
			}

			else if (grid[i].phase == PHASE_GAS)
			{
				if (grid[i].temperature < 2330)
				{
					grid[i].phase = PHASE_LIQUID;
				}
			}
		}


		else if (grid[i].material == MATERIAL_OLIVINE)
		{
			if (grid[i].phase == PHASE_POWDER)
			{
				if (grid[i].temperature > 1000)
				{
					grid[i].phase = PHASE_LIQUID;
				}
			}
			else if (grid[i].phase == PHASE_LIQUID)
			{
				if (grid[i].temperature < 1000)
				{
					grid[i].phase = PHASE_POWDER;
				}
				else if (grid[i].temperature > 1000 && grid[i].temperature < 1100)
				{

					unsigned int squareBelow = i - sizeX;
					unsigned int squareAbove = i + sizeX;
					unsigned int neighbours[] =
					{
						//	squareBelow - 1,
						squareBelow,
						//	squareBelow + 1,
						//i - 1,
						//	i + 1,
						//	squareAbove - 1,
						squareAbove,
						//	squareAbove + 1
					};
					unsigned int nSolidNeighbours = 0;
					for (int i = 0; i < 2; ++i)
					{
						if ( grid[neighbours[i]].material == MATERIAL_OLIVINE )
						{
							if (grid[neighbours[i]].phase == PHASE_SOLID )
							{
								nSolidNeighbours++;
							}
						}
					}
					if (nSolidNeighbours > 1)
					{
						grid[i].phase = PHASE_SOLID;
					}
					else if (extremelyFastNumberFromZeroTo(1000) == 0)
					{
						grid[i].phase = PHASE_SOLID;
					}

				}
				else if (grid[i].temperature > 2330)
				{
					grid[i].phase = PHASE_GAS;
				}
			}
			else if (grid[i].phase == PHASE_GAS)
			{
				if (grid[i].temperature < 2330)
				{
					grid[i].phase = PHASE_LIQUID;
				}
			}
		}





	}

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_temperature " << elapsed.count() << " microseconds." << std::endl;
#endif
}

// return a random integer in the range. It is inclusive of both end values.
unsigned int randomIntegerInRange (unsigned int from, unsigned int to)
{
	return from + ( std::rand() % ( to - from + 1 ) );
}

void physics_sector (unsigned int from, unsigned int to)
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	for (unsigned int i = from; i < to; ++i)
	{
		unsigned int squareBelow = i - sizeX;

		if (grid[i].phase  == PHASE_POWDER)
		{
			unsigned int neighbours[] =
			{
				squareBelow - 1,
				squareBelow,
				squareBelow + 1
			};

			for (uint8_t k = 0; k < 1; ++k) // instead of checking all of them every turn, you can check half or less and it doesn't make a difference for gameplay.
			{
				uint8_t index = extremelyFastNumberFromZeroTo(2);
				// chemistry(i, neighbours[index]);
				if ((grid[neighbours[index]].phase == PHASE_VACUUM) ||  (grid[neighbours[index]].phase == PHASE_GAS) ||  (grid[neighbours[index]].phase == PHASE_LIQUID)  )
				{
					swapParticle(i, neighbours[index]);
					break;
				}
			}
		}

		// movement instructions for LIQUIDS
		else if (grid[i].phase == PHASE_LIQUID)
		{
			unsigned int neighbours[] =
			{
				squareBelow - 1,
				squareBelow,
				squareBelow + 1,
				i - 1,
				i + 1
			};

			for (uint8_t k = 0; k < 2; ++k)
			{
				uint8_t index = extremelyFastNumberFromZeroTo(4);
				// chemistry(i, neighbours[index]) ;
				if ((grid[neighbours[index]].phase == PHASE_VACUUM) || (grid[neighbours[index]].phase == PHASE_GAS) ||  (grid[neighbours[index]].phase == PHASE_LIQUID)     )
				{
					swapParticle(i, neighbours[index]);

					if (false)
					{
						unsigned int erosionNeighbours[] =
						{
							//squareBelow - 1,
							squareBelow,
							//squareBelow + 1,
							i - 1,
							i + 1
						};

						for (uint8_t k = 0; k < 3; ++k)
						{
							if ( grid[erosionNeighbours[k]].phase == PHASE_SOLID )
							{
								if (extremelyFastNumberFromZeroTo(10) == 0 )
								{
									grid[erosionNeighbours[k]].phase = PHASE_POWDER;
								}
							}
						}
					}
					break;
				}
			}
		}

		// movement instructions for GASES
		else if (grid[i].phase == PHASE_GAS)
		{
			unsigned int squareAbove = i + sizeX;
			unsigned int neighbours[] =
			{
				squareBelow - 1,
				squareBelow,
				squareBelow + 1,
				i - 1,
				i + 1,
				squareAbove - 1,
				squareAbove,
				squareAbove + 1
			};

			for (uint8_t j = 0; j < 3; ++j)
			{
				uint8_t index = extremelyFastNumberFromZeroTo(7);

				if (grid[i].material == MATERIAL_WATER)
				{
					if (grid[neighbours[index]].material != MATERIAL_VACUUM)
					{
						if (grid[i].temperature < 373 )
						{
							grid[i].phase = PHASE_LIQUID;
						}
					}
				}
				if (grid[neighbours[index]].material == MATERIAL_WATER)
				{
					if (grid[i].material != MATERIAL_VACUUM)
					{
						if (grid[neighbours[index]].temperature < 373 )
						{
							grid[neighbours[index]].phase = PHASE_LIQUID;
						}
					}
				}


				if (grid[neighbours[index]].phase  == PHASE_VACUUM || (grid[neighbours[index]].phase == PHASE_GAS) )
				{
					swapParticle(i, neighbours[index]);
					break;
				}
			}
		}
	}
#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_physics_sector " << elapsed.count() << " microseconds." << std::endl;
#endif
}

void thread_physics ()
{
	uint16_t windChange = extremelyFastNumberFromZeroTo(100);

	if (!windChange)
	{
		uint16_t windRand =  extremelyFastNumberFromZeroTo( 2);
		wind = windRand - 1;
	}

	// sprinkle rain
	if (false)
	{
		for (int i = (sizeY - 2) * sizeX; i < (sizeY - 1)*sizeX; ++i)
		{
			if (extremelyFastNumberFromZeroTo(10) == 0)
			{
				grid[i].material = MATERIAL_WATER;
				grid[i].phase = PHASE_LIQUID;
				memcpy(&colorGrid[i * numberOfFieldsPerVertex], &color_lightblue, 16  );
				grid[i].temperature = defaultTemperature;
			}
		}
	}

	// sprinkle photons
	if (true)
	{
		for (int i = (sizeY - 2) * sizeX; i < (sizeY - 1)*sizeX; ++i)
		{
			if (extremelyFastNumberFromZeroTo(100) == 0)
			{
				setPhoton(   i);
			}
		}
	}

	unsigned int quad1 = totalSize / 4;
	unsigned int quad2 = totalSize / 2;
	unsigned int quad3 = (totalSize / 4) * 3;
	boost::thread t11{ physics_sector, (sizeX + 1), quad1 };
	boost::thread t12{ physics_sector, (quad1 + 1), quad2 };
	boost::thread t13{ physics_sector, (quad2 + 1), quad3 };
	boost::thread t14{ physics_sector, (quad3 + 1), (totalSize - sizeX - 1) };
	t11.join();
	t12.join();
	t13.join();
	t14.join();
}

void setPointSize (unsigned int pointSize)
{
	glPointSize(pointSize);
}

void thread_graphics()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif
	preDraw();

	bool showEnergyGrid = false;
	if (showEnergyGrid)
	{



		unsigned int nVertsToRenderThisTurn = 1 * totalSize;
		long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

		float * energyColorGrid = new float[totalNumberOfFields];

		unsigned int x = 0;
		unsigned int y = 0;
		for (unsigned int i = 0; i < totalSize; ++i)
		{

			// float energySqrt = sqrt(ifeGrid[i].energy);
			// if (energySqrt > maximumDisplayEnergy)
			// {
			// 	maximumDisplayEnergy = energySqrt;
			// }
			// if (maximumDisplayEnergy < 0.1f) 
			// {
			// 	maximumDisplayEnergy = 0.1f;
			// }

			x = i % sizeX;
			if (!x) { y = i / sizeX; }
			float fx = x;
			float fy = y;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = lifeGrid[i].energy / maximumDisplayEnergy;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = lifeGrid[i].energy / maximumDisplayEnergy;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = lifeGrid[i].energy / maximumDisplayEnergy;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 3 ] = 1.0f;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 4 ] = fx;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 5 ] = fy;

			
		}


		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, energyColorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);


		postDraw();


		delete [] energyColorGrid;


	}

	else
	{

		unsigned int nVertsToRenderThisTurn = 1 * totalSize;
		long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, colorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, lifeColorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, seedColorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

		postDraw();


	}





#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_graphics " << elapsed.count() << " microseconds." << std::endl;
#endif
}

// THE EXTREMELY FAST LINE ALGORITHM Variation E (Addition Fixed Point PreCalc Small Display)
// Small Display (256x256) resolution.
std::list<vec_u2> EFLA_E(vec_u2 start, vec_u2 end)
{
	int x  = start.x;
	int y  = start.y;
	int x2 = end.x;
	int y2 = end.y;

	std::list<vec_u2> v;

	bool yLonger = false;
	int shortLen = y2 - y;
	int longLen = x2 - x;
	if (abs(shortLen) > abs(longLen))
	{
		int swap = shortLen;
		shortLen = longLen;
		longLen = swap;
		yLonger = true;
	}
	int decInc;
	if (longLen == 0) decInc = 0;
	else decInc = (shortLen << 8) / longLen;

	if (yLonger) {
		if (longLen > 0) {
			longLen += y;
			for (int j = 0x80 + (x << 8); y <= longLen; ++y) {
				v.push_back( vec_u2(j >> 8, y) );
				j += decInc;
			}
			return v;
		}
		longLen += y;
		for (int j = 0x80 + (x << 8); y >= longLen; --y) {
			v.push_back( vec_u2(j >> 8, y) );
			j -= decInc;
		}
		return v;
	}

	if (longLen > 0) {
		longLen += x;
		for (int j = 0x80 + (y << 8); x <= longLen; ++x) {
			v.push_back( vec_u2(x, j >> 8) );
			j += decInc;
		}
		return v;
	}
	longLen += x;
	for (int j = 0x80 + (y << 8); x >= longLen; --x) {
		v.push_back( vec_u2(x, j >> 8) );
		j -= decInc;
	}
	return v;
}

int drawCharacter ( std::string genes , unsigned int identity, int energyDebtSoFar)
{
	char c = genes[cursor_string];

	printf("char %c ", c);
	drawActions++;
	// printf("cursor_grid %u %u\n", cursor_grid.x, cursor_grid.y);
	if (recursion_level > recursion_limit)
	{
		printf("recursion_level exceeded\n");
		cursor_string++;
		return -1;
	}

	if (drawActions > drawActionlimit)
	{
		printf("drawActions exceeded\n");
		cursor_string++;
		return -1;
	}

	
	std::list<vec_u2> v;

	std::list<vec_u2> v_seeds;

	unsigned int genesize = genes.length();

	switch (c)
	{

	case 't': // branch. a sequence that grows at an angle to the main trunk
	{
		printf("branch\n");
		cursor_string++; if (cursor_string > genesize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		float rotation = numberModifier;

		// record trunk rotation
		float trunkRotation = accumulatedRotation;

		// figure out new branch rotation
		accumulatedRotation = accumulatedRotation + (rotation / 26.0f) * 2.0f * 3.14159f;
		accumulatedRotation_precomputedSin = sin(accumulatedRotation);
		accumulatedRotation_precomputedCos = cos(accumulatedRotation);

		// record old cursor position
		vec_u2 old_cursorGrid = cursor_grid;

		recursion_level++;
		while (1)
		{
			if ( drawCharacter(genes, identity, energyDebtSoFar) < 0)
			{
				break;
			}
		}
		recursion_level--;

		// return to normal rotation
		accumulatedRotation = trunkRotation;
		accumulatedRotation_precomputedSin = sin(accumulatedRotation);
		accumulatedRotation_precomputedCos = cos(accumulatedRotation);

		// return to normal position
		cursor_grid = old_cursorGrid;



		break;
	}

	case 's': // sequence. a motif is repeated serially a number of times.
	{
		printf("sequence\n");
		cursor_string++; if (cursor_string > genesize) {return -1;}

		// get number of times to repeat the sequence
		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		int repeats = numberModifier;

		// the character after that is the next thing to be arrayed
		cursor_string++; if (cursor_string > genesize) {return -1;}

		unsigned int sequenceOrigin = cursor_string;

		recursion_level++;
		for ( int i = 0; i < repeats; ++i)
		{
			while (1)
			{
				if ( drawCharacter(genes, identity, energyDebtSoFar) < 0)
				{
					break;
				}
			}
			cursor_string = sequenceOrigin;
		}
		recursion_level--;


		break;

	}
	case ' ': // break innermost array
	{
		printf("break\n");
		cursor_string++; if (cursor_string > genesize) {return -1;}


		return -1;
		break;
	}

	case '.': // break array and return cursor to origin

		printf("break and return cursor\n");
		cursor_string++; if (cursor_string > genesize) {return -1;}

		cursor_grid = origin;


		return -1;
		break;

	case 'f': // make a seed
	{

		printf("draw a seed\n");
		cursor_string++; if (cursor_string > genesize) {return -1;}

		v_seeds.push_back( cursor_grid );


		break;
	}

	case 'c': // paint a circle at the cursor
	{

		printf("draw a circle\n");
		cursor_string++; if (cursor_string > genesize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		// get the circle radius (the next number in the string.)
		unsigned int radius = numberModifier;

		// define the range of pixels you'll draw- so you don't have to navigate the entire, massive grid.
		unsigned int drawingAreaLowerX = cursor_grid.x - radius;
		unsigned int drawingAreaLowerY = cursor_grid.y - radius;
		unsigned int drawingAreaUpperX = cursor_grid.x + radius;
		unsigned int drawingAreaUpperY = cursor_grid.y + radius;

		// raster a circle
		for (unsigned int i = drawingAreaLowerX; i < drawingAreaUpperX; ++i)
		{
			for (unsigned int j = drawingAreaLowerY; j < drawingAreaUpperY; ++j)
			{
				if (  magnitude_int (  i - cursor_grid.x , j - cursor_grid.y )  < radius )
				{
					v.push_back( vec_u2(i, j) );

				}
			}
		}


		break;
	}

	case 'l':
	{
		printf("draw a line\n");
		// rasters a line, taking into account the accumulated rotation
		cursor_string++; if (cursor_string > genesize) {return -1;}

		// set the previous x and y, which will be the start of the line
		prevCursor_grid = cursor_grid;

		bool chosenSignX = false;
		int signX = 1;

		while (!chosenSignX)
		{
			cursor_string++; if (cursor_string > genesize) {return -1;}
			signX = alphanumeric( genes[cursor_string]);
			if (signX > 13) { signX = 1; } else { signX = -1; }
			chosenSignX = true;
		}

		bool chosenSignY = false;
		int signY = 1;

		while (!chosenSignY)
		{
			cursor_string++; if (cursor_string > genesize) {return -1;}
			signY = alphanumeric( genes[cursor_string]);
			if (signY > 13) { signY = 1; }
			else {
				signY = 1; // signY = -1; // i made it this way so the plants always grow up. instead of into the ground.
			}
			chosenSignY = true;
		}

		unsigned int numberModifier = 0;

		while (!numberModifier)
		{
			cursor_string++; if (cursor_string > genesize) {return -1;}
			numberModifier = alphanumeric( genes[cursor_string] );
		}

		bool inRange = true;

		if (cursor_grid.x + numberModifier  > sizeX && signX > 0)
		{
			cursor_grid.x = sizeX - 1;
			inRange = false;
		}
		if (numberModifier > cursor_grid.x && signX < 0 )
		{
			cursor_grid.x = 0;
			inRange = false;
		}
		if (inRange)
		{
			if (signX > 0)
			{
				cursor_grid.x += (numberModifier );
			}
			else
			{
				cursor_grid.x -= (numberModifier );
			}
		}
		numberModifier = 0;

		while (!numberModifier)
		{
			cursor_string++; if (cursor_string > genesize) {return -1;}
			numberModifier = alphanumeric( genes[cursor_string] );
		}

		inRange = true;
		if (cursor_grid.y + numberModifier  > sizeY && signY > 0)
		{
			cursor_grid.y = sizeY - 1;
			inRange = false;
		}
		if (numberModifier > cursor_grid.y && signY < 0 )
		{
			cursor_grid.y = 0;
			inRange = false;
		}

		if (inRange)
		{
			if (signY > 0)
			{
				cursor_grid.y += (numberModifier );
			}
			else
			{
				cursor_grid.y -= (numberModifier );
			}
		}

		vec_f2 rotatedPoint = rotatePointPrecomputed( vec_f2(prevCursor_grid.x, prevCursor_grid.y),  accumulatedRotation_precomputedSin,  accumulatedRotation_precomputedCos, vec_f2(  cursor_grid.x, cursor_grid.y));

		if 		(rotatedPoint.x > 0 && rotatedPoint.x < sizeX - 1) 	{ cursor_grid.x = rotatedPoint.x;}
		else if (rotatedPoint.x < 0) 								{ cursor_grid.x = 0;}
		else if (rotatedPoint.x > sizeX - 1) 						{ cursor_grid.x = sizeX - 1; }

		if 		(rotatedPoint.y > 0 && rotatedPoint.y < sizeY - 1) 	{ cursor_grid.y = rotatedPoint.y;}
		else if (rotatedPoint.y < 0) 								{ cursor_grid.y = 0;}
		else if (rotatedPoint.y > sizeY - 1) 						{ cursor_grid.y = sizeY - 1; }

		v = EFLA_E( prevCursor_grid, cursor_grid);


		break;
	}

	// set immediate red value
	case 'r':
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] ) / 26;
		cursor_color = Color(  numberModifier, cursor_color.g, cursor_color.b, 1.0f    );
		break;
	}

	// set immediate green value
	case 'g':
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] ) / 26;
		cursor_color = Color(  cursor_color.r, numberModifier, cursor_color.b, 1.0f    );
		break;
	}

	// set immediate blue value
	case 'b':
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] ) / 26;
		cursor_color = Color(  cursor_color.r, cursor_color.g, numberModifier, 1.0f    );
		break;
	}

	default:
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
	}
	}

	// paint in the points with material
	// unsigned int n_points = 0;
	for (std::list<vec_u2>::iterator it = v.begin(); it != v.end(); ++it)
	{
		unsigned int i = (it->y * sizeX) + it->x;

		setLifeParticle(  genes, identity, i);
		energyDebtSoFar--;
	}

	// unsigned int n_seeds = 0;
	for (std::list<vec_u2>::iterator it = v_seeds.begin(); it != v_seeds.end(); ++it)
	{
		unsigned int i = (it->y * sizeX) + it->x;

		setSeedParticle(  genes, identity, energyDebtSoFar, i);
		mutateSentence(&(seedGrid[i].genes));
	}

	return 0;
}



void drawPlantFromSeed( std::string genes, unsigned int i )
{
	unsigned int x = 0;
	unsigned int y = 0;

	x = i % sizeX;
	y = i / sizeX;

	int energyDebtSoFar = 0;

	printf("drawPlantFromSeed\n");
	unsigned int identity = newIdentity();
	cursor_string = 2;
	cursor_grid = vec_u2(x, y);
	origin = cursor_grid;
	prevCursor_grid = cursor_grid;

	cursor_color = color_grey;

	drawActions = 0;

	while ( true )
	{
		if ( drawCharacter (genes , identity, energyDebtSoFar) < 0) 
		{
			if (cursor_string > genes.length()) 
			{
				// printf("broke out of the problem spot.\n");
				break;
			}
		} 
	}
}

void thread_life()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	unsigned int x = 0;
	unsigned int y = 0;

	for (unsigned int i = (sizeX + 1); i < (totalSize - (sizeX + 1)); ++i)
	{

		if ( lifeGrid[i].identity)
		{
			x = i % sizeX;
			if (!x) { y = i / sizeX; }

			unsigned int squareBelow = i - sizeX;
			unsigned int squareAbove = i + sizeX;
			unsigned int neighbours[] =
			{
				squareBelow - 1,
				squareBelow,
				squareBelow + 1,
				i - 1,
				i + 1,
				squareAbove - 1,
				squareAbove,
				squareAbove + 1
			};
			for (unsigned int j = 0; j < 8; ++j)
			{
				if (lifeGrid[neighbours[j]].identity == lifeGrid[i].identity)
				{
					int equalizedEnergy = ( lifeGrid[neighbours[j]].energy + lifeGrid[i].energy ) / 2;
					lifeGrid[neighbours[j]].energy = equalizedEnergy;
					lifeGrid[i].energy = equalizedEnergy;
				}
			}
		}
	}

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_life " << elapsed.count() << " microseconds." << std::endl;
#endif
}

void thread_seeds()
{

#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif
	for (unsigned int i = (sizeX + 1); i < (totalSize - (sizeX + 1)); ++i)
	{
		if (seedGrid[i].stage == STAGE_PHOTON)
		{
			unsigned int squareBelow = i - sizeX;
			if (lifeGrid[squareBelow].identity > 0)
			{
				if (extremelyFastNumberFromZeroTo(8) == 0)
				{
					lifeGrid[squareBelow].energy += 1.0f;
					clearSeedParticle(i);
					continue;
				}
				else
				{
					swapSeedParticle( i, squareBelow);
					continue;
				}
			}
			if (grid[squareBelow].phase != PHASE_VACUUM)
			{
				clearSeedParticle(i);
				continue;
			}
			else
			{
				if (seedGrid[squareBelow].stage != 0x00)
				{
					clearSeedParticle(i);
					continue;
				}
				swapSeedParticle( i, squareBelow);
				continue;
			}
		}

		if (seedGrid[i].parentIdentity > 0)
		{
			if (seedGrid[i].stage == STAGE_SEED)
			{
				drawPlantFromSeed(seedGrid[i].genes, i);
				clearSeedParticle(i);
				break;
			}

			if (seedGrid[i].stage == STAGE_FRUIT)
			{
				unsigned int squareBelow = i - sizeX;
				unsigned int squareAbove = i + sizeX;
				unsigned int neighbours[] =
				{
					squareBelow - 1,
					squareBelow,
					squareBelow + 1,
					i - 1,
					i + 1,
					squareAbove - 1,
					squareAbove,
					squareAbove + 1
				};
				unsigned int nSolidNeighbours = 0;
				for (unsigned int j = 0; j < 8; ++j)
				{
					if (j < 5)
					{
						if (grid[neighbours[j]].phase == PHASE_VACUUM || grid[neighbours[j]].phase == PHASE_GAS )
						{
							if (extremelyFastNumberFromZeroTo(8) == 0)
							{
								swapSeedParticle( i, neighbours[j] );
								break;
							}
						}
						if (grid[neighbours[j]].material == MATERIAL_QUARTZ)
						{
							nSolidNeighbours++;
						}
					}

					if (grid[neighbours[j]].phase == PHASE_LIQUID )
					{
						if (extremelyFastNumberFromZeroTo(8) == 0)
						{
							swapSeedParticle( i, neighbours[j] );
							break;
						}
					}
				}
				if (nSolidNeighbours > 2)
				{
					seedGrid[i].stage = STAGE_SEED;
				}
				continue;
			}

			if (seedGrid[i].stage == STAGE_BUD)
			{
				unsigned int squareBelow = i - sizeX;
				unsigned int squareAbove = i + sizeX;
				unsigned int neighbours[] =
				{
					squareBelow - 1,
					squareBelow,
					squareBelow + 1,
					i - 1,
					i + 1,
					squareAbove - 1,
					squareAbove,
					squareAbove + 1
				};

				unsigned int nFeedingCells = 0;
				for (unsigned int j = 0; j < 8; ++j)
				{
					if (lifeGrid[neighbours[j]].identity == seedGrid[i].parentIdentity)
					{
						if (lifeGrid[neighbours[j]].energy > 0)
						{
							seedGrid[i].energy += (lifeGrid[neighbours[j]].energy/2);
							lifeGrid[neighbours[j]].energy = (lifeGrid[neighbours[j]].energy/2);
						}
						nFeedingCells++;
					}
				}

				if (nFeedingCells == 0)
				{
					clearSeedParticle(i);
					continue;
				}

				if (seedGrid[i].energy > 0)
				{
					seedGrid[i].stage = STAGE_FRUIT;
				}

				continue;
			}
		}
	}

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_seeds " << elapsed.count() << " microseconds." << std::endl;
#endif

}