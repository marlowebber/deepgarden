#include "deepgarden.h"
#include "deepgarden_graphics.h"

#include <ctime>
#include <chrono>
#include <iostream>

// #define THREAD_TIMING
#define RENDERING_THREADS 4
#define PLANT_DRAWING_READOUT 1

const unsigned int totalSize = sizeX * sizeY;
const unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

const Color color_lightblue = Color( 0.1f, 0.3f, 0.65f, 1.0f );
const Color color_yellow    = Color( 1.0f, 1.0f, 0.0f, 1.0f );
const Color color_lightgrey = Color( 0.75f, 0.75f, 0.75f, 1.0f );
const Color color_grey      = Color( 0.50f, 0.50f, 0.50f, 1.0f );
const Color color_darkgrey  = Color( 0.25f, 0.25f, 0.25f, 1.0f );
const Color color_black     = Color( 0.0f, 0.0f, 0.0f, 1.0f );
const Color color_white_clear     = Color( 1.0f, 1.0f, 1.0f, 0.25f );
const Color color_purple     = Color( 0.8f, 0.0f, 0.8f, 1.0f );

const Color color_clear     = Color( 0.0f, 0.0f, 0.0f, 0.0f );


const Color color_defaultSeedColor     = Color( 0.75f, 0.35f, 0.1f, 1.0f );

const Color color_defaultColor     = Color( 0.35f, 0.35f, 0.35f, 1.0f );


// a tree with a long stem and a fan of branches.
std::string plant_Pycad = std::string( "rqgqbqxuydzb. lz addlzf." );

// a fern with a long curling frond.
std::string plant_Lomondra = std::string( "rjgmbdxqygzd. lmskfoplhtdlh  sccldf " );

// a tall reed.
std::string plant_Worrage = std::string( "rdgqbixfygzu. shlmacclg  acclgf " );

// a fat, round stink ball.
std::string plant_MilkWombler = std::string( "rmggbmxuyuzu. cz. lz sggofldf " );

// a mysterious urchin.
std::string plant_SpleenCoral = std::string( "rgggbuxuyuzu. cz. rmgmbw. azzlmcdf  " );


float colorGrid[ totalSize * numberOfFieldsPerVertex ];
float lifeColorGrid[totalSize * numberOfFieldsPerVertex ];
float lifeColorGridB[totalSize * numberOfFieldsPerVertex ];
float seedColorGrid[totalSize * numberOfFieldsPerVertex];


// int wind = 0;


vec_i2 wind = vec_i2(0, 0);


int defaultTemperature = 300;

unsigned int recursion_level = 0;
const unsigned int recursion_limit = 4;

unsigned int drawActions = 0;
const unsigned int drawActionlimit = 200;

// variables keep track of the sequence and rotation states.
float accumulatedRotation = (0.5 * 3.1415);

vec_u2 cursor_grid = vec_u2(0, 0);
vec_u2 prevCursor_grid = vec_u2(0, 0);
vec_u2 origin = vec_u2(0, 0);


unsigned int cursor_string = 0;
Color cursor_color = Color(0.1f, 0.1f, 0.1f, 1.0f);
Color cursor_seedColor = Color(0.1f, 0.1f, 0.1f, 1.0f);

float lengthNoise = 0.0f;

float energyDebtSoFar = 0.0f;

float maximumDisplayEnergy = 1;

std::list<unsigned int> identities;


std::list<vec_u2> v_seeds;

bool showEnergyGrid = false;

unsigned int nGerminatedSeeds = 0;
unsigned int germinatedSeedsLimit = 1000;
unsigned int lampBrightness = 250;
struct Particle
{
	uint8_t material;
	uint8_t phase;
	uint16_t temperature;
	Particle();
};

Particle::Particle()
{
	this->material = MATERIAL_VACUUM;
	this->phase = PHASE_VACUUM;
	this->temperature = defaultTemperature;
}

struct LifeParticle
{
	std::string genes;
	uint16_t identity;
	float energy;
	LifeParticle();
};

LifeParticle::LifeParticle()
{
	this->genes = std::string("");
	this->identity = 0x00;
	this->energy = 0.0f;
}

struct ProposedLifeParticle
{
	Color color = Color(0.5f, 0.5f, 0.5f, 1.0f);
	vec_u2 position = vec_u2(0, 0);

	ProposedLifeParticle(Color color, vec_u2 position);


};


ProposedLifeParticle::ProposedLifeParticle(Color color, vec_u2 position)
{
	this->position = position;
	this->color = color;
}

struct SeedParticle
{
	uint16_t parentIdentity;
	std::string genes;
	float energy;
	uint8_t stage;


	SeedParticle();

};

SeedParticle::SeedParticle()
{
	this->genes = std::string("");
	this->parentIdentity = 0x00;
	this->energy = 0.0f;
	this->stage = STAGE_NULL;
}


Particle grid[totalSize];
LifeParticle lifeGrid[totalSize];
SeedParticle seedGrid[totalSize];

std::list<ProposedLifeParticle> v;



std::string randomSentence(unsigned int length)
{

	std::string s = std::string("");

	for (int i = 0; i < length; ++i)
	{
		s.push_back((char)('a' + rand() % 26));
	}

	return s;

}

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




Color materialColor ( unsigned int material )
{
	if ((material == MATERIAL_VACUUM))      {    return color_clear;      }
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

void setLifeParticle( std::string genes, unsigned int identity, unsigned int i, Color color)
{
	lifeGrid[i].identity = identity;
	lifeGrid[i].genes  = genes;
	lifeGrid[i].energy = 0.0f;

	memcpy( (&lifeColorGrid[i * numberOfFieldsPerVertex]),  &(color),  sizeof(Color) );


	// to draw 'shadows', find the particle that's one pixel down and one across, and set its color down by half.
	if (i > sizeX)
	{
		unsigned int shadowIndex = (i - sizeX) + 1;
		shadowIndex = (shadowIndex * numberOfFieldsPerVertex);

		lifeColorGrid[shadowIndex + 0] = lifeColorGrid[ shadowIndex + 0] * 0.5f;
		lifeColorGrid[shadowIndex + 1] = lifeColorGrid[ shadowIndex + 1] * 0.5f;
		lifeColorGrid[shadowIndex + 2] = lifeColorGrid[ shadowIndex + 2] * 0.5f;
	}
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

#ifdef PLANT_DRAWING_READOUT
	printf("seed with energy debt %f \n", energyDebt);
#endif

	memcpy( (&seedColorGrid[i * numberOfFieldsPerVertex]) ,  &(cursor_seedColor),  sizeof(Color) );
}

void setPhoton(  unsigned int i)
{
	seedGrid[i].stage = STAGE_PHOTON;
	memcpy( (&seedColorGrid[i * numberOfFieldsPerVertex]) ,  &(color_white_clear),  sizeof(Color) );
}

void clearSeedParticle( unsigned int i)
{
	seedGrid[i].stage = 0x00;
	seedGrid[i].parentIdentity = 0x00;
	seedGrid[i].energy = 0.0f;
	seedGrid[i].genes = std::string("");
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

#ifdef PLANT_DRAWING_READOUT
			printf("claimed new ID: %u\n", identityCursor);
#endif

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

	// memset(grid, 0x00, sizeof(Particle) * ( totalSize));

	// // setup the x and y positions in the color grid. these never change so you can just calculate them once.
	unsigned int x = 0;
	unsigned int y = 0;
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		x = i % sizeX;
		if (!x) { y = i / sizeX; }
		float fx = x;
		float fy = y;

		grid[i] = Particle();
		seedGrid[i] = SeedParticle();
		lifeGrid[i] = LifeParticle();

		// RGBA color occupies the first 4 places.
		// also, initialize the color alpha to 1.


		lifeColorGridB[ (i * numberOfFieldsPerVertex) + 3] = 0.0f;
		lifeColorGridB[ (i * numberOfFieldsPerVertex) + 4] = fx;
		lifeColorGridB[ (i * numberOfFieldsPerVertex) + 5] = fy;


		colorGrid[ (i * numberOfFieldsPerVertex) + 3] = 0.0f;
		colorGrid[ (i * numberOfFieldsPerVertex) + 4] = fx;
		colorGrid[ (i * numberOfFieldsPerVertex) + 5] = fy;

		lifeColorGrid[ (i * numberOfFieldsPerVertex) + 3] = 0.0f;
		lifeColorGrid[ (i * numberOfFieldsPerVertex) + 4] = fx;
		lifeColorGrid[ (i * numberOfFieldsPerVertex) + 5] = fy;

		seedColorGrid[ (i * numberOfFieldsPerVertex) + 3] = 0.0f;
		seedColorGrid[ (i * numberOfFieldsPerVertex) + 4] = fx;
		seedColorGrid[ (i * numberOfFieldsPerVertex) + 5] = fy;

		if (true)
		{
			// olivine cube

			// if (x > 100 && x < 200)
			// {
			// 	if (y > 100 && y < 200)
			// 	{
			// 		setParticle( MATERIAL_QUARTZ, i);
			// 	}
			// }

			// if (x > 500 && x < 600)
			// {
			// 	if (y > 100 && y < 200)
			// 	{
			// 		setParticle( MATERIAL_QUARTZ, i);
			// 	}
			// }

			// if (x > 800 && x < 900)
			// {
			// 	if (y > 100 && y < 200)
			// 	{
			// 		setParticle( MATERIAL_QUARTZ, i);
			// 	}
			// }
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
			// if (i > (100 * sizeX) && i < (125 * sizeX))
			// {
			// 	if (RNG() < 0.5)
			// 	{
			// 		setParticle( MATERIAL_WATER, i);
			// 	}
			// }
		}

		if (x == 500 && y == 100)
		{

			// setSeedParticle(exampleSentence, newIdentity() , 0, i);
			// seedGrid[i].stage = STAGE_FRUIT;
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


			uint8_t offset = extremelyFastNumberFromZeroTo(4);
			for (uint8_t k = 0; k < 5; ++k)
			{

				unsigned int index = (k + offset) % 5;

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

	// blow the wind
	uint16_t windChange = extremelyFastNumberFromZeroTo(100);

	if (!windChange)
	{
		int16_t windRand =  extremelyFastNumberFromZeroTo( 2);
		wind.x = windRand - 1;

		windRand =  extremelyFastNumberFromZeroTo( 2);
		wind.y = windRand - 1;

		printf("CHAMGED WIMD %i %i \n", wind.x, wind.y);
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
			if (extremelyFastNumberFromZeroTo(lampBrightness) == 0)
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

void toggleEnergyGridDisplay ()
{
	showEnergyGrid = !showEnergyGrid;
}

void thread_graphics()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif
	preDraw();

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

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, lifeColorGridB, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

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
std::list<ProposedLifeParticle> EFLA_E(vec_u2 start, vec_u2 end)
{
	int x  = start.x;
	int y  = start.y;
	int x2 = end.x;
	int y2 = end.y;

	std::list<ProposedLifeParticle> v;

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
				v.push_back(   ProposedLifeParticle(cursor_color, vec_u2(j >> 8, y))   );
				j += decInc;
			}
			return v;
		}
		longLen += y;
		for (int j = 0x80 + (x << 8); y >= longLen; --y) {
			v.push_back( ProposedLifeParticle(cursor_color, vec_u2(j >> 8, y) ));
			j -= decInc;
		}
		return v;
	}

	if (longLen > 0) {
		longLen += x;
		for (int j = 0x80 + (y << 8); x <= longLen; ++x) {
			v.push_back( ProposedLifeParticle(cursor_color, vec_u2(x, j >> 8) ) );
			j += decInc;
		}
		return v;
	}
	longLen += x;
	for (int j = 0x80 + (y << 8); x >= longLen; --x) {
		v.push_back( ProposedLifeParticle(cursor_color, vec_u2(x, j >> 8) ));
		j -= decInc;
	}
	return v;
}

int drawCharacter ( std::string genes , unsigned int identity)
{
	char c = genes[cursor_string];

#ifdef PLANT_DRAWING_READOUT
	printf("char %c cursor %u  ", c, cursor_string);
#endif

	drawActions++;
	// printf("cursor_grid %u %u\n", cursor_grid.x, cursor_grid.y);
	if (recursion_level > recursion_limit)
	{

#ifdef PLANT_DRAWING_READOUT
		printf("recursion_level exceeded\n");
#endif

		cursor_string++;
		return -1;
	}

	if (drawActions > drawActionlimit)
	{


#ifdef PLANT_DRAWING_READOUT
		printf("drawActions exceeded\n");
#endif

		cursor_string++;
		return -1;
	}



	unsigned int genesize = genes.length();

	switch (c)
	{

	case 't': // branch. a sequence that grows at an angle to the main trunk
	{


		cursor_string++; if (cursor_string > genesize) {return -1;}

		float numberModifier = 0.0f;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		float branchRotation = ((numberModifier - 13.0f) / 13.0f) * 3.14159f;



#ifdef PLANT_DRAWING_READOUT
		printf("branch with angle %f\n", branchRotation);
#endif

		// record trunk rotation
		float trunkRotation = accumulatedRotation;

		// figure out new branch rotation
		accumulatedRotation = accumulatedRotation + branchRotation;

		// record old cursor position
		vec_u2 old_cursorGrid = cursor_grid;

		recursion_level++;
		while (1)
		{
			if ( drawCharacter(genes, identity) < 0)
			{
				break;
			}
		}
		recursion_level--;

		// return to normal rotation
		accumulatedRotation = trunkRotation;

		// return to normal position
		cursor_grid = old_cursorGrid;

		break;
	}


	case 'a': // array. a motif is repeated a few times from the same place but with an angle offset.
	{



		cursor_string++; if (cursor_string > genesize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		// rotationIncrement describes what fraction of the whole circle the array occupies.
		float arrayTotalAngle =  numberModifier;
		arrayTotalAngle = arrayTotalAngle / 26;
		arrayTotalAngle = arrayTotalAngle * 2 * 3.1415;


		numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		float repeats = numberModifier;
		float rotationIncrement = (arrayTotalAngle / repeats);



#ifdef PLANT_DRAWING_READOUT
		printf("array angle %f repeats %f \n", rotationIncrement, repeats);
#endif

		float trunkRotation = accumulatedRotation; // rotation of the stem that the array is growing on.
		float arrayBaseRotation = (repeats * rotationIncrement) / 2 ; // rotation of the 0th (most counter-clockwise) element in the array.
		accumulatedRotation -= arrayBaseRotation;

		// record old cursor position
		vec_u2 old_cursorGrid = cursor_grid;
		unsigned int sequenceOrigin = cursor_string;

		prevCursor_grid = cursor_grid;

		recursion_level++;
		for (int i = 0; i < repeats; ++i)
		{


			while (1)
			{
				if ( drawCharacter(genes, identity) < 0)
				{
					break;
				}
			}

			cursor_string = sequenceOrigin;

			accumulatedRotation += rotationIncrement ;

			cursor_grid = old_cursorGrid;

		}

		recursion_level--;

		// return to normal rotation
		accumulatedRotation = trunkRotation;

		// return to normal position
		cursor_grid = old_cursorGrid;

		break;
	}


	case 's': // sequence. a motif is repeated serially a number of times.
	{

#ifdef PLANT_DRAWING_READOUT
		printf("sequence\n");
#endif

		cursor_string++; if (cursor_string > genesize) {return -1;}

		// get number of times to repeat the sequence
		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ) / 2  ; cursor_string++; if (cursor_string > genesize) {return -1;} }

		int repeats = numberModifier % 8;


		// the character after that is the next thing to be arrayed
		cursor_string++; if (cursor_string > genesize) {return -1;}

		unsigned int sequenceOrigin = cursor_string;
		prevCursor_grid = cursor_grid;

		recursion_level++;
		for ( int i = 0; i < repeats; ++i)
		{
			while (1)
			{
				if ( drawCharacter(genes, identity) < 0)
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

#ifdef PLANT_DRAWING_READOUT
		printf("break\n");
#endif

		cursor_string++; if (cursor_string > genesize) {return -1;}


		return -1;
		break;
	}

	case ',': // break array and return cursor to prevcursor
	{
#ifdef PLANT_DRAWING_READOUT
		printf("break and return cursor\n");
#endif
		cursor_string++; if (cursor_string > genesize) {return -1;}
		cursor_grid = prevCursor_grid;

		return -1;
		break;
	}


	case '.': // break array and return cursor to origin
	{
#ifdef PLANT_DRAWING_READOUT
		printf("break and return cursor\n");
#endif
		cursor_string++; if (cursor_string > genesize) {return -1;}
		cursor_grid = origin;

		return -1;
		break;
	}

	case 'f': // make a seed
	{

#ifdef PLANT_DRAWING_READOUT
		printf("draw a seed\n");
#endif


		cursor_string++; if (cursor_string > genesize) {return -1;}

		v_seeds.push_back( cursor_grid );


		break;
	}

	case 'c': // paint a circle at the cursor
	{

#ifdef PLANT_DRAWING_READOUT
		printf("draw a circle\n");
#endif


		cursor_string++; if (cursor_string > genesize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		// get the circle radius (the next number in the string). nerfed on purpose otherwise there are giant blobs everywhere.
		unsigned int radius = numberModifier / 2;

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
					v.push_back( ProposedLifeParticle(cursor_color, vec_u2(i, j) ));

				}
			}
		}


		break;
	}

	case 'l':
	{


#ifdef PLANT_DRAWING_READOUT
		printf("draw a line (with angle and length noise)");
#endif



		// rasters a line, taking into account the accumulated rotation
		cursor_string++; if (cursor_string > genesize) {return -1;}

		// set the previous x and y, which will be the start of the line
		prevCursor_grid = cursor_grid;


		int numberModifier = 0;

		// while (!numberModifier)
		// {
		numberModifier = alphanumeric( genes[cursor_string] ) ;



#ifdef PLANT_DRAWING_READOUT
		printf("length %i \n", numberModifier);
#endif

		cursor_string++; if (cursor_string > genesize) {return -1;}

		// numberModifier = numberModifier;

		accumulatedRotation += ( RNG() - 0.5 ) * 0.1; // angle noise is not heritable, and just serves to make the world more natural-looking.


		int deltaX = (numberModifier + ( numberModifier * lengthNoise)) * cos(accumulatedRotation);
		int deltaY = (numberModifier + ( numberModifier * lengthNoise)) * sin(accumulatedRotation);

		bool wrap = false;

		cursor_grid.x += deltaX;
		cursor_grid.y += deltaY;

		v.splice(v.end(), EFLA_E( prevCursor_grid, cursor_grid) );

		break;
	}

	// set immediate red value
	case 'r':
	{

		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] );
		numberModifier = numberModifier / 26;

#ifdef PLANT_DRAWING_READOUT
		printf("set color R %f, char '%c'\n", numberModifier, genes[cursor_string] );
#endif
		cursor_color = Color(  numberModifier, cursor_color.g, cursor_color.b, 1.0f    );

		cursor_string++; if (cursor_string > genesize) {return -1;}
		break;
	}

	// set immediate green value
	case 'g':
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] );
		numberModifier = numberModifier / 26;



#ifdef PLANT_DRAWING_READOUT
		printf("set color G %f, char '%c'\n", numberModifier, genes[cursor_string] );
#endif

		cursor_color = Color(  cursor_color.r, numberModifier, cursor_color.b, 1.0f    );

		cursor_string++; if (cursor_string > genesize) {return -1;}
		break;
	}

	// set immediate blue value
	case 'b':
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] );
		numberModifier = numberModifier / 26;

#ifdef PLANT_DRAWING_READOUT
		printf("set color B %f, char '%c'\n", numberModifier, genes[cursor_string] );
#endif

		cursor_color = Color(  cursor_color.r, cursor_color.g, numberModifier, 1.0f    );

		cursor_string++; if (cursor_string > genesize) {return -1;}
		break;
	}


// set seed red
	case 'x':
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] );
		numberModifier = numberModifier / 26;

#ifdef PLANT_DRAWING_READOUT
		printf("set seed color R %f, char '%c'\n", numberModifier, genes[cursor_string] );
#endif
		cursor_seedColor = Color(  numberModifier, cursor_seedColor.g, cursor_seedColor.b, 1.0f    );

		cursor_string++; if (cursor_string > genesize) {return -1;}
		break;
	}
// set seed green
	case 'y':
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] );
		numberModifier = numberModifier / 26;

#ifdef PLANT_DRAWING_READOUT
		printf("set seed color G %f, char '%c'\n", numberModifier, genes[cursor_string] );
#endif
		cursor_seedColor = Color(  cursor_seedColor.r, numberModifier, cursor_seedColor.b, 1.0f    );

		cursor_string++; if (cursor_string > genesize) {return -1;}
		break;
	}
// set seed blue
	case 'z':
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] );
		numberModifier = numberModifier / 26;

#ifdef PLANT_DRAWING_READOUT
		printf("set seed color B %f, char '%c'\n", numberModifier, genes[cursor_string] );
#endif
		cursor_seedColor = Color(  cursor_seedColor.r, cursor_seedColor.g, numberModifier, 1.0f    );

		cursor_string++; if (cursor_string > genesize) {return -1;}
		break;
	}

	// modify angle
	case 'o':
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] );
		numberModifier = ((numberModifier - 13) / 13) * 3.1415 * 0.5;

#ifdef PLANT_DRAWING_READOUT
		printf("modify angle %f char '%c'\n", numberModifier,  genes[cursor_string]);
#endif

		accumulatedRotation += numberModifier;

		cursor_string++; if (cursor_string > genesize) {return -1;}
		break;
	}


	default:
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
	}
	}



	return 0;
}



void drawPlantFromSeed( std::string genes, unsigned int i )
{
	unsigned int x = 0;
	unsigned int y = 0;

	x = i % sizeX;
	y = i / sizeX;

#ifdef PLANT_DRAWING_READOUT
	printf("drawPlantFromSeed\n");
#endif
	unsigned int identity = newIdentity();
	cursor_string = 0;
	cursor_grid = vec_u2(x, y);
	origin = cursor_grid;
	prevCursor_grid = cursor_grid;
	accumulatedRotation = (0.5 * 3.1415);

	lengthNoise = ( RNG() - 0.5 ) * 0.25 ;

	// color noise helps a forest to look a bit more interesting. It is not genetically heritable.
	float colorNoiseLevel = (RNG() - 0.5) * 0.1;

	cursor_color =  Color(
	                    color_defaultColor.r + colorNoiseLevel,
	                    color_defaultColor.g + colorNoiseLevel,
	                    color_defaultColor.b + colorNoiseLevel,
	                    1.0f
	                );

	float seedColorNoiseLevel = (RNG() - 0.5) * 0.1;


	cursor_seedColor =  Color(
	                        color_defaultSeedColor.r + seedColorNoiseLevel,
	                        color_defaultSeedColor.g + seedColorNoiseLevel,
	                        color_defaultSeedColor.b + seedColorNoiseLevel,
	                        1.0f
	                    );



	drawActions = 0;

	v.clear();
	v_seeds.clear();



	while ( true )
	{
		if ( drawCharacter (genes , identity) < 0)
		{
			if (cursor_string > genes.length())
			{
				// printf("broke out of the problem spot.\n");
				break;
			}
		}
	}


	energyDebtSoFar = 0.0f;

	// paint in the points with material
	// unsigned int n_points = 0;
	for (std::list<ProposedLifeParticle>::iterator it = v.begin(); it != v.end(); ++it)
	{
		unsigned int i = (it->position.y * sizeX) + it->position.x;

		if ( i < totalSize)
		{
			setLifeParticle(  genes, identity, i, it->color);
			clearSeedParticle(i);
			energyDebtSoFar -= 1.0f;
		}

	}

	// unsigned int n_seeds = 0;
	for (std::list<vec_u2>::iterator it = v_seeds.begin(); it != v_seeds.end(); ++it)
	{
		unsigned int i = (it->y * sizeX) + it->x;

		if ( i < totalSize)
		{
			setSeedParticle(  genes, identity, energyDebtSoFar, i);
			mutateSentence(&(seedGrid[i].genes));
		}
	}


	// cursor_energyDebt = 0;
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
					float equalizedEnergy = ( lifeGrid[neighbours[j]].energy + lifeGrid[i].energy ) / 2;
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




void thread_plantDrawing()
{

#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif


	for (unsigned int i =  0; i < totalSize; ++i)
	{
		if (seedGrid[i].parentIdentity > 0)
		{
			if (seedGrid[i].stage == STAGE_SEED)
			{
				drawPlantFromSeed(seedGrid[i].genes, i);
				clearSeedParticle(i);
				nGerminatedSeeds ++;
				continue;
			}

		}


	}


#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_plantDrawing " << elapsed.count() << " microseconds." << std::endl;
#endif

}







void thread_seeds()
{

#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif


// unsigned int nGerminatedSeeds = 0;
// unsigned int germinatedSeedsLimit = 1000;
	if ( nGerminatedSeeds > germinatedSeedsLimit )
	{

		sendLifeToBackground();

	}



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
					if (seedGrid[squareBelow].parentIdentity > 0x00)
					{
						clearSeedParticle(i);
						continue;
					}
					else
					{
						swapSeedParticle( i, squareBelow);
						continue;
					}

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
			// if (seedGrid[i].stage == STAGE_SEED)
			// {
			// 	drawPlantFromSeed(seedGrid[i].genes, i);
			// 	clearSeedParticle(i);
			// 	nGerminatedSeeds ++;
			// 	continue;
			// }

			if (seedGrid[i].stage == STAGE_FRUIT)
			{
				// unsigned int squareBelow = i - sizeX;
				// unsigned int squareAbove = i + sizeX;

				// unsigned int neighbourAddressLookups[] =
				// {
				// 	squareBelow - 1,
				// 	squareBelow,
				// 	squareBelow + 1,
				// 	i + 1,
				// 	i - 1,
				// 	squareAbove - 1,
				// 	squareAbove,
				// 	squareAbove + 1
				// };

				// for (unsigned int j = 0; j < 8; ++j)
				// {


				if (extremelyFastNumberFromZeroTo(1) == 0) // get blown by the wind only some of the time
				{
					while (true) // this while loop is just here so you can 'break' out of it to end this sequence quickly.
					{
						unsigned int neighbour;
						if (wind.x > 0)
						{
							neighbour = i + 1;
							if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  )
							{
								swapSeedParticle( i, neighbour );
								break;
							}
						}
						else if (wind.x < 0)
						{
							neighbour = i - 1;
							if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  )
							{
								swapSeedParticle( i, neighbour );
								break;
							}
						}
						break;
					}

					while (true) // this while loop is just here so you can 'break' out of it to end this sequence quickly.
					{
						unsigned int neighbour;
						if (wind.y > 0)
						{
							neighbour = i + sizeX;
							if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  )
							{
								swapSeedParticle( i, neighbour );
								break;
							}
						}
						else if (wind.y < 0)
						{
							neighbour = i - sizeX;
							if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  )
							{
								swapSeedParticle( i, neighbour );
								break;
							}
						}
						break;
					}
				}

				unsigned int j = extremelyFastNumberFromZeroTo(4);
				unsigned int neighbour;

				if (		j == 0)		{ neighbour = i - sizeX - 1 ;	}
				else if (	j == 1)		{ neighbour = i - sizeX	 	;	}
				else if (	j == 2)		{ neighbour = i - sizeX + 1 ;	}
				else if (	j == 3)		{ neighbour = i + 1 		;	}
				else if (	j == 4)		{ neighbour = i - 1  		;	}
				// else if (	j == 5)		{ neighbour = i + sizeX - 1 ;	}
				// else if (	j == 6)		{ neighbour = i + sizeX		;	}
				// else if (	j == 7)		{ neighbour = i + sizeX + 1 ;	}



				if (grid[neighbour].material == MATERIAL_QUARTZ)
				{


#ifdef PLANT_DRAWING_READOUT
					printf("germinated\n");
#endif
					seedGrid[i].stage = STAGE_SEED;
				}

				// if (j < 5)
				// {
				if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  )
				{
					// if (extremelyFastNumberFromZeroTo(4) == 0)
					// {
					swapSeedParticle( i, neighbour );
					continue;
					// }
				}

				// if (grid[neighbour].phase == PHASE_LIQUID )
				// {

				// }

				// }

				// else if (j > 2)
				// {

				// 	if (grid[neighbour].phase == PHASE_LIQUID || grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS )
				// 	{
				// 		// if (extremelyFastNumberFromZeroTo(4) == 0)
				// 		// {
				// 		swapSeedParticle( i, neighbour );
				// 		break;
				// 		// }
				// 	}
				// }

				continue;
			}

			if (seedGrid[i].stage == STAGE_BUD)
			{
				// unsigned int squareBelow = i - sizeX;
				// unsigned int squareAbove = i + sizeX;
				// unsigned int neighbours[] =
				// {
				// 	squareBelow - 1,
				// 	squareBelow,
				// 	squareBelow + 1,
				// 	i - 1,
				// 	i + 1,
				// 	squareAbove - 1,
				// 	squareAbove,
				// 	squareAbove + 1
				// };


				unsigned int j = extremelyFastNumberFromZeroTo(7);
				unsigned int neighbour;

				if (		j == 0)		{ neighbour = i - sizeX - 1 ;	}
				else if (	j == 1)		{ neighbour = i - sizeX	 	;	}
				else if (	j == 2)		{ neighbour = i - sizeX + 1 ;	}
				else if (	j == 3)		{ neighbour = i + 1	    	;	}
				else if (	j == 4)		{ neighbour = i - 1 		;	}
				else if (	j == 5)		{ neighbour = i + sizeX - 1 ;	}
				else if (	j == 6)		{ neighbour = i + sizeX		;	}
				else if (	j == 7)		{ neighbour = i + sizeX + 1 ;	}


				// unsigned int nFeedingCells = 0;
				// for (unsigned int j = 0; j < 8; ++j)
				// {
				if (lifeGrid[neighbour].identity == seedGrid[i].parentIdentity)
				{
					// printf("neighbour energy %f\n", lifeGrid[neighbours[j]].energy);
					if (lifeGrid[neighbour].energy > 0)
					{
						seedGrid[i].energy += (lifeGrid[neighbour].energy );
						lifeGrid[neighbour].energy = (lifeGrid[neighbour].energy );
					}

					lifeGrid[neighbour].energy -= 0.005f; // upkeep cost of the seed, balances the number of seeds vs leaf area.
					// nFeedingCells++;
				}
				// }

				// if (nFeedingCells == 0)
				// {
				// 	clearSeedParticle(i);
				// 	continue;
				// }

				if (seedGrid[i].energy >= 0)
				{
					seedGrid[i].stage = STAGE_FRUIT;

#ifdef PLANT_DRAWING_READOUT
					printf("fruited\n");
#endif
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



void sendLifeToBackground ()
{

	nGerminatedSeeds = 0;

	unsigned int nSeeds = 0;
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		if (seedGrid[i].parentIdentity > 0)
		{
			nSeeds ++;
		}

	}


	unsigned int nSeedsDesired = 100;
	unsigned int seedReductionRatio = nSeeds / nSeedsDesired;






	for (unsigned int i = 0; i < totalSize; ++i)
	{


		lifeColorGridB[ (i * numberOfFieldsPerVertex) + 0 ] = lifeColorGrid[(i * numberOfFieldsPerVertex) + 0];
		lifeColorGridB[ (i * numberOfFieldsPerVertex) + 1 ] = lifeColorGrid[(i * numberOfFieldsPerVertex) + 1];
		lifeColorGridB[ (i * numberOfFieldsPerVertex) + 2 ] = lifeColorGrid[(i * numberOfFieldsPerVertex) + 2];
		lifeColorGridB[ (i * numberOfFieldsPerVertex) + 3 ] = lifeColorGrid[(i * numberOfFieldsPerVertex) + 3] = 0.5f;


		lifeColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = 0.0f;
		lifeColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = 0.0f;
		lifeColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = 0.0f;
		lifeColorGrid[ (i * numberOfFieldsPerVertex) + 3 ] = 0.0f;



		lifeGrid[i].identity = 0;
		lifeGrid[i].energy = 0.0f;
		lifeGrid[i].genes = std::string("");


		if (seedGrid[i].parentIdentity > 0)
		{
			if (extremelyFastNumberFromZeroTo(seedReductionRatio) == 0)
			{
				seedGrid[i].stage = STAGE_FRUIT;
				seedGrid[i].energy = 0;
			}
			else
			{
				clearSeedParticle(i);
			}

		}



	}


}

void insertRandomSeed()
{






	unsigned int x = 0;
	unsigned int y = 0;

	unsigned int targetX = extremelyFastNumberFromZeroTo(sizeX);
	unsigned int targetY = extremelyFastNumberFromZeroTo(sizeY / 2) + (sizeY / 2);

	for (unsigned int i = (sizeX + 1); i < (totalSize - (sizeX + 1)); ++i)
	{


		x = i % sizeX;
		if (!x) { y = i / sizeX; }



		if (x == targetX && y == targetY)
		{





// 			// a tree with a long stem and a fan of branches.
// std::string plant_Pycad = std::string( "rqgqbqxuydzb. lz addlzf." );

// // a fern with a long curling frond.
// std::string plant_Lomondra = std::string( "rjgmbdxqygzd. lmskfoplhtdlh  sccldf " );

// // a tall reed.
// std::string plant_Worrage = std::string( "rdgqbixfygzu. shlmacclg  acclgf " );

// // a fat, round stink ball.
// std::string exampleSentence = std::string( "rmggbmxuyuzu. cz. lz sggofldf " );

// // a mysterious urchin.
// std::string plant_SpleenCoral = std::string( "rgggbuxuyuzu. cz. rmgmbw. azzlmcdf  " );

			std::string exampleSentence = std::string("");


			unsigned int randomPlantIndex = extremelyFastNumberFromZeroTo(4);

			switch (randomPlantIndex)
			{
			case 0:
			{
				exampleSentence = plant_Pycad;
				break;
			}
			case 1:
			{
				exampleSentence = plant_Lomondra;
				break;
			}
			case 2:
			{
				exampleSentence = plant_Worrage;
				break;
			}
			case 3:
			{
				exampleSentence = plant_MilkWombler;
				break;
			}
			case 4:
			{
				exampleSentence = plant_SpleenCoral;
				break;
			}

			}










			setSeedParticle(exampleSentence, newIdentity() , 0, i);
			seedGrid[i].stage = STAGE_FRUIT;
		}



	}
}


void increaseLampBrightness ()
{
	lampBrightness ++;
}

void decreaseLampBrightness ()
{
	lampBrightness--;
}