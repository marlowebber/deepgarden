#include "deepgarden.h"
#include "deepgarden_graphics.h"
#include <ctime>
#include <chrono>
#include <iostream>
#include <fstream>
#include "main.h"

// #define THREAD_TIMING_READOUT 1
// #define DETAIL_TIMING_READOUT 1

// #define PLANT_DRAWING_READOUT 1
#define ANIMAL_DRAWING_READOUT 1
// #define ANIMAL_BEHAVIOR_READOUT 1
// #define MUTATION_READOUT 1

// these are used by the developer to turn features on and off.
const bool useGerminationMaterial = false;
const bool animalReproductionEnabled = true;
const bool doWeather = true;
const bool carnageMode = false;
const bool normalMaterials = true;
const bool advanceDay = false;
const bool weatherUseTake = true;

// preset colors available for painting.
const Color color_lightblue          = Color( 0.1f, 0.3f, 0.65f, 1.0f );
const Color color_yellow             = Color( 1.0f, 1.0f, 0.0f, 1.0f );
const Color color_lightgrey          = Color( 0.75f, 0.75f, 0.75f, 1.0f );
const Color color_grey               = Color( 0.50f, 0.50f, 0.50f, 1.0f );
const Color color_darkgrey           = Color( 0.25f, 0.25f, 0.25f, 1.0f );
const Color color_black              = Color( 0.0f, 0.0f, 0.0f, 1.0f );
const Color color_white_quarterClear = Color( 1.0f, 1.0f, 1.0f, 0.25f );
const Color color_white_halfClear    = Color (1.0f, 1.0f, 1.0f, 0.5f);

const Color color_white              = Color (1.0f, 1.0f, 1.0f, 1.0f);
const Color color_purple             = Color( 0.8f, 0.0f, 0.8f, 1.0f );
const Color color_orange             = Color( 1.0f, 0.8f, 0.0f, 1.0f);
const Color color_clear              = Color( 0.0f, 0.0f, 0.0f, 0.0f );
const Color color_shadow             = Color( 0.0f, 0.0f, 0.0f, 0.5f);
const Color color_defaultSeedColor   = Color( 0.75f, 0.35f, 0.1f, 1.0f );
const Color color_defaultColor       = Color( 0.35f, 0.35f, 0.35f, 1.0f );

const Color phaseTingePowder         = Color( 1.0f, 1.0f, 1.0f, 0.2f );
const Color phaseTingeLiquid         = Color( -1.0f, -1.0f, -1.0f, 0.2f );

const Color color_offwhite           = Color( 0.9f, 1.0f, 0.8f, 1.0f );
const Color color_brightred          = Color( 0.9f, 0.1f, 0.0f, 1.0f);
const Color color_darkred            = Color( 0.5f, 0.05f, 0.0f, 1.0f);
const Color color_brown              = Color(  0.25f, 0.1f, 0.0f, 1.0f );

const Color color_nightLight         = Color(1.0f, 1.0f, 1.0f, 0.3f);

// set the dimensions of the world. totalsize is used for the finely detailed grids
const unsigned int totalSize = sizeX * sizeY;

// weathergridsize is used for some grids that are scaled down for better performance
const int weatherGridSizeX = sizeX / weatherGridScale;
const int weatherGridSizeY = sizeY / weatherGridScale;
const int weatherGridSize = weatherGridSizeX * weatherGridSizeY;

// numberOfFieldsPerVertex is how many fields there are per vertex passed to opengl. it ultimately determines the size of color grids.
const unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

// these offsets can be used to navigate the grids. By choosing one and adding your index, it will give you the address of a neighbouring cell. It has to be sized to match the grid.
const int neighbourOffsets[] =
{
	- 1,
	- sizeX - 1,
	- sizeX ,
	- sizeX  + 1,
	+ 1,
	+sizeX + 1,
	+sizeX,
	+sizeX - 1
};
const int spriteNeighbourOffsets[] =
{
	- 1,
	- sizeAnimalSprite - 1,
	- sizeAnimalSprite ,
	- sizeAnimalSprite  + 1,
	+ 1,
	+sizeAnimalSprite + 1,
	+sizeAnimalSprite,
	+sizeAnimalSprite - 1
};
const int weatherGridOffsets[] =
{
	- 1,
	- weatherGridSizeX - 1,
	- weatherGridSizeX ,
	- weatherGridSizeX  + 1,
	+ 1,
	+weatherGridSizeX + 1,
	+weatherGridSizeX,
	+weatherGridSizeX - 1
};

// color grids are giant buffers of numbers that are like paintings. Other than the animation grid, they are updated incrementally, so only a small amount changes between turns. They are also literally opengl VBOs so they are rendered directly with no further processing.
float * colorGrid          = new float[totalSize * numberOfFieldsPerVertex];        // it concerns the physical material.
float * lifeColorGrid      = new float[totalSize * numberOfFieldsPerVertex];        // concerning growing plants. Because plant color information is not easily stored anywhere else, this grid must be preserved in save and load operations.
float * animationGrid      = new float[totalSize * numberOfFieldsPerVertex];        //  for the sprites of animals.
float * seedColorGrid      = new float[totalSize * numberOfFieldsPerVertex];        //  for the colors of seeds and falling photons.


const float const_tau = 6.28f;
const float const_pi  = 3.14f;


unsigned int photonsIssuedThisTurn = 0;
const unsigned int photonIdealNumber = 2 * sizeX;
// unsigned int photonChance = 1;




// PLAYER
bool firstPerson = false;
unsigned int player = 0;
unsigned int playerPosition = 0;
unsigned int playerMouseCursor = 0;




// PLANT DRAWING
unsigned int recursion_level = 0;
const unsigned int recursion_limit = 4;
unsigned int extrusion_level = 0;
const unsigned int extrusion_limit = 13;
unsigned int drawActions = 0;
const unsigned int drawActionlimit = 200;
float accumulatedRotation = (0.5 * const_pi);
float scalingFactor = 1.0f;
vec_u2 cursor_grid = vec_u2(0, 0);
vec_u2 prevCursor_grid = vec_u2(0, 0);
vec_u2 origin = vec_u2(0, 0);
unsigned int cursor_germinationMaterial = 0;
unsigned int cursor_energySource = ENERGYSOURCE_LIGHT;
unsigned int cursor_string = 0;
Color cursor_color = Color(0.1f, 0.1f, 0.1f, 1.0f);
Color cursor_seedColor = Color(0.1f, 0.1f, 0.1f, 1.0f);
float lengthNoise = 0.0f;
float energyDebtSoFar = 0.0f;

// a tree with a long stem and a fan of branches.
std::string plant_Pycad = std::string( "rqgqbqxuydzb. lz addlzf." );

// a fern with a long curling frond.
std::string plant_Lomondra = std::string( "rjgmbdxqygzd. lmskafoplhtdlh  scacldf " );

// a tall reed.
std::string plant_Worrage = std::string( "rdgqbixfygzu. shalmacclg  acclgf " );

// a fat, round stink ball.
std::string plant_MilkWombler = std::string( "rmggbmxuyuzu. cz. lz sgagofldf " );

// a mysterious urchin.
std::string plant_SpleenCoral = std::string( "rgggbuxuyuzu. cz. rmgmbw. azzlmcdf  " );

// a thick obelisk.
std::string plant_ParbasarbTree = std::string( "rgggbuxuyuzu. lzeeeflaf " );

// a short, dense grass.
std::string plant_LardGrass = std::string( "rgggbuxuyuzu. oiilfflbflbf  " );

// a blob of jelly.
std::string plant_Primordial = std::string( "rmgmbmxmymzm. lbflafe. " );

struct ProposedLifeParticle
{
	Color color = Color(0.5f, 0.5f, 0.5f, 1.0f);
	vec_u2 position = vec_u2(0, 0);
	unsigned int energySource = ENERGYSOURCE_LIGHT;

	ProposedLifeParticle(Color color, vec_u2 position, unsigned int energySource);
};

ProposedLifeParticle::ProposedLifeParticle(Color color, vec_u2 position, unsigned int energySource)
{
	this->position = position;
	this->color = color;
	this->energySource = energySource;
}





// GPLANTS "growing plants"
std::string exampleGplant = std::string("");










// ANIMAL DRAWING
std::string exampleAnimal = std::string(" uaiajbcmemjccded ");
unsigned int animalCursorFrame = FRAME_BODY;
unsigned int animalCursorString = 0;
unsigned int animalCursorX = 0;
unsigned int animalCursorY = 0;
unsigned int animalCursorExtrusionN = 0;
unsigned int animalCursorExtrusionCondition = CONDITION_GREATERTHAN;
unsigned int animalCursorSegmentNumber = 0;
unsigned int animalCursorEnergySource = ENERGYSOURCE_PLANT;
unsigned int animalRecursionLevel = 0;
unsigned int animalCursorOrgan = ORGAN_MUSCLE;
Color animalCursorColor = Color(0.5f, 0.5f, 0.5f, 1.0f);

int defaultTemperature = 300;
int defaultPressure = 1000;

unsigned int sunlightBrightness = 50;            // the amount of actual light coming from the emitter. how far it penetrates material.
unsigned int sunlightTemp = 1000;                // this is not the temperature applied to lit objects, but, the actual color temperature of the emitter.
Color sunlightColor = color_white_quarterClear;  // this is also the color of the emitter but in an easier to use form.
unsigned int sunlightDirection = 2;
float fsundirection = 0.0f;
float timeOfDay = 0.5f;

const unsigned int gasLawConstant = 2;

// raw energy values are DIVIDED by these numbers to get the result. So more means less.
const unsigned int lightEfficiency   = 1000;
const float movementEfficiency = 5000.0f;

// except for these, where the value is what you get from eating a square.
const float meatEfficiency    = 1.0f;
const float bloodEfficiency   = 1.0f;
const float seedEfficiency    = 1.0f;
const float mineralEfficiency = 1.0f;
const float plantEfficiency   = 1.0f;

// vec_u2 playerCursor = vec_u2(0, 0);


const float maximumDisplayEnergy = 1.0f;
// const float maximumDisplayTemperature = 100.0f;
const float maximumDisplayPressure = 2000.0f;
const float maximumDisplayVelocity = 100.0f;

unsigned int visualizer = VISUALIZE_MATERIAL;

unsigned int identityCursor = 0;


std::list<vec_u2> v_seeds;

unsigned int animationChangeCount = 0;
unsigned int animationGlobalFrame = FRAME_BODY;

unsigned int ppPhaseOffset = 0;
const unsigned int ppSkipSize = 10;

struct Material
{
	unsigned int crystal_n;
	unsigned int crystal_condition;
	Color color = Color(0.0f, 0.0f, 1.0f, 1.0f);
	unsigned int melting;
	unsigned int boiling;
	int insulativity;
	unsigned int availability;
	Material();
};

Material::Material()
{
	this->crystal_condition = CONDITION_GREATERTHAN;
	this->crystal_n = 2;
	this->color = color_grey;
	this->melting = 500;
	this->boiling = 1000;
	this->insulativity = 2;
}

std::vector<Material> materials;

const unsigned int MATERIAL_VACUUM = 0;

struct Particle
{
	unsigned int material;
	unsigned int phase;
	int temperature;
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
	unsigned int identity;
	float energy;
	unsigned int energySource;
	LifeParticle();
};

LifeParticle::LifeParticle()
{
	this->genes = std::string("");
	this->identity = 0x00;
	this->energy = 0.0f;
	this->energySource = ENERGYSOURCE_LIGHT;
}

struct transportableLifeParticle
{
	unsigned int identity;
	float energy;
	unsigned int energySource;
	transportableLifeParticle();
};

transportableLifeParticle::transportableLifeParticle()
{
	this->identity = 0x00;
	this->energy = 0.0f;
	this->energySource = ENERGYSOURCE_LIGHT;
}

struct SeedParticle
{
	unsigned int parentIdentity;
	std::string genes;
	float energy;
	unsigned int stage;
	unsigned int germinationMaterial;

	bool drawingInProgress;
	SeedParticle();
};
SeedParticle::SeedParticle()
{
	this->genes = std::string("");
	this->parentIdentity = 0x00;
	this->energy = 0.0f;
	this->stage = STAGE_NULL;
	this->germinationMaterial = 0;

	this->drawingInProgress = false;
}

struct transportableSeed
{
	unsigned int parentIdentity;
	float energy;
	unsigned int stage;
	transportableSeed();
};

transportableSeed::transportableSeed()
{
	this->parentIdentity = 0x00;
	this->energy = 0.0f;
	this->stage = STAGE_NULL;
}

unsigned int newIdentity ()
{
	identityCursor++;

#ifdef PLANT_DRAWING_READOUT
	printf("claimed new ID: %u\n", identityCursor);
#endif

	return identityCursor;
}

Particle *  grid = new Particle[totalSize];
LifeParticle * lifeGrid = new LifeParticle[totalSize];
SeedParticle * seedGrid = new SeedParticle[totalSize];
std::list<ProposedLifeParticle> v;
std::list<ProposedLifeParticle> v_extrudedParticles;

void clearParticle( unsigned int i)
{
	grid[i].temperature = defaultTemperature;
	grid[i].material = MATERIAL_VACUUM;
	grid[i].phase = PHASE_VACUUM;
	unsigned int a_offset = (i * numberOfFieldsPerVertex);
	memcpy( &colorGrid[ a_offset ], &color_clear, 16 );
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

void copyParticle(unsigned int from, unsigned int to)
{
	grid[to] = grid[from];
	unsigned int from_offset = (from * numberOfFieldsPerVertex);
	unsigned int to_offset = (to * numberOfFieldsPerVertex);
	memcpy( &colorGrid[ to_offset ],  &colorGrid[ from_offset ], 16 );
}

struct Weather
{
	int temperature;
	int pressure;
	int velocityX;
	int velocityY;
	int saturation;
	unsigned int lightBlockedSquares;
	unsigned int airBlockedSquares;
	int          newsaturation;
	unsigned int newlightBlockedSquares;
	unsigned int newairBlockedSquares;
	int dt;
	int dp;
	int dx;
	int dy;
	Color color; // this is where lighting information for the sim is stored, it is integrated with weather for performance reasons
	unsigned int direction;
	unsigned int velocityAbs;

	Weather();
};

Weather::Weather()
{
	this->temperature = defaultTemperature << temperatureScale;
	this->pressure = defaultPressure;
	this->velocityX = (RNG() - 0.5f) * 100;
	this->velocityY = (RNG() - 0.5f) * 100;
	this->saturation = 0;
	this->airBlockedSquares = 0;
	this->lightBlockedSquares = 0;
	this->color = color_clear;
	this->direction = 2;
	this->velocityAbs = 0;

	this->dt = 0;
	this->dp = 0;
	this->dx = 0;
	this->dy = 0;

	this-> newsaturation          = 0;
	this-> newlightBlockedSquares = 0;
	this-> newairBlockedSquares   = 0;

}

// the light grid holds lighting information. it is not drawn directly but informs what will be drawn on the color grid.
// Color lightGrid[weatherGridSize];
Color backgroundStars[totalSize];
Color backgroundSky  [totalSize];

// the weather grid holds air pressure, temperature, velocity, and solute saturation information.
Weather weatherGrid[weatherGridSize];

unsigned int calculateVelocityDirection( int velocityX,  int velocityY)
{
	// the game simplifies angle in some cases to a number in the range 0 to 7, which points to one of the 8 neighbours.
	// this algorithm uses integer comparisons to steer the direction around from a starting angle.
	unsigned int angle = 0;
	int absX = abs( velocityX  );
	int absY = abs( velocityY  );

	if (velocityX > 0)
	{
		angle += (N_NEIGHBOURS / 2);

		if (velocityY > 0)
		{
			if (absX > absY )
			{
				angle++;
				if (absX > ( absY << 1) ) // this means, if absX is more than twice the size of absY
				{
					angle++;
				}
			}
		}
		else
		{
			if (absX > absY )
			{
				angle--;
				if (absX > ( absY << 1) )
				{
					angle--;
				}
			}
		}

		angle -= 2;
		angle += (extremelyFastNumberFromZeroTo(2) - 1);
		if (angle >= N_NEIGHBOURS ) { angle = angle % N_NEIGHBOURS; }

	}
	else
	{
		angle += N_NEIGHBOURS ;
		if (velocityY > 0)
		{
			if (absX > absY )
			{
				angle++;
				if (absX > ( absY << 1) )
				{
					angle++;
				}
			}
		}
		else
		{
			if (absX > absY )
			{
				angle--;
				if (absX > ( absY << 1) )
				{
					angle--;
				}
			}
		}
		angle -= 2;
		angle += (extremelyFastNumberFromZeroTo(2) - 1);
		if (angle >= N_NEIGHBOURS ) { angle = angle % N_NEIGHBOURS; }
	}
	return angle;
}

void materialPhaseChange( unsigned int currentPosition,  float saturationDifference  )
{
	if (grid[currentPosition].phase == PHASE_SOLID)
	{
		if  (grid[currentPosition].temperature > materials[grid[currentPosition].material].boiling)
		{
			grid[currentPosition].phase = PHASE_GAS;
		}
		if  (grid[currentPosition].temperature > materials[grid[currentPosition].material].melting)
		{
			grid[currentPosition].phase = PHASE_LIQUID;
		}
	}
	else if (grid[currentPosition].phase == PHASE_POWDER)
	{
		if  (grid[currentPosition].temperature > materials[grid[currentPosition].material].melting)
		{
			grid[currentPosition].phase = PHASE_LIQUID;
		}
	}
	else if (grid[currentPosition].phase == PHASE_LIQUID)
	{
		if (grid[currentPosition].temperature > materials[grid[currentPosition].material].boiling)
		{
			grid[currentPosition].phase = PHASE_GAS;
		}
		else if (saturationDifference > 0.0f && grid[currentPosition].temperature > (materials[grid[currentPosition].material].boiling >> 1 ) )
		{
			for (unsigned int j = 0; j < N_NEIGHBOURS; ++j)
			{
				unsigned int neighbour = neighbourOffsets[j] + currentPosition;
				if (neighbour < totalSize)
				{
					if (grid[neighbour].phase == PHASE_VACUUM )
					{
						grid[currentPosition].phase = PHASE_GAS;
						break;
					}
				}
			}
		}
		else if  (grid[currentPosition].temperature < materials[grid[currentPosition].material].melting)
		{
			if  (grid[currentPosition].temperature < ( materials[grid[currentPosition].material].melting >> 1))
			{
				grid[currentPosition].phase = PHASE_POWDER;
			}

			// crystallization
			if (false)
			{
				unsigned int nSolidNeighbours = 0;
				unsigned int nAttachableNeighbours = 0;

				// go around the neighbours in order, check if they are solid. check how many in a row are solid, and whether it started from an odd or even number, indicating if it is a corner or an edge.
				unsigned int currentSolidStreak = 0;
				unsigned int longestSolidStreak = 0;
				unsigned int longestSolidStreakOffset = 0;
				unsigned int currentStreakOffset = 0;

				for (unsigned int j = 0; j < N_NEIGHBOURS; ++j)
				{
					unsigned int neighbour = neighbourOffsets[j] + currentPosition;
					if (neighbour > totalSize) {neighbour = currentPosition; }
					if (grid[neighbour].phase == PHASE_SOLID )
					{
						nSolidNeighbours++;
						currentSolidStreak++;
						if (currentSolidStreak > longestSolidStreak)
						{
							longestSolidStreak = currentSolidStreak;
							longestSolidStreakOffset = currentStreakOffset;
						}
						nAttachableNeighbours++;
					}
					else
					{
						if (grid[neighbour].phase == PHASE_LIQUID ||
						        grid[neighbour].phase == PHASE_POWDER )
						{
							nAttachableNeighbours++;
						}
						currentStreakOffset = j;
					}
				}

				if (materials[grid[currentPosition].material].crystal_condition == CONDITION_GREATERTHAN)
				{
					if (nSolidNeighbours > materials[grid[currentPosition].material].crystal_n)
					{
						if (nAttachableNeighbours > 0) {    grid[currentPosition].phase = PHASE_SOLID; }
					}
				}
				else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_EQUAL)
				{
					if (nSolidNeighbours == materials[grid[currentPosition].material].crystal_n)
					{
						if (nAttachableNeighbours > 0) {    grid[currentPosition].phase = PHASE_SOLID; }
					}
				}
				else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_LESSTHAN)
				{
					if (nSolidNeighbours < materials[grid[currentPosition].material].crystal_n)
					{
						if (nAttachableNeighbours > 0) {    grid[currentPosition].phase = PHASE_SOLID; }
					}
				}
				else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_CORNER)
				{
					if (longestSolidStreak == 3 && (longestSolidStreakOffset % 2) == 0 )
					{
						if (nAttachableNeighbours > 0) {    grid[currentPosition].phase = PHASE_SOLID; }
					}
				}
				else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_EDGE)
				{
					if (longestSolidStreak == 3 && (longestSolidStreakOffset % 2) == 1 )
					{
						if (nAttachableNeighbours > 0) {    grid[currentPosition].phase = PHASE_SOLID; }
					}
				}
				else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_ROW)
				{
					if (longestSolidStreak == materials[grid[currentPosition].material].crystal_n )
					{
						if (nAttachableNeighbours > 0) {    grid[currentPosition].phase = PHASE_SOLID; }
					}
				}
				else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_LEFTN)
				{
					if (    grid[currentPosition + (materials[grid[currentPosition].material].crystal_n) ].phase == PHASE_SOLID)
					{
						if (nAttachableNeighbours > 0) {    grid[currentPosition].phase = PHASE_SOLID; }
					}
				}
				else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_NOTLEFTRIGHTN)
				{
					if (
					    grid[currentPosition + (materials[grid[currentPosition].material].crystal_n) ].phase != PHASE_SOLID &&
					    grid[currentPosition - (materials[grid[currentPosition].material].crystal_n) ].phase != PHASE_SOLID
					)
					{
						if (nAttachableNeighbours > 0) {    grid[currentPosition].phase = PHASE_SOLID; }
					}
					else
					{
						grid[currentPosition].phase = PHASE_LIQUID;
					}
				}

				else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_NOTLRNEIGHBOURS)
				{
					if (currentPosition > sizeX + 40 && currentPosition < (totalSize - sizeX - 40))
					{
						if (
						    grid[currentPosition           + (materials[ grid[currentPosition].material].crystal_n * 2 )].phase != PHASE_SOLID &&
						    grid[currentPosition           - (materials[ grid[currentPosition].material].crystal_n * 2 )].phase != PHASE_SOLID &&
						    grid[currentPosition + (sizeX) + (materials[ grid[currentPosition].material].crystal_n * 2 )].phase != PHASE_SOLID &&
						    grid[currentPosition + (sizeX) - (materials[ grid[currentPosition].material].crystal_n * 2 )].phase != PHASE_SOLID &&
						    grid[currentPosition - (sizeX) + (materials[ grid[currentPosition].material].crystal_n * 2 )].phase != PHASE_SOLID &&
						    grid[currentPosition - (sizeX) - (materials[ grid[currentPosition].material].crystal_n * 2 )].phase != PHASE_SOLID
						)
						{
							if (nAttachableNeighbours > 0) {    grid[currentPosition].phase = PHASE_SOLID; }
						}
						else
						{
							grid[currentPosition].phase = PHASE_LIQUID;
						}
					}
				}

				// small chance to solidify at random and form the nucleus of a new crystal.
				if (extremelyFastNumberFromZeroTo(10000) == 0)
				{
					grid[currentPosition].phase = PHASE_SOLID;
				}
			}
		}
	}
	else if (grid[currentPosition].phase == PHASE_GAS)
	{

		if (saturationDifference < 0.0f )
		{
			if (grid[currentPosition].temperature < materials[grid[currentPosition].material].boiling)
			{
				grid[currentPosition].phase = PHASE_LIQUID;
			}
		}
	}
}

void conductHeat(unsigned int currentPosition, unsigned int neighbour)
{
	// if (grid[neighbour].phase !=)
	// Exchange heat by conduction.
	if (grid[neighbour].phase != PHASE_VACUUM)
	{
		int avgTemp = (((grid[currentPosition].temperature ) - (grid[neighbour].temperature)) ) ;
		avgTemp = avgTemp >> materials[ grid[neighbour].material ].insulativity;
		grid[neighbour].temperature += avgTemp;
		grid[currentPosition].temperature -= avgTemp;
	}
}

// The classic falling sand physics thread.
void materialPhysics (unsigned int currentPosition, unsigned int weatherGridI)
{
	unsigned int random = extremelyFastNumberFromZeroTo(N_NEIGHBOURS);

	// movement instructions for GASES
	if (grid[currentPosition].phase == PHASE_GAS)
	{
		unsigned int neighbour = neighbourOffsets[ weatherGrid[weatherGridI].direction ] + currentPosition;
		if (neighbour >= totalSize) {neighbour = currentPosition;}
		conductHeat(currentPosition, neighbour);
		if (grid[neighbour].phase  == PHASE_VACUUM || (grid[neighbour].phase == PHASE_GAS) )
		{
			swapParticle(currentPosition, neighbour);
		}
	}

	// movement instructions for LIQUIDS
	else if (grid[currentPosition].phase == PHASE_LIQUID)
	{
		unsigned int neighbour = neighbourOffsets[ (  0 +  (random >> 1) )  ] + currentPosition;
		if (weatherGrid[weatherGridI].velocityAbs > 10000)
		{
			neighbour = neighbourOffsets[ weatherGrid[weatherGridI].direction  ] + currentPosition;
		}
		if (neighbour >= totalSize) {neighbour = currentPosition;}
		conductHeat(currentPosition, neighbour);
		if ((    grid[neighbour].phase == PHASE_VACUUM) ||
		        (grid[neighbour].phase == PHASE_GAS)    ||
		        (grid[neighbour].phase == PHASE_LIQUID)     )
		{
			swapParticle(currentPosition, neighbour);
			currentPosition = neighbour;
		}
	}

	// movement instructions for POWDERS
	else if (grid[currentPosition].phase  == PHASE_POWDER)
	{
		unsigned int neighbour = neighbourOffsets[ (  1 +  (random >> 2) )  ] + currentPosition;
		if (weatherGrid[weatherGridI].velocityAbs > 20000 )
		{
			neighbour = neighbourOffsets[ weatherGrid[weatherGridI].direction  ] + currentPosition;
		}
		if (neighbour >= totalSize) {neighbour = currentPosition;}
		conductHeat(currentPosition, neighbour);
		if ((    grid[neighbour].phase == PHASE_VACUUM) ||
		        (grid[neighbour].phase == PHASE_GAS)    ||
		        (grid[neighbour].phase == PHASE_LIQUID)  )
		{
			swapParticle(currentPosition, neighbour);
			currentPosition = neighbour;
		}
	}

	// movement instructions for SOLIDS
	else if (grid[currentPosition].phase  == PHASE_SOLID)
	{
		unsigned int neighbour = neighbourOffsets[ random ] + currentPosition;
		if (neighbour >= totalSize) {neighbour = currentPosition;}
		conductHeat(currentPosition, neighbour);
		if (weatherGrid[weatherGridI].velocityAbs > 50000)
		{
			neighbour = neighbourOffsets[ weatherGrid[weatherGridI].direction  ] + currentPosition;
			if (neighbour >= totalSize) {neighbour = currentPosition;}
			swapParticle(currentPosition, neighbour);
			currentPosition = neighbour;
		}
	}
}

// boundary conditions are one of the trickiest parts to get right.
// this one simply mirrors the conditions of the cells closest the edge, making the edge itself seem like a one-way trip into a wider sky.
void airflowEdge(  unsigned int weatherGridI , unsigned int weatherGridX, unsigned int weatherGridY)
{

	// smooth the simulation by mixing each cell with the average of its neighbours.
	int ap = (weatherGrid[ weatherGridI].pressure   ) ;
	int ax = (weatherGrid[ weatherGridI].velocityX  ) ;
	int ay = (weatherGrid[ weatherGridI].velocityY  ) ;
	int at = (weatherGrid[ weatherGridI].temperature) ;
	int count = 1;
	for ( int cy = -1; cy < 2; ++cy)
	{
		for ( int cx = -1; cx < 2; ++cx)
		{
			int ix = cx + weatherGridX;
			int iy = cy + weatherGridY;

			if (ix >= weatherGridSizeX || ix < 0) { continue; }
			if (iy >= weatherGridSizeY || iy < 0) { continue; }

			unsigned int weatherGridNeighbour = (iy * weatherGridSizeX) + ix;

			ap += (weatherGrid[ weatherGridNeighbour ].pressure   ) ;
			ax += (weatherGrid[ weatherGridNeighbour ].velocityX  ) ;
			ay += (weatherGrid[ weatherGridNeighbour ].velocityY  ) ;
			at += (weatherGrid[ weatherGridNeighbour ].temperature) ;
			count++;
		}
	}

	ax = ax / count ;                                                                                              // N_NEIGHBOURS is 8, so you can do the division part of the average by using a bit shift.
	ay = ay / count ;
	ap = ap / count ;
	at = at / count ;

	weatherGrid[weatherGridI].pressure    = ap;
	weatherGrid[weatherGridI].velocityX   = ax;
	weatherGrid[weatherGridI].velocityY   = ay;
	weatherGrid[weatherGridI].temperature = at;

	weatherGrid[weatherGridI].direction = calculateVelocityDirection( weatherGrid[weatherGridI]. velocityX,  weatherGrid[weatherGridI]. velocityY);
	weatherGrid[weatherGridI].velocityAbs = abs(weatherGrid[weatherGridI]. velocityX) +  abs( weatherGrid[weatherGridI]. velocityY);
}

void applyAirflowChanges(unsigned int weatherGridI)
{
	// cancel just a little bit of velocity, because the sim is unstable if it's exactly 1. Cancel more if there is stuff in the way.
	weatherGrid[weatherGridI].velocityX  -= weatherGrid[weatherGridI].velocityX >> 12 ;// *= 0.9999;
	weatherGrid[weatherGridI].velocityY  -= weatherGrid[weatherGridI].velocityY >> 12 ;// *= 0.9999;

	weatherGrid[weatherGridI].pressure    += (  defaultPressure                        - weatherGrid[weatherGridI].pressure   ) >> 7 ;//* 0.01;
	weatherGrid[weatherGridI].temperature += ( (defaultTemperature << temperatureScale) - weatherGrid[weatherGridI].temperature)  >> 9 ;//* 0.001;

	weatherGrid[weatherGridI].pressure    += weatherGrid[weatherGridI]. dp   ;
	weatherGrid[weatherGridI].velocityX   += weatherGrid[weatherGridI]. dx   ;
	weatherGrid[weatherGridI].velocityY   += weatherGrid[weatherGridI]. dy   ;

	weatherGrid[weatherGridI].direction = calculateVelocityDirection( weatherGrid[weatherGridI]. velocityX,  weatherGrid[weatherGridI]. velocityY);
	weatherGrid[weatherGridI].velocityAbs = abs(weatherGrid[weatherGridI]. velocityX) +  abs( weatherGrid[weatherGridI]. velocityY);

	weatherGrid[weatherGridI].temperature += weatherGrid[weatherGridI]. dt   ;

	weatherGrid[weatherGridI]. dp = 0;
	weatherGrid[weatherGridI]. dx = 0;
	weatherGrid[weatherGridI]. dy = 0;
	weatherGrid[weatherGridI]. dt = 0;
}

void airflow( unsigned int weatherGridI)
{


	// make sure these are never less than 1! the only reason they are signed integers is so you have a chance to check, instead of suffering an immediate failure.
	if (weatherGrid[weatherGridI].temperature  < 1) {weatherGrid[weatherGridI].temperature = 1;}
	if (weatherGrid[weatherGridI].pressure  < 1)    {weatherGrid[weatherGridI].pressure = 1;}


	if (weatherGrid[weatherGridI].airBlockedSquares < (weatherGridScale * weatherGridScale))
	{
		// smooth the simulation by mixing each cell with the average of its neighbours.
		int ap = 0;
		int ax = 0;
		int ay = 0;
		int at = 0;
		for (unsigned int n = 0; n < N_NEIGHBOURS; ++n)
		{
			unsigned int weatherGridNeighbour = (weatherGridI + weatherGridOffsets[n] )  ;//% weatherGridSize;
			if (weatherGridNeighbour > weatherGridSize) {weatherGridNeighbour = weatherGridI;}
			ap += (weatherGrid[ weatherGridNeighbour ].pressure   ) ;
			ax += (weatherGrid[ weatherGridNeighbour ].velocityX  ) ;
			ay += (weatherGrid[ weatherGridNeighbour ].velocityY  ) ;
			at += (weatherGrid[ weatherGridNeighbour ].temperature) ;
		}
		ax = ax >> 3;                                                                                              // N_NEIGHBOURS is 8, so you can do the division part of the average by using a bit shift.
		ay = ay >> 3;
		ap = ap >> 3;
		at = at >> 3;

		weatherGrid[weatherGridI]. dp +=   (ap - weatherGrid[weatherGridI].pressure   ) >> 3;
		weatherGrid[weatherGridI]. dx +=   (ax - weatherGrid[weatherGridI].velocityX  ) >> 3;
		weatherGrid[weatherGridI]. dy +=   (ay - weatherGrid[weatherGridI].velocityY  ) >> 3;
		weatherGrid[weatherGridI]. dt +=   (at - weatherGrid[weatherGridI].temperature) >> 3;

		// pt interchange
		int dtp = (weatherGrid[weatherGridI].dt - weatherGrid[weatherGridI].dp) >> gasLawConstant;
		weatherGrid[weatherGridI].dp -= dtp ;
		weatherGrid[weatherGridI].dt += dtp;

		// pv interchange
		const unsigned int baseBlockageRatio = 3;
		for (unsigned int direction = 0; direction < N_NEIGHBOURS; direction += 2)
		{
			unsigned int neighbour = weatherGridI + weatherGridOffsets[direction];
			int sign = 1; if (neighbour < weatherGridI) {sign = -1;}
			if (neighbour >= weatherGridSize) { continue; } //neighbour = weatherGridI; }
			if (direction == 0 || direction == 4)
			{
				weatherGrid[weatherGridI].dp += ((sign * (weatherGrid[ neighbour ].velocityX - weatherGrid[ weatherGridI ].velocityX )) >> (baseBlockageRatio) )   ; // A difference in speed creates pressure.
				weatherGrid[weatherGridI].dx += ((sign * (weatherGrid[ neighbour ].pressure  - weatherGrid[ weatherGridI ].pressure  )) >> (baseBlockageRatio) )   ; // A difference in pressure creates movement.
			}

			else if (direction == 2 || direction == 6)
			{
				weatherGrid[weatherGridI].dp += ((sign * (weatherGrid[ neighbour ].velocityY - weatherGrid[ weatherGridI ].velocityY )) >> (baseBlockageRatio))  ;
				weatherGrid[weatherGridI].dy += ((sign * (weatherGrid[ neighbour ].pressure  - weatherGrid[ weatherGridI ].pressure  )) >> (baseBlockageRatio))  ;
			}
		}

		// mix heat and velocity from far away. This is a key component of turbulent behavior in the sim, and produces a billowing effect that looks very realistic. It is prone to great instability.
		if (weatherUseTake)
		{
			int takeX = weatherGrid[weatherGridI].dx >> 1;    // strong mixing here allows the sim to make stunning clouds and air currents, but radiating shockwaves look better when the take component is smaller.
			int takeY = weatherGrid[weatherGridI].dy >> 1;

			unsigned int weatherGridX = weatherGridI % weatherGridSizeX;
			unsigned int weatherGridY = weatherGridI / weatherGridSizeX;

			takeX = weatherGridX + takeX  ;                                                                           // the velocity itself is used to find the grid location to take from.
			takeY = weatherGridY + takeY ;                                                          // velocity numbers range greatly and can be very high, use this number to scale them to an appropriate take distance.

			if (takeY >= 0 && takeY < weatherGridSizeY)
			{
				int takeI = ((takeY * weatherGridSizeX) + takeX );
				if (takeI >= 0 && takeI < weatherGridSize )
				{



					int takeTemperature = (( weatherGrid[weatherGridI].temperature - weatherGrid[takeI].temperature ) >> (2  ) );
					int takeVelocityX   = (( weatherGrid[weatherGridI].velocityX   - weatherGrid[takeI].velocityX   ) >> (3  ) );
					int takeVelocityY   = (( weatherGrid[weatherGridI].velocityY   - weatherGrid[takeI].velocityY   ) >> (3  ) );

					// weatherGrid[takeI].dt += takeTemperature;
					// weatherGrid[takeI].dx += takeVelocityX;                // mix in the velocity contribution from far-away.
					// weatherGrid[takeI].dy += takeVelocityY;                // adding more looks cool, but makes the fluid explode on touch like nitroglycerin!

					weatherGrid[weatherGridI].dt -= takeTemperature;
					weatherGrid[weatherGridI].dx -= takeVelocityX;                // mix in the velocity contribution from far-away.
					weatherGrid[weatherGridI].dy -= takeVelocityY;                // adding more looks cool, but makes the fluid explode on touch like nitroglycerin!
				}
			}
		}
	}
}

void floatPhoton( unsigned int weatherGridI ,  Color lightColor,  float lightBrightness,  float lightDirection)
{
	// if (extremelyFastNumberFromZeroTo( photonChance ) == 0 )
	// {

	float blocked = 0.0f;
	float positionX = weatherGridI % weatherGridSizeX;
	float positionY = weatherGridI / weatherGridSizeX;
	unsigned int wx = positionX;
	unsigned int wy = positionY;
	weatherGridI =  (wy * weatherGridSizeX) + wx ;

	float noisyLength = 1.3f           + ((RNG() - 0.5f) * 0.2f) ;                               // the idea is to put some noise to avoid aliasing which creates stripes of light and dark in evenly lit material
	float noisyAngle  = lightDirection + ((RNG() - 0.5f) * 0.2f) ;

	float incrementX = (noisyLength * cos(noisyAngle));
	float incrementY = (noisyLength * sin(noisyAngle)) ;

	int ibrightness = lightBrightness;
	weatherGrid[weatherGridI].temperature -= ibrightness;

	photonsIssuedThisTurn ++;

	while (true)
	{
		positionX += incrementX;
		positionY += incrementY;
		wx = positionX;
		wy = positionY;

		if (wx == 0 || wy == 0 || wx >= weatherGridSizeX - 1 || wy >= weatherGridSizeY - 1) {return;} // stop when you get to the edge of the map

		weatherGridI =  (wy * weatherGridSizeX) + wx ;

		float appliedEnergy;//  = energy/100;
		if (weatherGridI >= weatherGridSize) {return;}

		float energy  = (lightBrightness - blocked );
		if (energy < 0.0f)
		{
			return;
		}
		else
		{
			Color appliedColor = lightColor;

			float appliedEnergy  = energy / 1000;
			appliedColor.a = appliedEnergy  ;

			weatherGrid[weatherGridI].color = addColor(weatherGrid[weatherGridI].color, appliedColor);

		}
		int amountToBlock  = weatherGrid[weatherGridI].lightBlockedSquares + weatherGrid[weatherGridI].saturation;
		blocked += amountToBlock ;
		weatherGrid[weatherGridI].temperature += amountToBlock * ( appliedEnergy); // ( () -  weatherGrid[weatherGridI].temperature  );
	}
}

void darkenLightfield(unsigned int weatherGridI )
{
	// darken the light field over time. This is crucial to refresh it without constantly redrawing the whole thing.
	weatherGrid[weatherGridI].color.r += (color_nightLight.r - weatherGrid[weatherGridI].color.r) * 0.2f ;
	weatherGrid[weatherGridI].color.g += (color_nightLight.g - weatherGrid[weatherGridI].color.g) * 0.2f ;
	weatherGrid[weatherGridI].color.b += (color_nightLight.b - weatherGrid[weatherGridI].color.b) * 0.2f ;
	weatherGrid[weatherGridI].color.a += (color_nightLight.a - weatherGrid[weatherGridI].color.a) * 0.2f ;
}

const Color color_blackbody_772      = Color(0.16f, 0.0f, 0.0f, 0.03f);
const Color color_blackbody_852      = Color(0.33f, 0.0f, 0.0f, 0.11f);
const Color color_blackbody_908   = Color(0.5f, 0.0f, 0.0f, 0.20f);

Color blackbodyLookup(  int temperature )
{
	if (temperature > 0 && temperature < 600 )
	{
		return color_clear;;
	}
	else if (temperature < 772) // faint red
	{
		return color_blackbody_772;
	}
	else if (temperature < 852) // blood red
	{
		return  color_blackbody_852;
	}
	else if (temperature < 908) // dark cherry
	{
		return color_blackbody_908;
	}
	else if (temperature < 963) // medium cherry
	{
		return Color(0.66f, 0.0f, 0.0f, 0.27f);
	}
	else if (temperature < 1019) // cherry
	{
		return Color(0.833f, 0.0f, 0.0f, 0.36f);
	}
	else if (temperature < 1060) // bright cherry
	{
		return  Color(1.0f, 0.0f, 0.0f, 0.45f);
	}
	else if (temperature < 1116) // salmon (??)
	{
		return  Color(1.0f, 0.25f, 0.0f, 0.53f);
	}
	else if (temperature < 1188) // dark orange
	{
		return Color(1.0f, 0.5f, 0.0f, 0.61f);
	}
	else if (temperature < 1213) // orange
	{
		return Color(1.0f, 0.75f, 0.0f, 0.70f);
	}
	else if (temperature < 1272) // lemon
	{
		return Color(1.0f, 1.0f, 0.0f, 0.78f);
	}
	else if (temperature < 1352) // light yellow
	{
		return  Color(1.0f, 1.0f, 0.5f, 0.86f);
	}
	else if (temperature < 5000) // white
	{
		return  Color(1.0f, 1.0f, 1.0f, 0.91f);
	}
	else if (temperature < 10000) // cool white
	{
		return  Color(0.95f, 0.95f, 1.0f, 1.0f);
	}
	else // blue
	{
		return Color(0.9f, 0.9f, 1.0f, 1.0f);
	}
}


// updates a location on the color grid with a material's new color information, based on phase and temperature.
void materialPostProcess(unsigned int i, unsigned int weatherGridI, float saturationLimit)
{
	if (grid[i].material  < materials.size() && weatherGridI < weatherGridSize)
	{
		unsigned int b_offset = i * numberOfFieldsPerVertex;
		Color ppColor = color_clear;

		// then, we will do everything which reflects light, but does not emit it.
		Color materialColor = materials[ grid[i].material ].color ;

		// different shades for material phases, a nice visual detail
		if (grid[i].phase != PHASE_GAS)
		{
			if (grid[i].phase == PHASE_LIQUID)
			{	materialColor = addColor(materialColor, phaseTingeLiquid);
				materialColor.a = 0.65f;
			}
			else if (grid[i].phase == PHASE_POWDER)
			{
				materialColor = addColor(materialColor, phaseTingePowder);
			}
			ppColor = filterColor(ppColor, materialColor);
		}

		// paint in the living plants and their seeds.
		Color seed_color = color_clear;
		Color life_color;
		if (seedGrid[i].stage != STAGE_NULL)
		{
			memcpy( &seed_color, &seedColorGrid[ b_offset ] , sizeof(Color) );
		}
		memcpy( &life_color, &lifeColorGrid[ b_offset ] , sizeof(Color) );
		life_color = filterColor( life_color, seed_color );
		ppColor = filterColor( ppColor, life_color );

		// paint clouds. If there is a lot of gas in an area.
		if (weatherGrid[weatherGridI].saturation > 0  )
		{
			Color cloudTinge = color_clear;
			cloudTinge = addColor(materialColor, color_white_halfClear);
			float fsat = weatherGrid[weatherGridI].saturation ;
			float flim = saturationLimit;
			cloudTinge.a = (fsat / flim )  ;
			cloudTinge = clampColor(cloudTinge);
			ppColor = filterColor(    ppColor , cloudTinge );
		}

		// diffuse drawing now complete.
		ppColor = multiplyColor(ppColor, weatherGrid[weatherGridI].color);

		// now we will do the things that emit light.

		// Make hot stuff glow
		if (grid[i].phase != PHASE_VACUUM)
		{
			ppColor = addColor( ppColor, blackbodyLookup( (grid[i].temperature) ) );
		}
		ppColor = addColor(ppColor, blackbodyLookup( (weatherGrid[weatherGridI].temperature >> temperatureScale  ) ));

		// shine some background stars.
		if (grid[i].phase != PHASE_SOLID && grid[i].phase != PHASE_POWDER )
		{
			Color star = backgroundStars[i];
			ppColor = filterColor(star, ppColor);
		}

		// apply changes.
		colorGrid[ b_offset + 0 ] = ppColor.r;
		colorGrid[ b_offset + 1 ] = ppColor.g;
		colorGrid[ b_offset + 2 ] = ppColor.b;
		colorGrid[ b_offset + 3 ] = ppColor.a;
	}
}

void materialHeatGlow(unsigned int i, unsigned int weatherGridI)
{
	if (grid[i].temperature > 600 )
	{
		bool edge = false;
		unsigned int randomDirection = extremelyFastNumberFromZeroTo(N_NEIGHBOURS);
		unsigned int neighbour = i;
		for (int j = 0; j < N_NEIGHBOURS; ++j)
		{
			randomDirection ++;
			randomDirection = randomDirection % N_NEIGHBOURS;

			neighbour  = (i + neighbourOffsets[ randomDirection ]);
			if (neighbour < totalSize)
			{
				if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_LIQUID || grid[neighbour].phase == PHASE_GAS)  // but only if it is on the edge of a material, not inside the bulk (which is already painted with glow in a cheaper way)
				{
					edge = true;
					break;
				}
			}
		}
		if (edge)
		{
			float radiantLightIntensity = ((grid[i].temperature - 600) >> 4);
			float fdirection = randomDirection;
			fdirection = ((fdirection / N_NEIGHBOURS) * (const_tau)) - const_pi; // radiate in the empty direction. this converts 1-to-8 to radians.
			fdirection += (RNG() - 0.5f) * (0.75f);                         // add up to 1/8th of a full circles worth of direction noise.. 1/8 of a circle is 0.75 radians
			floatPhoton(weatherGridI, blackbodyLookup(grid[i].temperature) , radiantLightIntensity, fdirection);
		}
	}
}

void weatherHeatGlow(unsigned int weatherGridI)
{
	if (weatherGrid[weatherGridI].temperature > (600 << temperatureScale) )
	{
		float radiantLightIntensity = (((weatherGrid[weatherGridI].temperature >> temperatureScale) - 600) >> 4);
		float fdirection = RNG() * const_tau;
		floatPhoton(weatherGridI, blackbodyLookup(weatherGrid[weatherGridI].temperature) , radiantLightIntensity, fdirection);
	}
}

void thread_sector( unsigned int from, unsigned int to )
{
	unsigned int fromX = from % sizeX;
	unsigned int fromY = from / sizeX;
	unsigned int weatherFromX = fromX % weatherGridScale;
	unsigned int weatherFromY = fromY / weatherGridScale;
	unsigned int weatherFromI = ( (weatherFromY) * weatherGridSizeX ) + (weatherFromX) ;

	unsigned int toX = to % sizeX;
	unsigned int toY = to / sizeX;
	unsigned int weatherToX = toX % weatherGridScale;
	unsigned int weatherToY = toY / weatherGridScale;
	unsigned int weatherToI = ( (weatherToY) * weatherGridSizeX ) + (weatherToX) ;

	// for the airflow simulation, detect edges and execute them in advance, so normal run is not interrupted.
	// cast sunlight from the edges.
	if (fromY == 0)
	{
		fromY = 1;
	}

	if (toY >= weatherGridSizeY - 1)
	{
		toY = weatherGridSizeY - 2;
	}

	// iterate through selected WEATHER cells
#ifdef DETAIL_TIMING_READOUT
	auto start1 = std::chrono::steady_clock::now();
#endif

	for (unsigned int weatherGridI = weatherFromI; weatherGridI <= weatherToI; ++weatherGridI)
	{
		airflow(weatherGridI);
		darkenLightfield( weatherGridI );
	}
#ifdef DETAIL_TIMING_READOUT
	auto end1 = std::chrono::steady_clock::now();
	auto elapsed1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
	std::cout << "subthread weather 1 " << elapsed1.count() << " microseconds." << std::endl;
#endif





	// iterate through selected WEATHER cells
#ifdef DETAIL_TIMING_READOUT
	auto start9 = std::chrono::steady_clock::now();
#endif


	for (unsigned int weatherGridI = weatherFromI + ppPhaseOffset; weatherGridI < weatherToI; weatherGridI += ppSkipSize)
	{
		if (photonsIssuedThisTurn < photonIdealNumber)
		{
			weatherHeatGlow(weatherGridI);
		}
		weatherGrid[weatherGridI].saturation = weatherGrid[weatherGridI].newsaturation;
		weatherGrid[weatherGridI].lightBlockedSquares = weatherGrid[weatherGridI].newlightBlockedSquares;
		weatherGrid[weatherGridI].airBlockedSquares = weatherGrid[weatherGridI].newairBlockedSquares;


		weatherGrid[weatherGridI].newsaturation          = 0;
		weatherGrid[weatherGridI].newlightBlockedSquares = 0;
		weatherGrid[weatherGridI].newairBlockedSquares   = 0 ;

	}
#ifdef DETAIL_TIMING_READOUT
	auto end9 = std::chrono::steady_clock::now();
	auto elapsed9 = std::chrono::duration_cast<std::chrono::microseconds>(end9 - start9);
	std::cout << "subthread fine weather 9 " << elapsed9.count() << " microseconds." << std::endl;
#endif









#ifdef DETAIL_TIMING_READOUT
	auto start2 = std::chrono::steady_clock::now();
#endif


	// iterate over the weather grid parts again to apply the updated changes. Doing this separately to the calculation is vital to the detail and beauty of the simulation.
	for (unsigned int weatherGridI = weatherFromI; weatherGridI < weatherToI; ++weatherGridI)
	{
		applyAirflowChanges( weatherGridI);
	}

#ifdef DETAIL_TIMING_READOUT
	auto end2 = std::chrono::steady_clock::now();
	auto elapsed2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
	std::cout << "subthread airflowchanges 2 " << elapsed2.count() << " microseconds." << std::endl;
#endif




#ifdef DETAIL_TIMING_READOUT
	auto start3 = std::chrono::steady_clock::now();
#endif



//  COARSE iterate through cells in the range from..to
	for (unsigned int currentPosition = from; currentPosition < to; ++currentPosition)
	{
		// calculate indexes
		unsigned int x = currentPosition % sizeX;
		unsigned int y = currentPosition / sizeX;
		// unsigned int currentPosition = (y * sizeX) + x;
		unsigned int weatherGridX = x / weatherGridScale;
		unsigned int weatherGridY = y / weatherGridScale;
		unsigned int weatherGridI = (weatherGridY * weatherGridSizeX) + weatherGridX;


		if (weatherGridI < weatherGridSize)
		{
			// float saturationLimit = 0.0f;
			if (grid[currentPosition].phase != PHASE_VACUUM)
			{
				// iterate through grid cells. collect saturation information for each weather cell that you are in.

				// when the temperature goes down, the amount of water that the air can hold decreases.
				// when the temperature goes up, the amount of water that the air can hold increases, until the water boils into gas and mixes with the air completely.
				// saturation limit = ((difference between current temp and melting) / (total liquid temp range)
				// * (amount of grid cells per weather cell)) / (ratio of current pressure to default pressure)
				// produces the amount of cells of a weather grid square that the air can absorb at the current temperature and pressure
				// float adjustedMeltTemp = materials[grid[currentPosition].material].melting * temperatureScale ;
				// float adjustedBoilTemp = materials[grid[currentPosition].material].boiling * temperatureScale ;
				// saturationLimit =
				//     ((weatherGrid[weatherGridI].temperature - adjustedMeltTemp )
				//      / (adjustedBoilTemp - adjustedMeltTemp) );
				// saturationLimit *= (weatherGridScale * weatherGridScale);
				// saturationLimit = saturationLimit / (weatherGrid[weatherGridI].pressure / 1000);

				materialPhysics( currentPosition , weatherGridI);


			}


		}
	}


#ifdef DETAIL_TIMING_READOUT
	auto end3 = std::chrono::steady_clock::now();
	auto elapsed3 = std::chrono::duration_cast<std::chrono::microseconds>(end3 - start3);
	std::cout << "subthread coarse grid 3 " << elapsed3.count() << " microseconds." << std::endl;
#endif



#ifdef DETAIL_TIMING_READOUT
	auto start4 = std::chrono::steady_clock::now();
#endif

	//FINE iterate through selected cells
	for (unsigned int currentPosition = (from + ppPhaseOffset); currentPosition < to; currentPosition += ppSkipSize)
	{
		// calculate indexes
		unsigned int x = currentPosition % sizeX;
		unsigned int y = currentPosition / sizeX;
		unsigned int weatherGridX = x / weatherGridScale;
		unsigned int weatherGridY = y / weatherGridScale;
		unsigned int weatherGridI = (weatherGridY * weatherGridSizeX) + weatherGridX;



		if (weatherGridI < weatherGridSize)
		{
			const int saturationLimit = 1;

			if (photonsIssuedThisTurn < photonIdealNumber)
			{
				materialHeatGlow(    currentPosition, weatherGridI);
			}
			materialPhaseChange( currentPosition,   saturationLimit - weatherGrid[weatherGridI].saturation  );
			materialPostProcess( currentPosition, weatherGridI, saturationLimit);

			// couple the material grid temp to the weather grid temp
			if (grid[currentPosition].phase != PHASE_VACUUM)
			{
				int gridCouplingAmount = ( weatherGrid[weatherGridI].temperature  - (grid[currentPosition].temperature << temperatureScale) ) ;
				const unsigned int heatCouplingConstant = 4;
				grid[currentPosition].temperature += (gridCouplingAmount >> temperatureScale)  >> heatCouplingConstant ;
				weatherGrid[weatherGridI].temperature -= (gridCouplingAmount) >> heatCouplingConstant ;


				if (grid[currentPosition].phase == PHASE_GAS )
				{
					weatherGrid[weatherGridI].newsaturation++;
				}

				else if (grid[currentPosition].phase == PHASE_SOLID || grid[currentPosition].phase ==  PHASE_POWDER )
				{
					weatherGrid[weatherGridI].newlightBlockedSquares++;
					weatherGrid[weatherGridI].newairBlockedSquares++;
				}

				else if (grid[currentPosition].phase ==  PHASE_LIQUID )
				{
					weatherGrid[weatherGridI].newairBlockedSquares++;
				}
			}
		}
	}

#ifdef DETAIL_TIMING_READOUT
	auto end4 = std::chrono::steady_clock::now();
	auto elapsed4 = std::chrono::duration_cast<std::chrono::microseconds>(end4 - start4);
	std::cout << "subthread fineweather 4 " << elapsed4.count() << " microseconds." << std::endl;
#endif


}

void updateDaytime()
{
	if (advanceDay)
	{
		timeOfDay += 0.01f;
	}
	fsundirection =  1.5 * const_pi + (sin(timeOfDay) ) ;
	int effectiveTemp = sunlightTemp - (sunlightTemp * abs (sin(timeOfDay)) );
	sunlightColor = blackbodyLookup(effectiveTemp);

}




void thread_handleEdges()
{



#ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
#endif

	unsigned int fromY = 0;
	for (unsigned int weatherGridX = 0; weatherGridX < weatherGridSizeX; ++weatherGridX)
	{

		unsigned int weatherGridI = (fromY * weatherGridSizeX) + weatherGridX;
		// airflowEdge(weatherGridI, weatherGridX, fromY);
	}

	unsigned int toY = weatherGridSizeY - 1;
	for (unsigned int weatherGridX = 0; weatherGridX < weatherGridSizeX; ++weatherGridX)
	{
		unsigned int weatherGridI = (toY * weatherGridSizeX) + weatherGridX;
		airflowEdge(weatherGridI, weatherGridX, toY);
		floatPhoton(weatherGridI, sunlightColor, sunlightBrightness, fsundirection);
	}

	// unsigned int toY = weatherGridSizeY - 1;
// 	for (unsigned int currentPosition = (totalSize-sizeX); currentPosition < totalSize; ++currentPosition)
// 	{

// 		unsigned int x = currentPosition % sizeX;
// 		unsigned int y = currentPosition / sizeX;
// 		unsigned int weatherGridX = x / weatherGridScale;
// 		unsigned int weatherGridY = y / weatherGridScale;
// 		unsigned int weatherGridI = (weatherGridY * weatherGridSizeX) + weatherGridX;

// if ( (currentPosition+sizeX+1) >= totalSize || (weatherGridI+weatherGridSizeX+1) >= weatherGridSize) {continue;}
// 					materialPhysics( currentPosition , weatherGridI);


// 	}



#ifdef THREAD_TIMING_READOUT
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_handleEdges " << elapsed.count() << " microseconds." << std::endl;
#endif




}



// this thread is run once per frame.
void thread_master()
{
	seedExtremelyFastNumberGenerators(); // make sure randomness is really random
	updateDaytime();


	photonsIssuedThisTurn = 0;

#ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
#endif




	boost::thread t1{  thread_sector, 0                    , (1 * (totalSize / 8))  } ;
	boost::thread t2{  thread_sector, 1 * (totalSize / 8)  , (2 * (totalSize / 8))  } ;
	boost::thread t3{  thread_sector, 2 * (totalSize / 8)  , (3 * (totalSize / 8))  } ;
	boost::thread t4{  thread_sector, 3 * (totalSize / 8)  , (4 * (totalSize / 8))  } ;
	boost::thread t5{  thread_sector, 4 * (totalSize / 8)  , (5 * (totalSize / 8))  } ;
	boost::thread t6{  thread_sector, 5 * (totalSize / 8)  , (6 * (totalSize / 8))  } ;
	boost::thread t7{  thread_sector, 6 * (totalSize / 8)  , (7 * (totalSize / 8))  } ;
	boost::thread t8{  thread_sector, 7 * (totalSize / 8)  , ((totalSize )       )  } ;


	boost::thread t12 { thread_seeds };
	boost::thread t11 { thread_life};

	boost::thread t811{  thread_handleEdges } ;





	t811.join();


	t11.join();

	t12.join();



	t8.join();

	t7.join();

	t6.join();

	t5.join();

	t4.join();

	t3.join();

	t2.join();

	t1.join();

#ifdef THREAD_TIMING_READOUT
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_sector x8 " << elapsed.count() << " microseconds." << std::endl;
#endif








	// if too many photons were issued, the photon chance should be lowered.
	// if (photonIdealNumber > 0)
	// {
	// photonChance =  ((  photonsIssuedThisTurn / photonIdealNumber ) * 5);
	// }
	// printf("photonsIssuedThisTurn %u , photonChance %u , photonIdealNumber %u \n", photonsIssuedThisTurn, photonChance, photonIdealNumber);



	ppPhaseOffset++;
	if (ppPhaseOffset == ppSkipSize) { ppPhaseOffset = 0;}
}

struct AnimalSegment
{
	unsigned int position;
	unsigned int animationFrame;
	unsigned int frames[squareSizeAnimalSprite * NUMBER_OF_FRAMES];
	AnimalSegment();
};

AnimalSegment::AnimalSegment()
{
	this->animationFrame = FRAME_BODY;
	this->position = 0;
	for (int j = 0; j < NUMBER_OF_FRAMES; ++j)
	{

		for (int i = 0; i < (squareSizeAnimalSprite); ++i)
		{
			this->frames[(squareSizeAnimalSprite * j) + i] = ORGAN_NOTHING;
		}
	}
}

struct Animal
{
	AnimalSegment segments[maxAnimalSegments];
	unsigned int segmentsUsed;
	unsigned int movementFlags;
	unsigned int direction;
	unsigned int energyFlags;
	unsigned int personalityFlags;
	unsigned int directionChangeFrequency;
	unsigned int age;

	int attack;
	int defense;
	int perception;
	float reproductionEnergy;
	int hitPoints;
	int maxHitPoints;
	int fitness;
	int mobility;
	int biggestMuscle;
	int biggestEye;
	float maxStoredEnergy;
	int timesReproduced;
	bool steady;
	bool mated;
	bool moved;
	bool retired;
	std::string * partnerGenes;
	float partnerReproductiveCost;

	unsigned int totalArea ;
	unsigned int totalMuscle ;
	unsigned int totalLiver  ;
	unsigned int totalHeart  ;
	unsigned int totalEye    ;
	unsigned int totalBone   ;
	unsigned int totalMouth  ;
	unsigned int totalWeapon ;
	unsigned int totalVacuole;

	Animal();
};

Animal::Animal()
{
	this->age = 0;
	this->energyFlags = ENERGYSOURCE_PLANT;
	this->direction = 4;
	this->segmentsUsed = 0;
	this->biggestMuscle = 0;
	this->biggestEye = 0;
	this->attack = 0;
	this->maxStoredEnergy = 0.0f;
	this->reproductionEnergy = 0.0f;
	this->hitPoints = 1;
	this->maxHitPoints = this->hitPoints;
	this->steady = false;
	this->timesReproduced = 0;
	this->retired = true; // so IDs get used starting from 0
	this->partnerGenes = nullptr;
	this->mated = false;
	this->totalArea    = 0;
	this->totalMuscle  = 0;
	this->totalLiver   = 0;
	this->totalHeart   = 0;
	this->totalEye     = 0;
	this->totalBone    = 0;
	this->totalMouth   = 0;
	this->totalWeapon  = 0;
	this->totalVacuole = 0;

	for (int i = 0; i < maxAnimalSegments; ++i)
	{
		this->segments[i] = AnimalSegment();
	}
	this->movementFlags = MOVEMENT_ONPOWDER | MOVEMENT_ONSOLID | MOVEMENT_INPLANTS;
}

struct WorldInformation
{
	unsigned int nMaterials;
	unsigned int nAnimals;
};

// THE EXTREMELY FAST LINE ALGORITHM Variation E (Addition Fixed Point PreCalc Small Display)
// Small Display (256x256) resolution.
std::list<ProposedLifeParticle> EFLA_E(vec_i2 start, vec_i2 end)
{
	int x  = start.x;
	int y  = start.y;
	int x2 = end.x;
	int y2 = end.y;

	if ((x - x2) > 256  || ((y - y2) > 256) )
	{
		printf(" You have used EFLA_E with a number that is too big for it! \n");
		printf("EFLA_E startx %i starty %i , endx %i endy %i \n", start.x, start.y, end.x, end.y);
	}

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
				v.push_back(   ProposedLifeParticle(cursor_color, vec_u2(j >> 8, y), cursor_energySource)   );
				j += decInc;
			}
			return v;
		}
		longLen += y;
		for (int j = 0x80 + (x << 8); y >= longLen; --y) {
			v.push_back( ProposedLifeParticle(cursor_color, vec_u2(j >> 8, y), cursor_energySource ));
			j -= decInc;
		}
		return v;
	}

	if (longLen > 0) {
		longLen += x;
		for (int j = 0x80 + (y << 8); x <= longLen; ++x) {
			v.push_back( ProposedLifeParticle(cursor_color, vec_u2(x, j >> 8) , cursor_energySource) );
			j += decInc;
		}
		return v;
	}
	longLen += x;
	for (int j = 0x80 + (y << 8); x >= longLen; --x) {
		v.push_back( ProposedLifeParticle(cursor_color, vec_u2(x, j >> 8) , cursor_energySource));
		j -= decInc;
	}
	return v;
}


// std::vector<Animal> animals;
Animal animals[maxAnimals];



// prints the animal to the terminal!
void catScan(unsigned int animalIndex)
{
	if (animalIndex > maxAnimals) {return;}
	printf("\nCAT scan report on animal %u\n", animalIndex);
	for (unsigned int segmentNumber = 0; segmentNumber < animals[animalIndex].segmentsUsed; ++segmentNumber)
	{
		unsigned int frameOffset = FRAME_BODY;
		for (unsigned int i = 0; i < squareSizeAnimalSprite; ++i)
		{
			if (i % sizeAnimalSprite == 0)
			{
				printf("\n");
			}
			switch (animals[animalIndex].segments[segmentNumber].frames[ frameOffset + i])
			{
			case ORGAN_NOTHING:
			{
				printf("_");
				break;
			}
			case ORGAN_EYE:
			{
				printf("Y");
				break;
			}
			case ORGAN_MUSCLE:
			{
				printf("M");
				break;
			}
			case ORGAN_MOUTH:
			{
				printf("O");
				break;
			}
			case ORGAN_LIVER:
			{
				printf("L");
				break;
			}
			case ORGAN_BONE:
			{
				printf("B");
				break;
			}
			case ORGAN_VACUOLE:
			{
				printf("V");
				break;
			}
			case ORGAN_MARKER_A:
			{
				printf("A");
				break;
			}
			}
		}
	}
	{
		printf("\n\n");
	}
}

void fillAPolygon(unsigned int animalIndex, unsigned int usegmentNumber, unsigned int frame)
{
	if (animalIndex >= maxAnimals  ) {return;}
	Animal * a  = &(animals[animalIndex]);
	unsigned int frameOffset = (squareSizeAnimalSprite) * frame;
	int segmentNumber = usegmentNumber;

	// traverse the sprite.  you will be setting alpha to -1 to mark a pixel that has connection to the outside.
	// if a pixel is an edge neighbour (cardinal direction) of a -1 pixel or the edge of the sprite, set it also to -1.
	// ignore pixels that have an alpha value other than 0 or -1. this way, you will not intrude into the image.
	// if the shapes is not allowed to extend beyond the edges of the sprite, then it is guaranteed that none of the edge squares
	// this way, extending the 'outside the polygon'-ness from the edge squares inward is formally safe to do
	// print the output.
	for (unsigned int i = 0; i < squareSizeAnimalSprite; ++i)
	{
		if (
		    (
		        i < sizeAnimalSprite                                          ||  // if i is on the bottom edge of the sprite, or
		        i > ((sizeAnimalSprite - 1)*sizeAnimalSprite)                  ||  // if i is on the top edge of the sprite, or
		        i % sizeAnimalSprite == 0                                   ||  // if i is on one side of the sprite, or
		        i % sizeAnimalSprite == (sizeAnimalSprite - 1)                  // if i is on the other edge of the sprite
		    ) &&
		    (
		        a->segments[segmentNumber].frames[ frameOffset + i] == ORGAN_NOTHING                    // and the pixel is not already drawn.
		    )
		)
		{
			a->segments[segmentNumber].frames[frameOffset + i] = ORGAN_MARKER_A;                        // set the alpha to -1.0f, which does not occur in our drawings normally, and so can be used to mark a pixel with connection to the edge.
		}
	}

	for (unsigned int k = 0; k < (sizeAnimalSprite / 2); ++k)                                           // multiple passes help to ensure there are no daggy bits left
	{
		for (unsigned int i = 0; i < (squareSizeAnimalSprite); ++i)
		{
			if ( a->segments[segmentNumber].frames[ frameOffset + i ] != ORGAN_NOTHING) {continue;}
			unsigned int spriteNeighbours[] =                                                            // calculate the addresses of the four cardinal neighbours.
			{
				i - 1,
				i + 1,
				i - sizeAnimalSprite,
				i + sizeAnimalSprite
			};

			for (unsigned int j = 0; j < 4; ++j)
			{
				unsigned int neighbour = spriteNeighbours[j];
				if (neighbour > (squareSizeAnimalSprite)) {continue;}
				if (
				    a->segments[segmentNumber].frames[ frameOffset +  neighbour ] == ORGAN_MARKER_A
				)
				{
					a->segments[segmentNumber].frames[ frameOffset +  i ] = ORGAN_MARKER_A;
				}
			}
		}
	}

	for (unsigned int i = 0; i < squareSizeAnimalSprite; ++i)
	{
		if      (a->segments[segmentNumber].frames[ frameOffset + i] == ORGAN_MARKER_A)
		{
			a->segments[segmentNumber].frames[frameOffset + i] = ORGAN_NOTHING;
		}
		else if (a->segments[segmentNumber].frames[ frameOffset + i] ==  ORGAN_NOTHING)
		{
			a->segments[segmentNumber].frames[frameOffset + i] = animalCursorOrgan;
		}
	}
}

void clearSeedParticle( unsigned int i)
{
	seedGrid[i].stage = 0x00;
	seedGrid[i].parentIdentity = 0x00;
	seedGrid[i].energy = 0.0f;
	seedGrid[i].genes = std::string("");
	memcpy( &(seedColorGrid[ i * numberOfFieldsPerVertex ]) , &(color_clear), sizeof(Color) );
}

// return 0 to continue drawing sequence, return -1 to break sequence by one level.
int drawAnimalFromChar (unsigned int i, unsigned int animalIndex, std::string genes )
{
	if (animalCursorString >= genes.length())           {return -1;}
	if (animalCursorSegmentNumber >= maxAnimalSegments) {return -1;}
	if (animalIndex >= maxAnimals)                {return -1;}

	Animal * a = &(animals[animalIndex]);

	animalCursorString++;
	char c = genes[animalCursorString];

#ifdef ANIMAL_DRAWING_READOUT
	printf("char %c, index %u. ", genes[animalCursorString], animalCursorString );
#endif
	switch (c)
	{

	case 'c':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Add a new cell in the next n segments : \n");
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		unsigned int numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = numberModifier % maxAnimalSegments ;
		if (numberModifier == 0) {numberModifier = 1;}
		unsigned int newCellSegments   = numberModifier;

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Add a new %u cell into the next %u segments at (x %u, y %u) \n", genes[animalCursorString] , animalCursorString, animalCursorOrgan, newCellSegments, animalCursorX, animalCursorY);
#endif
		unsigned int limit = animalCursorSegmentNumber + newCellSegments;
		if (limit > maxAnimalSegments) {limit = maxAnimalSegments;}
		if (limit > animals[animalIndex].segmentsUsed)
		{
			animals[animalIndex].segmentsUsed = limit;
#ifdef ANIMAL_DRAWING_READOUT
			printf("The number of segments was extended to %u\n", animals[animalIndex].segmentsUsed);
#endif
		}

		for (unsigned int j = animalCursorSegmentNumber; j < limit; ++j)
		{
			unsigned int newCellI = (animalCursorY * sizeAnimalSprite) + animalCursorX;
			animals[animalIndex].segments[j].frames[ (squareSizeAnimalSprite * FRAME_BODY) + newCellI ] = animalCursorOrgan;
		}
		return 0;
	}

	case 'e':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Extrude selected organ in the next n segments on all frames:\n");
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		unsigned int numberModifier = alphanumeric( genes[animalCursorString] );

		if (animals[animalIndex].segmentsUsed > 0)
		{
			unsigned int numberOfSegmentsToExtrude   = numberModifier;
#ifdef ANIMAL_DRAWING_READOUT
			printf("char %c, index %u. Extrude organ %u in the next %u segments \n", genes[animalCursorString] , animalCursorString, animalCursorOrgan, numberOfSegmentsToExtrude);
#endif
			unsigned int limit = animalCursorSegmentNumber + numberOfSegmentsToExtrude;
			if (limit > maxAnimalSegments) {limit = maxAnimalSegments;}
			if (limit > animals[animalIndex].segmentsUsed) {limit = animals[animalIndex].segmentsUsed;}

			for (unsigned int j = animalCursorSegmentNumber; j < limit; ++j)
			{
				for (unsigned int k = 0; k < squareSizeAnimalSprite; ++k)
				{

					unsigned int nSameOrganNeighours = 0;
					unsigned int nDifferentOrganNeighbours = 0;
					unsigned int nEmptyNeighbours = 0;

					// make sure you traverse the nearby layers too so it works in 3D!
					for (unsigned int scanningLayer = 0; scanningLayer < 3; ++scanningLayer)
					{
						unsigned int neighbours[] =
						{
							k - 1,
							k + 1,
							k - sizeAnimalSprite,
							k + sizeAnimalSprite
						};

						unsigned int layerIndex = j;
						if (layerIndex >= (scanningLayer - 1))
						{
							layerIndex -= (scanningLayer - 1);
						}

						//traverse the cell neighbours and tally up the important stats.
						for (unsigned int l = 0; l < 4; ++l)
						{
							unsigned int neighbour = neighbours[l] % squareSizeAnimalSprite;
							if (animals[animalIndex].segments[layerIndex].frames[ (squareSizeAnimalSprite * FRAME_BODY ) + neighbour ] == animalCursorOrgan)
							{
								nSameOrganNeighours ++;
							}
							else if (animals[animalIndex].segments[layerIndex].frames[ (squareSizeAnimalSprite * FRAME_BODY ) + neighbour ] == ORGAN_NOTHING)
							{
								nEmptyNeighbours ++;
							}
							else
							{
								nDifferentOrganNeighbours++;
							}
						}
					}

					// now you can apply crystallization rules.
					if ( nSameOrganNeighours > 0) // the crystal must at least be touching another crystal to grow. No spontaneous nucleation.
					{
						if (animalCursorExtrusionCondition == CONDITION_GREATERTHAN)
						{
							if ( nSameOrganNeighours > animalCursorExtrusionN)
							{
								animals[animalIndex].segments[j].frames[ (squareSizeAnimalSprite * FRAME_BODY ) + k ] = ORGAN_MARKER_A;
							}
						}
						else if (animalCursorExtrusionCondition == CONDITION_EQUAL)
						{
							if ( nSameOrganNeighours == animalCursorExtrusionN)
							{
								animals[animalIndex].segments[j].frames[ (squareSizeAnimalSprite * FRAME_BODY ) + k ] = ORGAN_MARKER_A;
							}
						}
						else if (animalCursorExtrusionCondition == CONDITION_LESSTHAN)
						{
							if ( nSameOrganNeighours < animalCursorExtrusionN)
							{
								animals[animalIndex].segments[j].frames[ (squareSizeAnimalSprite * FRAME_BODY ) + k ] = ORGAN_MARKER_A;
							}
						}
					}
				}
			}

			for (unsigned int j = animalCursorSegmentNumber; j < limit; ++j)
			{
				// then fill the marked areas, it's neater this way.
				for (unsigned int k = 0; k < squareSizeAnimalSprite; ++k)
				{
					if (animals[animalIndex].segments[j].frames[ (squareSizeAnimalSprite * FRAME_BODY ) + k ] == ORGAN_MARKER_A)
					{
						animals[animalIndex].segments[j].frames[ (squareSizeAnimalSprite * FRAME_BODY ) + k ] = animalCursorOrgan;
					}
				}
			}
		}
		return 0;
	}


	case 's':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Change segment cursor: ");
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		unsigned int numberModifier = alphanumeric( genes[animalCursorString] );

		if (animals[animalIndex].segmentsUsed > 0)
		{

			numberModifier = numberModifier % animals[animalIndex].segmentsUsed ;

			animalCursorSegmentNumber = numberModifier;
#ifdef ANIMAL_DRAWING_READOUT
			printf("char %c, index %u. Set segment cursor to %u\n", genes[animalCursorString] , animalCursorString, animalCursorSegmentNumber);
#endif
		}

		else
		{
#ifdef ANIMAL_DRAWING_READOUT
			printf( "the animal had 0 segments and was deleted\n" );
#endif
			clearSeedParticle(i);
			return -1;
		}
		return 0;
	}

	case 'u':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Change extrusion condition: ");
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		unsigned int numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = numberModifier % NUMBER_OF_CONDITIONS;

		animalCursorExtrusionCondition = numberModifier;

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set animal extrusion condition to %u\n", genes[animalCursorString] , animalCursorString, animalCursorExtrusionCondition);
#endif
		return 0;
	}

	case 'i':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Change extrusion n : ");
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		unsigned int numberModifier = alphanumeric( genes[animalCursorString] );

		animalCursorExtrusionN = numberModifier;

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set animal extrusion n parameter to %u\n", genes[animalCursorString] , animalCursorString, animalCursorExtrusionN);
#endif
		return 0;
	}

	case 'j':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Change organ cursor\n" );
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		unsigned int numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = numberModifier % 6;


		if        (numberModifier == 0) { animalCursorOrgan  = ORGAN_EYE;      }
		else if   (numberModifier == 1) { animalCursorOrgan  = ORGAN_MUSCLE;   }
		else if   (numberModifier == 2) { animalCursorOrgan  = ORGAN_MOUTH;    }
		else if   (numberModifier == 3) { animalCursorOrgan  = ORGAN_LIVER;    }
		else if   (numberModifier == 4) { animalCursorOrgan  = ORGAN_BONE;     }
		else if   (numberModifier == 5) { animalCursorOrgan  = ORGAN_VACUOLE;  }

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set organ type to %u \n", genes[animalCursorString] , animalCursorString, animalCursorOrgan );
#endif
		return 0;
	}

	case 'x':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Change drawing cursor X: ");
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		int numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = (numberModifier - 13);

		int diffX = (animalCursorX + numberModifier ) ;
		int sign = 1; if (diffX < 0) {sign = -1;}
		diffX = abs(diffX);

		// the drawing cursor must be guided to the destination, so that it does not go off the edge of the allowed drawing area.
		for (unsigned int j = 0; j < diffX; ++j)
		{
			unsigned int testI = (animalCursorY * sizeAnimalSprite) + animalCursorX;
			if (animals[animalIndex].segments[ animalCursorSegmentNumber].frames[ (squareSizeAnimalSprite * animalCursorFrame ) + testI ] == ORGAN_NOTHING)
			{
				break;
			}
			else
			{
				if (sign > 0)
				{
					animalCursorX ++;
				}
				else
				{
					animalCursorX --;
				}
			}
		}

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set drawing cursor X to %u\n", genes[animalCursorString] , animalCursorString, animalCursorX);
#endif
		return 0;
	}

	case 'y':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Change drawing cursor Y: ");
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		int numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = (numberModifier - 13);

		int diffY = (animalCursorY + numberModifier ) ;
		int sign = 1; if (diffY < 0) {sign = -1;}
		diffY = abs(diffY);

		// the drawing cursor must be guided to the destination, so that it does not go off the edge of the allowed drawing area.
		for (unsigned int j = 0; j < diffY; ++j)
		{
			unsigned int testI = (animalCursorY * sizeAnimalSprite) + animalCursorX;
			if (animals[animalIndex].segments[ animalCursorSegmentNumber].frames[ (squareSizeAnimalSprite * animalCursorFrame ) + testI ] == ORGAN_NOTHING)
			{
				break;
			}
			else
			{
				if (sign > 0)
				{
					animalCursorY ++;
				}
				else
				{
					animalCursorY --;
				}
			}
		}

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set drawing cursor Y to %u\n", genes[animalCursorString] , animalCursorString, animalCursorY);
#endif
		return 0;
	}

	case 'd':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Change diet\n", genes[animalCursorString] , animalCursorString );
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		unsigned int numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = numberModifier % 5;

		// ^= operator toggles the bit. this means the animal can have more than one movement type. It is important the ENERGYSOURCE_ defines are powers of 2 only.
		if        (numberModifier == 0) { a->energyFlags  ^= ENERGYSOURCE_LIGHT; }
		else if   (numberModifier == 1) { a->energyFlags  ^= ENERGYSOURCE_MINERAL; }
		else if   (numberModifier == 2) { a->energyFlags  ^= ENERGYSOURCE_SEED; }
		else if   (numberModifier == 3) { a->energyFlags  ^= ENERGYSOURCE_PLANT;    }
		else if   (numberModifier == 4) { a->energyFlags  ^= ENERGYSOURCE_ANIMAL;  }

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set diet to %u\n", genes[animalCursorString] , animalCursorString, a->energyFlags);
#endif
		return 0;
	}

	case 'm':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Change movement type\n", genes[animalCursorString] , animalCursorString );
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		unsigned int numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = numberModifier % 5;

		// ^= operator toggles the bit. this means the animal can have more than one movement type. It is important the MOVEMENT_ defines are powers of 2 only.
		if        (numberModifier == 0) { a->movementFlags  ^= MOVEMENT_ONPOWDER; }
		else if   (numberModifier == 1) { a->movementFlags  ^= MOVEMENT_INLIQUID; }
		else if   (numberModifier == 2) { a->movementFlags  ^= MOVEMENT_INPLANTS; }
		else if   (numberModifier == 3) { a->movementFlags  ^= MOVEMENT_INAIR;    }
		else if   (numberModifier == 4) { a->movementFlags  ^= MOVEMENT_ONSOLID;  }

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set movement type to %u\n", genes[animalCursorString] , animalCursorString, a->movementFlags );
#endif
		return 0;
	}

	case 'v':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Change personality type\n", genes[animalCursorString] , animalCursorString );
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		unsigned int numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = numberModifier % 4;

		// ^= operator toggles the bit. this means the animal can have more than one personality type. It is important the PERSONALITY_ defines are powers of 2 only.
		if        (numberModifier == 0) { a->personalityFlags  ^= PERSONALITY_AGGRESSIVE; }
		else if   (numberModifier == 1) { a->personalityFlags  ^= PERSONALITY_COWARDLY; }
		else if   (numberModifier == 2) { a->personalityFlags  ^= PERSONALITY_FLOCKING; }
		else if   (numberModifier == 3) { a->personalityFlags  ^= PERSONALITY_FRIENDLY;    }

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set personality type to %u\n", genes[animalCursorString] , animalCursorString, a->personalityFlags );
#endif
		return 0;
	}

	case 'r':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Set red component: \n");
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		float numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = numberModifier / 26;

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set red to %f\n", genes[animalCursorString] , animalCursorString, numberModifier);
#endif
		animalCursorColor = Color(  numberModifier, animalCursorColor.g, animalCursorColor.b, 1.0f    );
		return 0;
	}

	case 'g':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Set green component: \n");
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		float numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = numberModifier / 26;

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set green to %f\n", genes[animalCursorString] , animalCursorString, numberModifier);
#endif
		animalCursorColor = Color(  animalCursorColor.r, numberModifier, animalCursorColor.b, 1.0f    );
		return 0;
	}

	case 'b':
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Set blue component\n");
#endif
		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		float numberModifier = alphanumeric( genes[animalCursorString] );
		numberModifier = numberModifier / 26;

#ifdef ANIMAL_DRAWING_READOUT
		printf("char %c, index %u. Set blue to %f\n", genes[animalCursorString] , animalCursorString, numberModifier);
#endif
		animalCursorColor = Color(  animalCursorColor.r, animalCursorColor.g, numberModifier, 1.0f    );
		return 0;
	}

	default:
	{
#ifdef ANIMAL_DRAWING_READOUT
		printf("Junk DNA was used to move the animal cursors.\n");
#endif

		animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
		int numberModifier = alphanumeric( genes[animalCursorString] );
		if (numberModifier < 13)
		{

			animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
			int numberModifier = alphanumeric( genes[animalCursorString] );
			numberModifier = (numberModifier - 13);

			int diffX = (animalCursorX + numberModifier ) ;
			int sign = 1; if (diffX < 0) {sign = -1;}
			diffX = abs(diffX);

			// the drawing cursor must be guided to the destination, so that it does not go off the edge of the allowed drawing area.
			for (unsigned int j = 0; j < diffX; ++j)
			{
				unsigned int testI = (animalCursorY * sizeAnimalSprite) + animalCursorX;
				if (animals[animalIndex].segments[ animalCursorSegmentNumber].frames[ (squareSizeAnimalSprite * animalCursorFrame ) + testI ] == ORGAN_NOTHING)
				{
					break;
				}
				else
				{
					if (sign > 0)
					{
						animalCursorX ++;
					}
					else
					{
						animalCursorX --;
					}
				}
			}

#ifdef ANIMAL_DRAWING_READOUT
			printf("char %c, index %u. Set drawing cursor X to %u\n", genes[animalCursorString] , animalCursorString, animalCursorY);
#endif

		}
		else
		{
			animalCursorString++; if (animalCursorString > genes.length()) { return -1; }
			int numberModifier = alphanumeric( genes[animalCursorString] );
			numberModifier = (numberModifier - 13);

			int diffY = (animalCursorY + numberModifier ) ;
			int sign = 1; if (diffY < 0) {sign = -1;}
			diffY = abs(diffY);

			// the drawing cursor must be guided to the destination, so that it does not go off the edge of the allowed drawing area.
			for (unsigned int j = 0; j < diffY; ++j)
			{
				unsigned int testI = (animalCursorY * sizeAnimalSprite) + animalCursorX;
				if (animals[animalIndex].segments[ animalCursorSegmentNumber].frames[ (squareSizeAnimalSprite * animalCursorFrame ) + testI ] == ORGAN_NOTHING)
				{
					break;
				}
				else
				{
					if (sign > 0)
					{
						animalCursorY ++;
					}
					else
					{
						animalCursorY --;
					}
				}
			}

#ifdef ANIMAL_DRAWING_READOUT
			printf("char %c, index %u. Set drawing cursor Y to %u\n", genes[animalCursorString] , animalCursorString, animalCursorY);
#endif

		}
		return 0;
	}

	}
	return -1;
}

void measureAnimalQualities(unsigned int currentPosition)
{
	unsigned int animalIndex = seedGrid[currentPosition].parentIdentity;
	if (animalIndex >= maxAnimals)                {return;}

#ifdef ANIMAL_DRAWING_READOUT
	printf( "measureAnimalQualities on animal %u \n" , animalIndex );
#endif

	animals[animalIndex].attack = 1;
	animals[animalIndex].defense = 1;
	animals[animalIndex].perception = 1;
	animals[animalIndex].hitPoints = 1;
	animals[animalIndex].maxHitPoints = 1;
	animals[animalIndex].reproductionEnergy = 1.0f;
	animals[animalIndex].fitness = 1;

	unsigned int eyeStreak = 0;     // horizontal width of the largest eye found on the animal.
	unsigned int muscleStreak = 0;  // horizontal width of the largest muscle found on the animal.
	unsigned int prevOrgan = ORGAN_NOTHING;

	animals[animalIndex].totalArea = 0;
	animals[animalIndex].totalMuscle = 0;
	animals[animalIndex].totalLiver = 0;
	animals[animalIndex].totalHeart = 0;
	animals[animalIndex].totalEye = 0;
	animals[animalIndex].totalBone = 0;
	animals[animalIndex].totalMouth = 0;
	animals[animalIndex].totalWeapon = 0;
	animals[animalIndex].totalVacuole = 0;

	for (unsigned int j = 0; j < animals[animalIndex].segmentsUsed; ++j)
	{
		for (unsigned int k = 0; k < squareSizeAnimalSprite; ++k)
		{
			unsigned int organ = animals[animalIndex].segments[ j].frames[ (squareSizeAnimalSprite * FRAME_BODY ) + k ];

			if (organ != ORGAN_MUSCLE && prevOrgan == ORGAN_MUSCLE)
			{
				muscleStreak = 0;
			}

			if (k % sizeAnimalSprite == 0) { eyeStreak = 0; muscleStreak = 0; }

			if (organ != ORGAN_NOTHING )
			{
				animals[animalIndex].reproductionEnergy += 1.0f;
				animals[animalIndex].totalArea ++;

				if (organ == ORGAN_BONE )
				{
					animals[animalIndex].totalBone ++;
				}
				else if (organ == ORGAN_MUSCLE )
				{
					muscleStreak ++;

					animals[animalIndex].totalMuscle++;
					if (muscleStreak > animals[animalIndex].biggestMuscle)
					{
						animals[animalIndex].biggestMuscle = muscleStreak;
					}
				}
				else if (organ == ORGAN_LIVER )
				{
					animals[animalIndex].totalLiver ++;
				}
				else if (organ == ORGAN_EYE )
				{

					animals[animalIndex].totalEye ++;
					eyeStreak ++;
					if (eyeStreak > animals[animalIndex].biggestEye)
					{
						animals[animalIndex].biggestEye = eyeStreak;
					}
				}
				else if (organ == ORGAN_HEART )
				{
					animals[animalIndex].totalHeart ++;
				}
				else if (organ == ORGAN_WEAPON )
				{
					animals[animalIndex].totalWeapon ++;
				}
				else if (organ == ORGAN_MOUTH )
				{
					animals[animalIndex].totalMouth ++;
					animals[animalIndex].reproductionEnergy += 10.0f; // these have to be like crazy more expensive than everything else
				}

			}
			if (organ != ORGAN_EYE && prevOrgan == ORGAN_EYE)
			{
				eyeStreak = 0;
			}
			prevOrgan = animals[animalIndex].segments[ j].frames[ (squareSizeAnimalSprite * FRAME_BODY ) + k ];
		}
	}

	// some attributes depend on the ratio between body parts, and some are absolute.
	animals[animalIndex].attack = animals[animalIndex].totalWeapon ;
	animals[animalIndex].defense = animals[animalIndex].totalBone;
	animals[animalIndex].perception = animals[animalIndex].totalEye ;
	animals[animalIndex].hitPoints = animals[animalIndex].totalLiver;
	animals[animalIndex].reproductionEnergy = animals[animalIndex].totalArea;
	animals[animalIndex].fitness = animals[animalIndex].totalHeart;

	float mobilityRatio = (animals[animalIndex].totalMuscle / animals[animalIndex].totalArea);
	animals[animalIndex].mobility = 1 + (mobilityRatio * 2) ; // being about 50% muscle grants you an extra move square

	animals[animalIndex].maxHitPoints = animals[animalIndex].hitPoints;
	seedGrid[currentPosition].energy = animals[animalIndex].reproductionEnergy / 2;

#ifdef ANIMAL_DRAWING_READOUT
	printf( "attack %i \n" , animals[animalIndex].attack );
	printf( "defense %i \n" , animals[animalIndex].defense );
	printf( "perception %i \n" , animals[animalIndex].perception );
	printf( "hitPoints %i \n" , animals[animalIndex].hitPoints );
	printf( "reproductionEnergy %f \n" , animals[animalIndex].reproductionEnergy );
	printf( "biggestEye %u \n" , animals[animalIndex].biggestEye );
	printf( "biggestMuscle %u \n" , animals[animalIndex].biggestMuscle );
#endif
}

Color organColorLookup (unsigned int organ)
{
	switch (organ)
	{
	case ORGAN_NOTHING:
	{
		return color_clear;
		break;
	}
	case ORGAN_VACUOLE:
	{
		return color_grey;
	}
	case ORGAN_EYE:
	{
		return color_darkgrey;
		break;
	}
	case ORGAN_MUSCLE:
	{
		return color_darkred;
		break;
	}

	case ORGAN_MOUTH:
	{
		return color_brightred;
		break;
	}

	case ORGAN_LIVER:
	{
		return color_brown;
		break;
	}
	case ORGAN_BONE:
	{
		return color_offwhite;
		break;
	}
	case ORGAN_WEAPON:
	{
		return color_purple;
	}
	}
	return color_clear;
}

// check if a square is animal. created so you can do this from main without exposing the seed grid.
bool isAnimal(unsigned int i)
{
	if (seedGrid[i].stage == STAGE_ANIMAL)
	{return true;}
	else { return false;}
}

void clearAnimalSpritePixel ( unsigned int animalIndex, unsigned int segmentIndex, unsigned int pixelIndex )
{
	// find the x, y offset of the pixel from the origin in sprite coordinates.
	int pixelX = (pixelIndex % sizeAnimalSprite) - halfSizeAnimalSprite;
	int pixelY = (pixelIndex / sizeAnimalSprite) - halfSizeAnimalSprite;
	int segmentX = animals[animalIndex].segments[segmentIndex].position % sizeX;
	int segmentY = animals[animalIndex].segments[segmentIndex].position / sizeX;
	int worldX = segmentX + pixelX;
	int worldY = segmentY + pixelY;
	int worldI = ((worldY * sizeX) + worldX ) % totalSize;
	int j__color_offset = (worldI * numberOfFieldsPerVertex) ;

	if ( (animals[animalIndex].segments[segmentIndex].frames[(squareSizeAnimalSprite * animals[animalIndex].segments[segmentIndex].animationFrame) + pixelIndex]) != ORGAN_NOTHING  ) // only clear the pixel if it is not empty in the original image. This prevents disturbing other sprites.
	{
		memcpy( &animationGrid[ j__color_offset],     &color_clear ,     sizeof(Color) );
	}
}

void setAnimalSpritePixel ( unsigned int animalIndex, unsigned int segmentIndex, unsigned int pixelIndex )
{
	// find the x, y offset of the pixel from the origin in sprite coordinates.
	int pixelX = (pixelIndex % sizeAnimalSprite) - halfSizeAnimalSprite;
	int pixelY = (pixelIndex / sizeAnimalSprite) - halfSizeAnimalSprite;
	int segmentX = animals[animalIndex].segments[segmentIndex].position % sizeX;
	int segmentY = animals[animalIndex].segments[segmentIndex].position / sizeX;
	int worldX = segmentX + pixelX;
	int worldY = segmentY + pixelY;
	int worldI = ((worldY * sizeX) + worldX ) % totalSize;
	int weatherGridX = worldX / weatherGridScale;
	int weatherGridY = worldY / weatherGridScale;
	int weatherGridI = (weatherGridY * weatherGridSizeX) + weatherGridX;


	int j__color_offset = (worldI * numberOfFieldsPerVertex) ;

	unsigned int pixelOrgan =  animals[animalIndex].segments[segmentIndex].frames[
	                               (squareSizeAnimalSprite * animals[animalIndex].segments[segmentIndex].animationFrame) + pixelIndex
	                           ];

	if ( pixelOrgan != ORGAN_NOTHING)
	{
		Color organColor = organColorLookup(  pixelOrgan     );

		organColor = multiplyColor( organColor, weatherGrid[weatherGridI].color );

		memcpy( &animationGrid[ j__color_offset], &(    organColor )  ,     sizeof(Color) );

		// also, leave a pheromone trail for others to follow!
		if (seedGrid[worldI].stage == STAGE_NULL)
		{
			seedGrid[worldI].parentIdentity = animalIndex;
		}
	}
}

void clearAnimalDrawing(unsigned int i)
{
	if (seedGrid[i].parentIdentity < maxAnimals)
	{
		unsigned int animalIndex = seedGrid[i].parentIdentity;

		for (    unsigned int j = 0; j < animals[animalIndex].segmentsUsed; j++ )
		{
			for (unsigned int k = 0; k < (squareSizeAnimalSprite); ++k)
			{
				clearAnimalSpritePixel( animalIndex, j, k);
			}
		}
	}
}

// given an animal seed at position i (the method by which they are transported and reproduced), turn it into a complete animal
void drawAnimalFromSeed(unsigned int i)
{


	seedGrid[i].stage = STAGE_ANIMAL;

	// reset everything to the beginning state
	animalCursorFrame = 0;
	animalCursorString = 0;
	animalCursorX = (sizeAnimalSprite / 2); // return the drawing cursor to the center
	animalCursorY = (sizeAnimalSprite / 2);
	animalCursorColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
	animalCursorSegmentNumber = 0;
	animalCursorOrgan = ORGAN_MUSCLE;

	unsigned int animalIndex = 0;

	bool foundAnID = false;

	// printf("animals size %lu\n", maxAnimals);
	// if (maxAnimals > 0)
	// {

	// for (animalIndex = 0; animalIndex < 10; ++animalIndex)
	// {

	// 	printf("animal %u retirement state %u\n", animalIndex, animals[animalIndex].retired);
	// }



	for (animalIndex = 0; animalIndex < maxAnimals; ++animalIndex)
	{
		if (animals[animalIndex].retired)
		{
			// printf("animal %u was retired\n", animalIndex);
			// animalIndex = animal;
			foundAnID = true;

#ifdef ANIMAL_DRAWING_READOUT
			printf("Use animal ID %u\n", animalIndex);
#endif
			break;
		}
	}
	// }

	if (!foundAnID)
	{
		clearSeedParticle(i);
		return;
	}


	// printf("animal !!!!%u retirement state %u\n", animalIndex, animals[animalIndex].retired);
	animals[animalIndex].retired = false; // set this after creating Animal(), otherwise it will set back to true

	// printf("animal!!!! %u retirement state %u\n", animalIndex, animals[animalIndex].retired);
// 		// Animal newAnimal = Animal();


// 		std::vector<Animal>::iterator it;
// 		it = animals.end();
// 		animals.insert(it, Animal());
// 		// animals.push_back(newAnimal);
// 		animalIndex = maxAnimals - 1;

// 		// instead of push_back, use an iterator to insert
// 		// https://stackoverflow.com/questions/60393716/segmentation-fault-when-using-vector-push-back-inside-of-certain-loops
// 		// it still bugs out with insert if you are adding huge amounts of animals quickly

// #ifdef ANIMAL_DRAWING_READOUT
// 		printf("New animal with ID %u \n", animalIndex);
// #endif
// 	}


	seedGrid[i].parentIdentity = animalIndex;

	if (animalIndex < maxAnimals && seedGrid[i].stage == STAGE_ANIMAL)
	{
		seedGrid[i].drawingInProgress = true;

		// to improve code stability, this operation passes the animal and genome by literal value instead of by reference
		// it is operated on in a factory-like way and then the original is overwritten with the modified copy
		// animals[animalIndex] = Animal();
		std::string genome = seedGrid[i].genes;

#ifdef ANIMAL_DRAWING_READOUT
		printf("Drawing an animal at %u with genome length %lu \nThe first and last characters are ignored.\n", i, seedGrid[i].genes.length());
#endif
		int count = 0;
		while (true)
		{
			if (drawAnimalFromChar(i, animalIndex, genome) < 0 ) { break; }
			if (animalCursorString >= genome.length()) {break;}
			if (count > genome.length()) {break;}
			count++;
		}
		if (animals[animalIndex].segmentsUsed > 0)
		{
			catScan( animalIndex);
			measureAnimalQualities(i);
		}
		else
		{
#ifdef ANIMAL_DRAWING_READOUT
			printf( "the animal had 0 segments and was deleted\n" );
#endif
			clearSeedParticle(i);
		}

		if (animals[animalIndex].reproductionEnergy <= 0.0f)
		{
#ifdef ANIMAL_DRAWING_READOUT
			printf( "the animal reproduced at 0 or less energy, and was deleted\n" );
#endif
			clearSeedParticle(i);
		}

		seedGrid[i].drawingInProgress = false;
	}

	for (unsigned int segmentIndex = 0; segmentIndex < animals[animalIndex].segmentsUsed; ++segmentIndex)
	{
		animals[animalIndex].segments[segmentIndex].position = i;
	}
}

void setSeedParticle( std::string genes, unsigned int parentIdentity, float energyDebt, unsigned int i, unsigned int germinationMaterial)
{
	seedGrid[i].genes = genes;
	seedGrid[i].parentIdentity = parentIdentity;
	seedGrid[i].stage = STAGE_BUD;
	seedGrid[i].energy = energyDebt;
	seedGrid[i].germinationMaterial = germinationMaterial;

#ifdef PLANT_DRAWING_READOUT
	printf("seed with energy debt %f \n", energyDebt);
#endif

	memcpy( (&seedColorGrid[i * numberOfFieldsPerVertex]) ,  &(cursor_seedColor),  sizeof(Color) );
}


void swapSeedParticle(unsigned int a, unsigned int b)
{
	SeedParticle tempSeed = seedGrid[b];
	seedGrid[b] = seedGrid[a];
	seedGrid[a] = tempSeed;
	float temp_color[4];
	unsigned int a_offset = (a * numberOfFieldsPerVertex) ;
	unsigned int b_offset = (b * numberOfFieldsPerVertex) ;
	memcpy( temp_color,                 &seedColorGrid[ b_offset ],     sizeof(Color) ); // 4x floats of 4 bytes each
	memcpy( &seedColorGrid[ b_offset],  &seedColorGrid[ a_offset ],     sizeof(Color) );
	memcpy( &seedColorGrid[ a_offset ], temp_color,                     sizeof(Color) );
}



void setAnimal(unsigned int i, std::string genes)
{
	seedGrid[i].genes = genes;
	seedGrid[i].stage = STAGE_ANIMALEGG;
	memcpy( (&seedColorGrid[i * numberOfFieldsPerVertex]) ,  &(color_clear),  sizeof(Color) );

}

void mutateSentence ( std::string * genes )
{
	// a 50% chance not to mutate at all. this is to encourage a stable population if a design is good, and to buffer against genetic drift.
	if (extremelyFastNumberFromZeroTo(1) == 0) {return;}

	// make a single point mutation in the animal.
	unsigned int randomThingToChange = extremelyFastNumberFromZeroTo(3);

#ifdef MUTATION_READOUT
	printf("mutate : ");
#endif

	switch (randomThingToChange)
	{
	case 0:
	{
		// swap a letter
		if (genes->length() > 0)
		{
			(*genes)[extremelyFastNumberFromZeroTo(genes->length() - 1)] = (char)('a' + rand() % 26);

#ifdef MUTATION_READOUT
			printf("swap a letter               : ");
#endif
		}
		break;
	}
	case 1:
	{
		if (genes->length() > 0)
		{
			// add a letter
			genes->push_back(  (char)('a' + rand() % 26)  );

#ifdef MUTATION_READOUT
			printf("add a letter                : ");
#endif
		}
		break;
	}
	case 2:
	{
		// remove a letter
		if (genes->length() > 0)
		{
			genes->erase(extremelyFastNumberFromZeroTo(genes->length() - 1), 1);
#ifdef MUTATION_READOUT
			printf("remove a letter             : ");
#endif
		}
		else
		{
#ifdef MUTATION_READOUT
			printf("remove a letter: len 0 abort: ");
#endif
		}
		break;
	}
	case 3:
	{
		if (genes->length() > 0)
		{
			// swap a letter for punctuation
			unsigned int randomPunctuation = extremelyFastNumberFromZeroTo(1);

			if      (randomPunctuation == 0) { (*genes)[extremelyFastNumberFromZeroTo(genes->length() - 1)] = ' ' ; }
			else if (randomPunctuation == 1) { (*genes)[extremelyFastNumberFromZeroTo(genes->length() - 1)] = '.' ; }

#ifdef MUTATION_READOUT
			printf("swap letter for punctuation : ");
#endif
		}
		break;
	}
	}

#ifdef MUTATION_READOUT
	printf(" : %s\n", (*genes).c_str());
#endif
}

void setParticle(unsigned int material, unsigned int i)
{
	grid[i].temperature = defaultTemperature;
	grid[i].material = material;
	grid[i].phase = PHASE_POWDER;
	unsigned int a_offset = (i * numberOfFieldsPerVertex);
	memcpy( &colorGrid[ a_offset ], & (materials[material].color), 16 );
}

void killAnAnimal(unsigned int i)
{
	// delete the animal from the relevant grids, leaving behind a huge mess
	if (seedGrid[i].parentIdentity >= maxAnimals)
	{
		return;
	}

	unsigned int animalIndex = seedGrid[i].parentIdentity;

	for (     int j = animals[animalIndex].segmentsUsed - 1; j >= 0 ; j-- )
	{
		for ( int k = 0; k < squareSizeAnimalSprite; ++k)
		{
			int pixelIndex = (squareSizeAnimalSprite * FRAME_BODY) + k;

			if (j > animals[animalIndex].segmentsUsed) {continue;}
			if (pixelIndex > squareSizeAnimalSprite) {continue;}
			if ( animals[animalIndex].segments[j].frames[pixelIndex] != ORGAN_NOTHING)
			{
				int worldSegmentX = animals[animalIndex].segments[j].position % sizeX;
				int worldSegmentY = animals[animalIndex].segments[j].position / sizeX;
				int spritePixelX = k % sizeAnimalSprite;
				int spritePixelY = k / sizeAnimalSprite;
				int neighbourX = worldSegmentX + (spritePixelX - (sizeAnimalSprite / 2));
				int neighbourY = worldSegmentY + (spritePixelY - (sizeAnimalSprite / 2));
				int iworldRandomI = ((neighbourY * sizeX) + neighbourX ) ;
				if (iworldRandomI > 0)
				{
					unsigned int worldRandomI = iworldRandomI % totalSize;

					if (grid[worldRandomI].phase == PHASE_VACUUM)
					{
						if (animals[animalIndex].segments[j].frames[pixelIndex] == ORGAN_BONE)
						{
							setParticle(MATERIAL_BONE, worldRandomI);
							grid[worldRandomI].phase = PHASE_SOLID;
							grid[worldRandomI].temperature = 310;
						}
						else
						{
							setParticle(MATERIAL_BLOOD, worldRandomI);
							grid[worldRandomI].phase = PHASE_LIQUID;
							grid[worldRandomI].temperature = 310;
						}
					}
				}
			}
		}
	}

	printf("animal %u died\n", animalIndex);

	// animals[animalIndex].retired = true;
	clearAnimalDrawing(i);
	clearSeedParticle(i);
	animals[animalIndex] = Animal(); // reset to default state.
}

void killSelf()
{

	if (firstPerson)
	{
		killAnAnimal(playerPosition);
		firstPerson = false;
	}

}

void incrementAnimalSegmentPositions (unsigned int animalIndex, unsigned int i)
{
	// if (animalIndex < maxAnimals)
	// {

	// printf("incrementAnimalSegmentPositions ");

	if (animals[animalIndex].segmentsUsed > 0 )
	{
		// Update animal segment positions. Only do this if the animal has actually moved (otherwise it will pile into one square).
		if (animals[animalIndex].segments[0].position != i)
		{



			// clear the old drawing
			for (  int j = (animals[animalIndex].segmentsUsed - 1); j >= 0; --j) // when using lots of minuses and value comparisons like this its a good idea to use int. this would hang forever if j was unsigned
			{
				for (unsigned int k = 0; k < (squareSizeAnimalSprite); ++k)
				{
					clearAnimalSpritePixel(animalIndex, j, k);
				}
			}

			// update the segment positions
			for (  int j = (animals[animalIndex].segmentsUsed - 1); j > 0; --j)
			{

				animals[animalIndex].segments[j].position = animals[animalIndex].segments[j - 1].position;

				// printf("seg %i pos %u ", j,  animals[animalIndex].segments[j].position );
			}

			animals[animalIndex].segments[0].position = i;

			// draw the new drawing
			for (  int j = (animals[animalIndex].segmentsUsed - 1); j >= 0; --j)
			{
				for (unsigned int k = 0; k < (squareSizeAnimalSprite); ++k)
				{
					setAnimalSpritePixel( animalIndex, j, k);
				}
			}


			// printf(" \n " );
		}
	}
	// }
}

// these materials are expected by the game, so they must be loaded in every situation.
void resetMaterials(unsigned int materialsList)
{
	materials.clear();

	// materials[0] is supposed to be just air i guess, but it's called vacuum in the game, even though it acts like air.
	Material vacuum = Material();
	vacuum.color = color_clear;
	vacuum.melting = 0;
	vacuum.boiling = 0;
	vacuum.insulativity = 32;
	materials.push_back(vacuum);

// materials[1] is supposed to be water.
	Material water = Material();
	water.color = color_lightblue;
	water.melting = 273;
	water.boiling = 373;
	water.insulativity = 1;
	materials.push_back(water);

// materials[2] is supposed to be blood.
	Material blood = Material();
	blood.color = color_darkred;
	blood.melting = 0;
	blood.boiling = 100;
	blood.insulativity = 1;
	materials.push_back(blood);

// materials[3] is supposed to be bones.
	Material bone = Material();
	bone.color = color_offwhite;
	bone.melting = 1000;
	bone.boiling = 2200;
	bone.insulativity = 3;
	materials.push_back(bone);


	if (materialsList == MATERIALS_RANDOM)
	{
		// wacky alien goo.
		const unsigned int nNewMaterials = 5;
		for (unsigned int k = 0; k < nNewMaterials; ++k)
		{
			Material newMaterial = Material();
			newMaterial.boiling = RNG() * 2500;
			newMaterial.melting = RNG() * newMaterial.boiling;
			newMaterial.crystal_n = 1 + extremelyFastNumberFromZeroTo(4);
			unsigned int randomCondition =  extremelyFastNumberFromZeroTo(7);
			if (randomCondition == 0) {newMaterial.crystal_condition = CONDITION_GREATERTHAN; }
			else if (randomCondition == 1) {newMaterial.crystal_condition = CONDITION_EQUAL; }
			else if (randomCondition == 2) {newMaterial.crystal_condition = CONDITION_LESSTHAN; }
			else if (randomCondition == 3) {newMaterial.crystal_condition = CONDITION_EVENNUMBER; }
			else if (randomCondition == 4) {newMaterial.crystal_condition = CONDITION_ODDNUMBER; }
			else if (randomCondition == 5) {newMaterial.crystal_condition = CONDITION_EDGE; }
			else if (randomCondition == 6) {newMaterial.crystal_condition = CONDITION_CORNER; }
			else if (randomCondition == 7) {newMaterial.crystal_condition = CONDITION_NOTLEFTRIGHTN; }
			newMaterial . color = color_grey;
			newMaterial.color.r = RNG();
			newMaterial.color.g = RNG();
			newMaterial.color.b = RNG();
			materials.push_back(newMaterial);
		}


	}


	else if (materialsList == MATERIALS_TEMPLE)
	{
		Material redWater = water;
		redWater.color = color_brightred;
		materials.push_back(redWater);

		Material goldWater = water;
		goldWater.color = color_yellow;
		materials.push_back(goldWater);

		Material redStone = Material();
		redStone.color = color_brightred;
		redStone.melting = 1000;
		redStone.boiling = 2400;
		redStone.insulativity = 1;
		redStone.crystal_n = 4;
		redStone.crystal_condition = CONDITION_GREATERTHAN;
		materials.push_back(redStone);

	}


	else if (materialsList == MATERIALS_STANDARD)
	{
		// earth rocks.
		Material granite = Material();
		granite.color = color_lightgrey;
		granite.melting = 600;
		granite.boiling = 1800;
		granite.insulativity = 3;
		granite.crystal_n = 2;
		granite.crystal_condition = CONDITION_GREATERTHAN;
		materials.push_back(granite);

		Material amphibole = Material();
		amphibole.color = color_grey;
		amphibole.melting = 800;
		amphibole.boiling = 2000;
		amphibole.insulativity = 2;
		amphibole.crystal_n = 3;
		amphibole.crystal_condition = CONDITION_GREATERTHAN;
		materials.push_back(amphibole);

		Material olivine = Material();
		olivine.color = color_darkgrey;
		olivine.melting = 1000;
		olivine.boiling = 2400;
		olivine.insulativity = 1;
		olivine.crystal_n = 4;
		olivine.crystal_condition = CONDITION_GREATERTHAN;
		materials.push_back(olivine);
	}

}



void setLifeParticle( std::string genes, unsigned int identity, unsigned int i, Color color, unsigned int energySource)
{
	lifeGrid[i].identity = identity;
	lifeGrid[i].genes  = genes;
	lifeGrid[i].energy = 0.0f;
	lifeGrid[i].energySource = energySource;
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
	memcpy( &tempLife,  &(lifeGrid[a]),      sizeof(LifeParticle) );
	memcpy( &(lifeGrid[a]),  &(lifeGrid[b]), sizeof(LifeParticle) );
	memcpy( &(lifeGrid[b]),  &tempLife,      sizeof(LifeParticle) );
	float temp_color[4];
	unsigned int a_offset = (a * numberOfFieldsPerVertex)  ;
	unsigned int b_offset = (b * numberOfFieldsPerVertex)  ;
	memcpy( temp_color, &lifeColorGrid[ b_offset ] , sizeof(Color) ); // 4x floats of 4 bytes each
	memcpy( &lifeColorGrid[ b_offset], &lifeColorGrid[ a_offset] , sizeof(Color) );
	memcpy( &lifeColorGrid[ a_offset ], temp_color, sizeof(Color) );
}

void clearLifeParticle(unsigned int i)
{
	lifeGrid[i].identity = 0x00;
	lifeGrid[i].genes.clear();
	lifeGrid[i].energy = 0.0f;
	lifeGrid[i].energySource = ENERGYSOURCE_LIGHT;
	memcpy( (&lifeColorGrid[i * numberOfFieldsPerVertex]),  &(color_clear),  sizeof(Color) );
}

void clearColorGrids(unsigned int i)
{
	unsigned int x = i % sizeX;
	unsigned int y = i / sizeX;
	float fx = x;
	float fy = y;
	unsigned int       a_offset      = (i * numberOfFieldsPerVertex) ;
	// backgroundSky[     a_offset + 0] = 0.0f;
	// backgroundSky[     a_offset + 1] = 0.0f;
	// backgroundSky[     a_offset + 2] = 0.0f;
	// backgroundSky[     a_offset + 3] = 0.0f;
	// backgroundSky[     a_offset + 4] = fx;
	// backgroundSky[     a_offset + 5] = fy;
	colorGrid[         a_offset + 0] = 0.0f;
	colorGrid[         a_offset + 1] = 0.0f;
	colorGrid[         a_offset + 2] = 0.0f;
	colorGrid[         a_offset + 3] = 0.0f;
	colorGrid[         a_offset + 4] = fx;
	colorGrid[         a_offset + 5] = fy;
	lifeColorGrid[     a_offset + 0] = 0.0f;
	lifeColorGrid[     a_offset + 1] = 0.0f;
	lifeColorGrid[     a_offset + 2] = 0.0f;
	lifeColorGrid[     a_offset + 3] = 0.0f;
	lifeColorGrid[     a_offset + 4] = fx;
	lifeColorGrid[     a_offset + 5] = fy;
	animationGrid[     a_offset + 0] = 0.0f;
	animationGrid[     a_offset + 1] = 0.0f;
	animationGrid[     a_offset + 2] = 0.0f;
	animationGrid[     a_offset + 3] = 0.0f;
	animationGrid[     a_offset + 4] = fx;
	animationGrid[     a_offset + 5] = fy;
	seedColorGrid[     a_offset + 0] = 0.0f;
	seedColorGrid[     a_offset + 1] = 0.0f;
	seedColorGrid[     a_offset + 2] = 0.0f;
	seedColorGrid[     a_offset + 3] = 0.0f;
	seedColorGrid[     a_offset + 4] = fx;
	seedColorGrid[     a_offset + 5] = fy;
}

void clearAnimationGrid()
{
	unsigned int x = 0;
	unsigned int y = 0;
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		x = i % sizeX;
		y = i / sizeX;
		unsigned int a_offset = (i * numberOfFieldsPerVertex) ;
		memcpy( &animationGrid[a_offset], & color_clear, sizeof(Color) );
	}
}

void clearGrids()
{
	unsigned int x = 0;
	unsigned int y = 0;
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		x = i % sizeX;
		y = i / sizeX;
		float fx = x;
		float fy = y;

		grid[i] = Particle();
		seedGrid[i] = SeedParticle();
		lifeGrid[i] = LifeParticle();

		clearParticle(i);
		clearSeedParticle(i);
		clearLifeParticle(i);

		// RGBA color occupies the first 4 places.
		// also, initialize the color alpha to 1.
		clearColorGrids(i);
	}

	for (int i = 0; i < weatherGridSize; ++i)
	{
		weatherGrid [ i ] = Weather();
	}



}

void paintMaterialCircle(unsigned int k, unsigned int radius, unsigned int material, unsigned int temperature)
{
	unsigned int x = k % sizeX;
	unsigned int y = k / sizeX;
	unsigned int lower = k - (radius * sizeX);
	unsigned int upper = k + (radius * sizeX);
	for (unsigned int i = lower; i < upper; ++i)
	{
		unsigned int circleCursorX = i % sizeX;
		unsigned int circleCursorY = i / sizeX;
		if (  magnitude_int (  circleCursorX - x , circleCursorY - y )  < radius )
		{
			unsigned int l = i % totalSize;
			setParticle(material, l );
			grid[l].temperature = temperature;
		}
	}
}


void clearAnimals()
{
	for (int i = 0; i < maxAnimals; ++i)
	{
		animals[i] = Animal();
	}

	for (int i = 0; i < totalSize; ++i)
	{
		if (seedGrid[i].stage == STAGE_ANIMAL)
		{
			clearSeedParticle(i);
		}
	}
}

void createWorld( unsigned int world)
{
	clearGrids();
	clearAnimals();

	switch (world)
	{

	case WORLD_CATUN:
	{

		resetMaterials(MATERIALS_STANDARD);
		sunlightTemp = 2000;
		sunlightBrightness = 300;
		defaultTemperature = 200;
		defaultPressure = 1000;

		for (int i = 0; i < totalSize; ++i)
		{
			// // a layer of stone on the bottom
			if (i > 0 && i < 20 * sizeX)
			{
				setParticle(6, i);
			}

			// a dollop of super hot rock
			// if ((i > (100 * sizeX)) && (i < (200 * sizeX)) )
			// {
			// 	unsigned int x = i % sizeX;

			// 	if (x > 500 && x < 600)
			// 	{

			// 		setParticle(5, i);
			// 		grid[i].phase = PHASE_SOLID;
			// 		grid[i].temperature = 2000;
			// 	}
			// }

			// horizontal monolith
			if ((i > (200 * sizeX)) && (i < (300 * sizeX)) )
			{
				unsigned int x = i % sizeX;
				if (x > 700 && x < 1000)
				{
					setParticle(5, i);
					grid[i].phase = PHASE_SOLID;
				}
			}

			// vertical monolith
			if ((i > (300 * sizeX)) && (i < (1000 * sizeX)) )
			{
				unsigned int x = i % sizeX;
				if (x > 500 && x < 600)
				{
					setParticle(5, i);
					grid[i].phase = PHASE_SOLID;
				}
			}

			// a monolith in the 0 corner
			// if ((i > (300 * sizeX)) && (i < (1000 * sizeX)) )
			// {
			unsigned int x = i % sizeX;
			unsigned int y = i / sizeX;
			if (x < 50 && y < 50)
			{
				setParticle(5, i);
				grid[i].phase = PHASE_SOLID;
			}
			// }
		}
		break;
	}



	case WORLD_POOLOFTHESUN:
	{
		resetMaterials(MATERIALS_TEMPLE);
		sunlightTemp = 5900;
		sunlightBrightness = 200;
		defaultTemperature = 300;
		defaultPressure = 1000;



		for (int i = 0; i < totalSize; ++i)
		{


			int x = i % sizeX;
			int y = i / sizeX;
			int centerX = sizeX / 2;
			int centerY = sizeY / 2;

			if ( y < (sizeY / 3) )
			{
				if ( y < (sizeY / 8) )
				{
					setParticle(6, i);
					grid[i].phase = PHASE_SOLID;
				}

				if (magnitude_int( ((x) - centerX ) , y - centerY   )  <  (sizeX / 4))
				{
					if (magnitude_int( ((x) - centerX  ) , y - centerY  )  > (sizeX / 4.1))
					{
						setParticle(6, i);
						grid[i].phase = PHASE_SOLID;
					}

					else
					{
						if (x > centerX)
						{
							setParticle(4, i);
							grid[i].phase = PHASE_LIQUID;
						}
						else
						{
							setParticle(5, i);
							grid[i].phase = PHASE_LIQUID;
						}
					}
				}
			}
		}

		break;
	}

	case WORLD_EARTH:
	{
		resetMaterials(MATERIALS_STANDARD);
		sunlightTemp = 5900;
		sunlightBrightness = 200;
		defaultTemperature = 300;
		defaultPressure = 1000;
		for (int i = 0; i < totalSize; ++i)
		{
			// mixed rocks
			if (i > 0 && i < 50 * sizeX)
			{
				unsigned int rand = extremelyFastNumberFromZeroTo(2);
				if (rand == 0)
				{
					setParticle(4, i);
				}
				else if (rand == 1)
				{
					setParticle(5, i);
				} else if (rand == 2)
				{
					setParticle(6, i);
				}
			}

			// ocean
			if (i > 50 * sizeX && i < 100 * sizeX)
			{

				unsigned int x = i % sizeX;
				if (  !( x > 1500 && x < 2500 )  )
				{

					setParticle(1, i);
				}

			}

			if (i > 50 * sizeX && i < 120 * sizeX)
			{

				unsigned int x = i % sizeX;
				if (  ( x > 1500 && x < 2500 )  )
				{
					setParticle(4, i);
				}

			}


			// humidity
			if (i > 50)
			{
				if (extremelyFastNumberFromZeroTo(weatherGridScale * 20) == 0)
				{
					setParticle(1, i);
				}
			}

			// drop boiling lava in it
			// if ((i > (100 * sizeX)) && (i < (200 * sizeX)) )
			// {
			// 	unsigned int x = i % sizeX;
			// 	if (x > 500 && x < 600)
			// 	{
			// 		setParticle(5, i);
			// 		grid[i].phase = PHASE_SOLID;
			// 		grid[i].temperature = 2000;
			// 	}
			// }
		}
		break;
	}

	case WORLD_RANDOM:
	{

		resetMaterials(MATERIALS_RANDOM);
		sunlightTemp       = RNG() * 10000;
		defaultTemperature = RNG() * 1000;
		defaultPressure    = RNG() * 2000;


		sunlightBrightness = 100 + (RNG() * 1000 );

		for (unsigned int k = 0; k < materials.size(); ++k)
		{
			materials[k]. availability = extremelyFastNumberFromZeroTo(50);
			for (unsigned int i = 0; i < sizeX; ++i)
			{
				if (extremelyFastNumberFromZeroTo(100) == 0)
				{
					unsigned int radius = extremelyFastNumberFromZeroTo(materials[k]. availability  );
					paintMaterialCircle(i + (50 * sizeX * (k + 1)  ), radius , k , 10000000 );
				}
			}
		}
		break;
	}
	}

	sunlightColor = blackbodyLookup(sunlightTemp);

	// compute the background stars
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		unsigned int a_offset  = i * numberOfFieldsPerVertex;

		// memcpy( &(backgroundSky[ a_offset ]), &color_black, sizeof(Color) );

		if (extremelyFastNumberFromZeroTo(100) ==  0)
		{
			// create a background star with random blackbody color and alpha
			unsigned int randomColorTemperature = extremelyFastNumberFromZeroTo(5000);
			Color randomStarColor = blackbodyLookup(randomColorTemperature);
			float randomStarAlpha = RNG();
			randomStarAlpha = randomStarAlpha * randomStarAlpha * randomStarAlpha * randomStarAlpha * randomStarAlpha * randomStarAlpha * randomStarAlpha; // cubing the value or more shifts the distribution lower while preserving the range.
			randomStarAlpha = randomStarAlpha / 2;
			randomStarColor.r *= randomStarAlpha;
			randomStarColor.g *= randomStarAlpha;
			randomStarColor.b *= randomStarAlpha;
			randomStarColor.a = 1.0f; //randomStarAlpha;

			backgroundStars[i] = randomStarColor;
			// memcpy( &(backgroundSky[ a_offset ]), &randomStarColor, sizeof(Color) );
		}
	}
}

// given two world vertices, find the direction from a to b, as expressed by what entry in neighbourOffsets is the closest.
// basically get the angle with atan, apply an angle offset so both 0's are in the same place, and then map 0..2pi to 0..8
unsigned int getRelativeDirection (unsigned int a, unsigned int b)
{
	unsigned int result = 0;
	int ia = a;
	int ib = b;
	int aX =  ia % sizeX;
	int aY =  ia / sizeX;
	int bX =  ib % sizeX;
	int bY =  ib / sizeX;
	int diffX = bX - aX;
	int diffY = bY - aY;
	float fdiffX = diffX;
	float fdiffY = diffY;
	float angle = atan2(fdiffY, fdiffX);
	angle += const_pi;
	angle = angle / const_tau;
	angle *= 8.0f;
	result = angle;
	if (result == 8) {result = 0;}
	return result;
}

// should the animal move into that position?
bool animalCanMove(unsigned int i, unsigned int neighbour)
{
	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex < maxAnimals && neighbour < totalSize)
	{
		if (  (animals[animalIndex].movementFlags & MOVEMENT_INAIR ) == MOVEMENT_INAIR   )
		{
			animals[animalIndex].steady  = true;
			if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS)        // if one of the neighbouring cells is a material type and phase that the animal can exist within
			{
				return true;
			}
		}
		if (  (animals[animalIndex].movementFlags & MOVEMENT_INPLANTS ) == MOVEMENT_INPLANTS   )
		{
			if (lifeGrid[neighbour].identity > 0x00)
			{
				animals[animalIndex].steady  = true;
				return true;
			}
		}
		if (  (animals[animalIndex].movementFlags & MOVEMENT_INLIQUID ) == MOVEMENT_INLIQUID   )
		{
			if (grid[neighbour].phase == PHASE_LIQUID)                                              // if one of the neighbouring cells is a material type and phase that the animal can exist within
			{
				animals[animalIndex].steady  = true;
				return true;
			}
		}
		if (  (animals[animalIndex].movementFlags & MOVEMENT_ONPOWDER ) == MOVEMENT_ONPOWDER   )
		{
			if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS || grid[neighbour].phase == PHASE_LIQUID ) // if one of the neighbouring cells is a material type and phase that the animal can exist within
			{
				for (int l = 0; l < N_NEIGHBOURS; ++l)                                              // and it has a neighbour of a type and phase the animal can walk on
				{
					int neighboursNeighbour = (neighbour + neighbourOffsets[l]);
					if (neighboursNeighbour != i )
					{
						if (  grid[ neighboursNeighbour ] .phase == PHASE_POWDER )
						{
							animals[animalIndex].steady  = true;
							return true;
						}
					}
				}
			}
		}
		if (  (animals[animalIndex].movementFlags & MOVEMENT_ONSOLID ) == MOVEMENT_ONSOLID   )
		{
			if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS || grid[neighbour].phase == PHASE_LIQUID ) // if one of the neighbouring cells is a material type and phase that the animal can exist within
			{
				for (int l = 0; l < N_NEIGHBOURS; ++l)                                              // and it has a neighbour of a type and phase the animal can walk on
				{
					int neighboursNeighbour = (neighbour + neighbourOffsets[l]);
					if (neighboursNeighbour != i)
					{
						if (  grid[ neighboursNeighbour ] .phase == PHASE_SOLID )
						{
							animals[animalIndex].steady = true;
							return true;
						}
					}
				}
			}
		}
	}
	return false;  // this is taken as a signal not to move
}

/*
* find a direction of travel that is possible for the animal seed to move, as close to the direction of steering as possible.
* it doesn't have to be muscle tissue, but it will only ever be 1 point away from the seed.
* returns the index of the square to move to.
*/
unsigned int getDMostWalkableSquare(unsigned int i, unsigned int animalIndex, unsigned int direction, unsigned int startPosition)
{
	unsigned int neighbour = startPosition + neighbourOffsets[ direction ];
	if (animalIndex < maxAnimals && direction < N_NEIGHBOURS )
	{
		int sign = 1;                                // sign is inverted after each go, makes the search neighbour flip between left and right.
		if (extremelyFastNumberFromZeroTo(1) == 0) {sign = -1;}
		for (int j = 0; j < N_NEIGHBOURS; ++j)                                      // test neighbours in order of least to most angle difference
		{
			int noise = (extremelyFastNumberFromZeroTo(2) - 1);                     // without noise or other small disturbances, animals move very robotically and get stuck all the time.
			unsigned int walkableNeighbourDirection = (direction +  (j * sign) + noise) % N_NEIGHBOURS;
			neighbour = startPosition + neighbourOffsets[ walkableNeighbourDirection ];
			if (animalCanMove(i, neighbour))
			{
				// printf("getDMostWalkableSquare, animal @ %u, direction %u, result %u\n", i, direction, neighbour);
				return neighbour;
			}
			sign = sign * -1;
		}
	}

	return startPosition;
}

bool animalEat(unsigned int currentPosition , unsigned int neighbour )
{

	unsigned int animalIndex  = seedGrid[currentPosition].parentIdentity;

	if (animalIndex > maxAnimals) {return false;}
	if (currentPosition == neighbour) {return false;}

	bool eaten = false;

	while (true) //this statement is only here so it can be expediently broken out of
	{
		if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_SEED ) == ENERGYSOURCE_SEED   )
		{
			if (seedGrid[neighbour].stage == STAGE_BUD || seedGrid[neighbour].stage == STAGE_FRUIT ||  seedGrid[neighbour].stage == STAGE_SPROUT )
			{
				seedGrid[currentPosition].energy  += seedEfficiency ;
#ifdef ANIMAL_BEHAVIOR_READOUT
				printf("animal %u ate a seed for %f. Has %f reproduces at %f.\n", animalIndex, seedEfficiency, seedGrid[currentPosition].energy , animals[animalIndex].reproductionEnergy );
#endif
				clearSeedParticle(neighbour);
				eaten = true;
				break;
			}
		}
		if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_PLANT ) == ENERGYSOURCE_PLANT   )
		{
			if (lifeGrid[neighbour].identity > 0x00)
			{
				seedGrid[currentPosition].energy  += plantEfficiency;
#ifdef ANIMAL_BEHAVIOR_READOUT
				printf("animal %u ate a plant for %f. Has %f reproduces at %f.\n", animalIndex, plantEfficiency, seedGrid[currentPosition].energy , animals[animalIndex].reproductionEnergy );
#endif
				clearLifeParticle(neighbour);
				eaten = true;
				break;
			}
		}
		if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_MINERAL ) == ENERGYSOURCE_MINERAL   )
		{
			if (grid[neighbour].phase != PHASE_VACUUM)
			{
				seedGrid[currentPosition].energy  += mineralEfficiency;
#ifdef ANIMAL_BEHAVIOR_READOUT
				printf("animal %u ate a mineral for %f. Has %f reproduces at %f.\n" , animalIndex, mineralEfficiency, seedGrid[currentPosition].energy , animals[animalIndex].reproductionEnergy );
#endif
				clearParticle( neighbour);
				eaten = true;
				break;
			}
		}

		if ( (animals[animalIndex].energyFlags & ENERGYSOURCE_ANIMAL ) == ENERGYSOURCE_ANIMAL )        // if it eats other animals,
		{
			if (grid[neighbour].material == MATERIAL_BLOOD)
			{

#ifdef ANIMAL_BEHAVIOR_READOUT
				printf(" Animal %u scavenged some meat for %f.\n" , animalIndex , bloodEfficiency );
#endif
				seedGrid[currentPosition].energy  += bloodEfficiency;
				clearParticle(neighbour);
				eaten = true;
				break;
			}
		}
		break;
	}

	if (seedGrid[neighbour].stage == STAGE_ANIMAL )
	{
		if ((animals[animalIndex].personalityFlags & PERSONALITY_AGGRESSIVE ) == PERSONALITY_AGGRESSIVE  ||    // if the animal is aggressive, or
		        (animals[animalIndex].energyFlags & ENERGYSOURCE_ANIMAL ) == ENERGYSOURCE_ANIMAL ||                // if it eats other animals, or
		        carnageMode)
		{
			// attack the other animal and either try to kill it or take conquest of it.
			if (seedGrid[neighbour].parentIdentity < maxAnimals)
			{
				unsigned int animalIndexB = seedGrid[neighbour].parentIdentity ;
#ifdef ANIMAL_BEHAVIOR_READOUT
				printf("There was a fight! Animal %u fought animal %u\n" , animalIndex, animalIndexB );
#endif
				int damageInflictedB = animals[animalIndex].attack - animals[animalIndexB].defense;
				if (damageInflictedB < 0) // feed on the pain and misery; grow stronger
				{
					damageInflictedB = 0;
				}

#ifdef ANIMAL_BEHAVIOR_READOUT
				printf(" Animal %u dealt %i damage to animal %u\n" , animalIndex ,  damageInflictedB, animalIndexB );
#endif
				animals[animalIndexB].hitPoints -= damageInflictedB;
				eaten = true;
				if (animals[animalIndexB].hitPoints < 0) // the adversary is vanquished mortally
				{
					// if you are a carnivore, kill the opponent
					// it will explode into blood particles that you can then consume;
#ifdef ANIMAL_BEHAVIOR_READOUT
					printf("animal %u killed animal %u\n", animalIndex, animalIndexB);
#endif
					killAnAnimal(neighbour);
				}
				else if (animals[animalIndexB].hitPoints < (animals[animalIndexB].maxHitPoints / 3))
				{
					// if not a carnivore, force the opponent to bear your offspring
#ifdef ANIMAL_BEHAVIOR_READOUT
					printf("animal %u subdued %u and impregnated it!\n", animalIndex, animalIndexB );
#endif
					animals[animalIndexB].hitPoints = 1;
					animals[animalIndexB].partnerGenes = &(seedGrid[neighbour].genes);
					animals[animalIndexB].partnerReproductiveCost = animals[animalIndex].reproductionEnergy;
					animals[animalIndexB].mated = true;
				}
				else
				{
#ifdef ANIMAL_BEHAVIOR_READOUT
					printf("animal %u parried the blow!\n", animalIndexB );
#endif
				}
			}
		}
	}
	return eaten;
}

void animalFeed(unsigned int i)
{
	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex < maxAnimals)
	{
		for (unsigned int segmentIndex = 0; segmentIndex < animals[animalIndex].segmentsUsed; ++segmentIndex)
		{
			unsigned int segmentX = animals[animalIndex].segments[segmentIndex].position % sizeX;
			unsigned int segmentY = animals[animalIndex].segments[segmentIndex].position / sizeX;
			for (unsigned int spriteI = 0; spriteI < squareSizeAnimalSprite; ++spriteI)
			{
				unsigned int spriteX = spriteI % sizeAnimalSprite;
				unsigned int spriteY = spriteI / sizeAnimalSprite;
				unsigned int pixelIndex = (squareSizeAnimalSprite * FRAME_BODY) + spriteI;

				// // feed while you're here
				if ((animals[animalIndex].segments[segmentIndex].frames[pixelIndex] == ORGAN_MOUTH   ) )
				{
					unsigned int worldX = segmentX + spriteX - halfSizeAnimalSprite;
					unsigned int worldY = segmentY + spriteY - halfSizeAnimalSprite;
					unsigned int worldI = ((worldY * sizeX) + worldX ) % totalSize;
					if (worldI != i)
					{
						animalEat(i, worldI);
					}
				}
			}
		}
	}
}

void initialize ()
{
	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	srand((unsigned int)time(NULL));
	clearGrids();
	clearAnimals();
	// animals.push_back(Animal());

	// drawAnimalFromSeed(0);
}

void setEverythingHot()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		if (grid[i].phase != PHASE_VACUUM)
		{
			grid[i].temperature = 5000;
		}
		else {
			grid[i].temperature = 0;
		}
	}
}

void heatEverything ()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		if (grid[i].phase != PHASE_VACUUM)
		{
			if (grid[i].temperature < (0xFFFFFFFF - 25) )
			{
				grid[i].temperature += 25;
			}
			else {
				grid[i].temperature = (0xFFFFFFFF);
			}
		}
	}
}

void coolEverything ()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		if (grid[i].phase != PHASE_VACUUM)
		{
			if (grid[i].temperature > 25)
			{
				grid[i].temperature -= 25;
			}
			else {
				grid[i].temperature = 0;
			}
		}
	}
}

void clearGases()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		if ( grid[i].phase == PHASE_GAS)
		{
			clearParticle(i);
		}
	}
}

void clearLiquids()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		if ( grid[i].phase == PHASE_LIQUID)
		{
			clearParticle(i);
		}
	}
}

void clearAllPressureVelocity()
{
	for (unsigned int weatherGridI = 0; weatherGridI < weatherGridSize; ++weatherGridI)
	{
		weatherGrid[weatherGridI].velocityX = 0;
		weatherGrid[weatherGridI].velocityY = 0;
		weatherGrid[weatherGridI].pressure = defaultPressure;
		weatherGrid[weatherGridI].temperature = defaultTemperature << temperatureScale;
	}
}

void setNeutralTemp ()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		grid[i].temperature = 300;
	}

	for (unsigned int weatherGridI = 0; weatherGridI < weatherGridSize; ++weatherGridI)
	{
		weatherGrid[weatherGridI].temperature = 300 << temperatureScale;
	}
}

unsigned int convertScreenToWorld(unsigned int ux, unsigned int uy)
{
	int x = ux;
	int y = uy;

	// step 1 correct the y axis inversion
	y = 1080 - y;

	// step 2 undo offsets
	// world coordinates 0,0 is at the center of the screen normally
	x -= (1920 / 2);
	y -= (1080 / 2);
	x += (viewPanSetpointX / (viewZoomSetpoint / 1000) )  ;
	y += (viewPanSetpointY / (viewZoomSetpoint / 1000) )  ;

	// step 3 correct the scale
	x = x * ( viewZoomSetpoint / 1000  );
	y = y * ( viewZoomSetpoint / 1000  );

	// step 4 clip to safe settings
	x = abs(x) % sizeX;
	y = abs(y) % sizeY;

	// return vec_u2(x, y);

	return (y * sizeX) + x;
}

void setPlayerMouseCursor(unsigned int x, unsigned int y)
{
	playerMouseCursor = convertScreenToWorld(x, y);
}

void setExtremeTempPoint ()
{

	// unsigned int worldPos = convertScreenToWorld(sx, sy);


	unsigned int x = playerMouseCursor % sizeX;
	unsigned int y = playerMouseCursor / sizeX;

	int blobesize = 1;
	for (int i = -blobesize; i < blobesize; ++i)
	{
		for (int j = -blobesize; j < blobesize; ++j)
		{
			unsigned int weatherGridI =  (( (y + i) / weatherGridScale) * weatherGridSizeX ) + ((x + j) / weatherGridScale);
			weatherGridI = weatherGridI % weatherGridSize;
			weatherGrid[ weatherGridI].pressure += 100000;
			weatherGrid[ weatherGridI].temperature += 100000;
		}
	}

	if (false)
	{
		unsigned int i = ((y * sizeX) + x) % totalSize;
		unsigned int weatherGridI =  ((x / weatherGridScale) * weatherGridSizeX ) + (y / weatherGridScale);
		weatherGrid[ weatherGridI].temperature = 10000000;
		weatherGrid[ weatherGridI].pressure += 1000000;
	}
}






void setPointSize (unsigned int pointSize)
{
	glPointSize(pointSize);
}

void toggleEnergyGridDisplay ()
{
	visualizer = (visualizer + 1 ) % (NUMBER_OF_VISUALIZERS + 1);
}

bool animalCanEat(unsigned int currentPosition , unsigned int neighbour )
{
	unsigned int animalIndex  = seedGrid[currentPosition].parentIdentity;
	if (animalIndex > maxAnimals) {return false;}
	if (currentPosition == neighbour) {return false;}

	if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_SEED ) == ENERGYSOURCE_SEED   )
	{
		if (seedGrid[neighbour].stage == STAGE_BUD || seedGrid[neighbour].stage == STAGE_FRUIT ||  seedGrid[neighbour].stage == STAGE_SPROUT )
		{
			seedGrid[currentPosition].energy  += seedEfficiency ;
			return true;
		}
	}
	if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_PLANT ) == ENERGYSOURCE_PLANT   )
	{
		if (lifeGrid[neighbour].identity > 0x00)
		{
			return true;
		}
	}
	if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_MINERAL ) == ENERGYSOURCE_MINERAL   )
	{
		if (grid[neighbour].phase != PHASE_VACUUM)
		{
			seedGrid[currentPosition].energy  += mineralEfficiency;
			return true;
		}
	}

	if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_ANIMAL ) == ENERGYSOURCE_ANIMAL || carnageMode  ) // always attack other animals in carnage mode.. you don't have to eat them if you dont want to.
	{
		if (grid[neighbour].material == MATERIAL_BLOOD) // or, if there is meat laying around you can eat.
		{
			return true;
		}

	}
	return false;
}

// returns the world pixel that is the most ideal legal move
unsigned int animalDirectionFinding (unsigned int i)
{
	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex > maxAnimals) { return i;}

	// if the animal is the player, just go where the player commands.
	if (firstPerson)
	{
		if (animalIndex == player)
		{
			playerPosition = i;
		}
	}

	unsigned int decidedLocation;
	bool decided = false;
	bool runAway = false;
	bool hungry = false;
	if (seedGrid[i].energy > (animals[animalIndex].reproductionEnergy / 2))
	{
		hungry = true;
	}
	if (animalIndex < maxAnimals)
	{
		// scan the environment using the animal's perception.
		// more perception means the coverage of the area will be more complete (more rolls= less missed stuff)
		// but bigger eyes means the coverage is over a wider area
		for (unsigned int j = 0; j < animals[animalIndex].perception; ++j)
		{
			unsigned int animalX =  i % sizeX;
			unsigned int animalY =  i / sizeX;
			unsigned int worldRandomX = animalX + (extremelyFastNumberFromZeroTo(animals[animalIndex].biggestEye)) - animals[animalIndex].biggestEye;
			unsigned int worldRandomY = animalY + (extremelyFastNumberFromZeroTo(animals[animalIndex].biggestEye)) - animals[animalIndex].biggestEye;
			unsigned int worldRandomI = (((worldRandomY * sizeX) + worldRandomX)) % totalSize;

			// the animal personality types determine how it will react.
			if (worldRandomI == i) {continue;} // to observe the self is gay

			if (  (animals[animalIndex].personalityFlags & PERSONALITY_COWARDLY ) == PERSONALITY_COWARDLY   )
			{
				if (seedGrid[worldRandomI].stage == STAGE_ANIMAL)
				{
					runAway = true;
					decidedLocation = worldRandomI;
					decided = true;
				}
			}

			if (  (animals[animalIndex].personalityFlags & PERSONALITY_FRIENDLY ) == PERSONALITY_FRIENDLY   )
			{
				if (seedGrid[worldRandomI].stage == STAGE_ANIMAL)
				{
					if (!hungry)
					{
						decidedLocation = worldRandomI;
						decided = true;
					}
				}
			}

			if (animalCanEat(i, worldRandomI))
			{
				if (hungry)
				{
					decidedLocation = worldRandomI;
					decided = true;
				}
			}
		}

		if (decided)
		{
			animals[animalIndex].direction = getRelativeDirection (i, decidedLocation);

			if (runAway)
			{
				animals[animalIndex].direction = (animals[animalIndex].direction  + (N_NEIGHBOURS / 2)) % N_NEIGHBOURS; // if you're running away, make sure to head in the opposite direction!
			}

		}
		else
		{
			// check immediate cell neighbours for food
			for (int j = 0; j < N_NEIGHBOURS; ++j)
			{
				unsigned int neighbour = (i + neighbourOffsets[j]) % totalSize;
				if (animalCanEat(i, neighbour))
				{
					animals[animalIndex].direction = j;
					decided = true;
					break;
				}
			}
			if (!decided)
			{
				animals[animalIndex].direction = extremelyFastNumberFromZeroTo(N_NEIGHBOURS);
			}
		}
		unsigned int currentPosition = i;
		for (unsigned int move = 0; move < animals[animalIndex].mobility; ++move)
		{
			currentPosition = getDMostWalkableSquare( i, animalIndex, animals[animalIndex]. direction, currentPosition);
		}
		return currentPosition;
	}
	return 0;
}

void animalReproduce (unsigned int i)
{
	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex > maxAnimals) {return;}

	unsigned int nSolidNeighbours = 0;
	for (unsigned int j = 0; j < N_NEIGHBOURS; ++j)
	{
		unsigned int neighbour = (neighbourOffsets[j] + i) % totalSize;
		if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS || grid[neighbour].phase == PHASE_LIQUID)
		{
			if (animals[animalIndex].mated)
			{
				if (animals[animalIndex].partnerGenes != nullptr)
				{
					setAnimal( neighbour , *(animals[animalIndex].partnerGenes ));
					animals[animalIndex].mated = false;
					animals[animalIndex].partnerGenes = nullptr;
				}
			}
			else
			{
				setAnimal( neighbour , seedGrid[i].genes);
			}

#ifdef MUTATION_READOUT
			printf("animal : ");
#endif
			mutateSentence(  &(seedGrid[neighbour].genes) );
			seedGrid[i].energy = animals[animalIndex].reproductionEnergy * 0.5f;
			seedGrid[neighbour].energy = animals[animalIndex].reproductionEnergy * 0.5;

#ifdef ANIMAL_BEHAVIOR_READOUT
			printf("animal %u reproduced. Now has energy %f, child energy %f\n ", animalIndex , seedGrid[i].energy, seedGrid[neighbour].energy  );
#endif
			animals[animalIndex].timesReproduced++;
			if (animals[animalIndex].timesReproduced > maxTimesReproduced) // if it hit the limit, kill the animal. this is basically to keep the game moving.
			{

#ifdef ANIMAL_BEHAVIOR_READOUT
				printf("animal %u reproduced too many times and was sentenced to death.\n ", animalIndex  );
#endif
				killAnAnimal(i);
			}
			return;
			break;
		}
	}
}

// have all segments slump down under gravity.
void animalAllSegmentsFall(unsigned int i)
{
	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex > maxAnimals) {return;}
	for (unsigned int segmentIndex = 0; segmentIndex < animals[animalIndex].segmentsUsed; ++segmentIndex)
	{
		unsigned int squareBelow = (animals[animalIndex].segments[segmentIndex].position - sizeX) % totalSize;
		if (grid[squareBelow].phase == PHASE_VACUUM || grid[squareBelow].phase == PHASE_GAS || grid[squareBelow].phase == PHASE_LIQUID )
		{
			if (seedGrid[squareBelow].stage == STAGE_NULL)
			{
				animals[animalIndex].segments[segmentIndex].position = squareBelow;
			}
		}
	}
}

void animalCrudOps(unsigned int i)
{
	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex > maxAnimals ) {return;}

	// reproduce
	if (seedGrid[i].energy > animals[animalIndex].reproductionEnergy && animals[animalIndex].age > (animals[animalIndex].reproductionEnergy + 100)  && animalReproductionEnabled)
	{
		animalReproduce(i);
	}

	// die
	if (seedGrid[i].energy < 0.0f)
	{
#ifdef ANIMAL_BEHAVIOR_READOUT
		printf("animal %u ran out of energy and died.\n ", animalIndex  );
#endif
		killAnAnimal(i);
		return;
	}
}

void animalTurn(unsigned int i)
{
	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex > maxAnimals) {return;}
	if (animals[animalIndex].retired) {return;}

	animals[animalIndex].age++;

	bool moved = false;
	animals[animalIndex].steady = false;
	unsigned int recommendedMovePosition = i;

	animalFeed(i);
	unsigned int directionResult = i;
	directionResult = animalDirectionFinding(i);

	animalCanMove(i, i); // running this sets steady if the animal seed itself is sitting in a supportive environment. however, use steady to knock the animal if it gets stuck.
	if ( directionResult != i )
	{
		recommendedMovePosition = directionResult;
		moved = true;
	}
	else
	{
		animals[animalIndex].steady = false;
	}

	// fall if the animal is unsupported
	if (!(animals[animalIndex].steady) || extremelyFastNumberFromZeroTo(100) == 0 ) // small chance to loose footing and slip
	{
		unsigned int fallSquare = ((i - sizeX) + (extremelyFastNumberFromZeroTo(2) - 1 )) % totalSize;
		if (grid[fallSquare].phase == PHASE_VACUUM || grid[fallSquare].phase == PHASE_GAS || grid[fallSquare].phase == PHASE_LIQUID)
		{
			if (fallSquare > sizeX) // don't clip through the bottom
			{
				recommendedMovePosition = fallSquare;
				moved = true;
			}
		}
	}

	// exchange the seed to the new location, if any, and update the drawing
	if (moved)
	{
		seedGrid[i].energy -= ((animals[animalIndex].reproductionEnergy) / movementEfficiency); // every move costs energy proportional to the creature size
		swapSeedParticle(i, recommendedMovePosition);
		incrementAnimalSegmentPositions(animalIndex, recommendedMovePosition );
	}
}

void thread_graphics()
{
#ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
#endif

	preDraw();

	if (visualizer == VISUALIZE_ENERGY)
	{
		unsigned int nVertsToRenderThisTurn = 1 * totalSize;
		long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;
		float * energyColorGrid = new float[totalNumberOfFields];
		unsigned int x = 0;
		unsigned int y = 0;
		for (unsigned int i = 0; i < totalSize; ++i)
		{
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

	else if (visualizer == VISUALIZE_TEMPERATURE)
	{
		unsigned int nVertsToRenderThisTurn = 1 * totalSize;
		long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;
		float * energyColorGrid = new float[totalNumberOfFields];
		unsigned int x = 0;
		unsigned int y = 0;
		for (unsigned int i = 0; i < totalSize; ++i)
		{
			x = i % sizeX;
			if (!x) { y = i / sizeX; }
			float fx = x;
			float fy = y;

			unsigned int weatherGridI = ((y / weatherGridScale) * weatherGridSizeX + (x / weatherGridScale));

			int itemp = ((weatherGrid[weatherGridI].temperature >> temperatureScale) + grid[i].temperature) >> 1;
			float ftemp = itemp;
			ftemp -= 273;
			ftemp = ftemp / (100.0f);

			energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = ftemp;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = ftemp;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = ftemp;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 3 ] = 1.0f;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 4 ] = fx;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 5 ] = fy;

		}
		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, energyColorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);
		postDraw();
		delete [] energyColorGrid;
	}

	else if (visualizer == VISUALIZE_PHASE)
	{
		unsigned int nVertsToRenderThisTurn = 1 * totalSize;
		long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;
		float * energyColorGrid = new float[totalNumberOfFields];
		unsigned int x = 0;
		unsigned int y = 0;
		for (unsigned int i = 0; i < totalSize; ++i)
		{
			x = i % sizeX;
			if (!x) { y = i / sizeX; }
			float fx = x;
			float fy = y;

			if (grid[i].phase == PHASE_VACUUM)
			{
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = 0.0f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = 0.0f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = 0.0f;
			}
			else if (grid[i].phase == PHASE_GAS)
			{
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = 0.25f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = 0.25f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = 0.25f;
			}
			else if (grid[i].phase == PHASE_LIQUID)
			{
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = 0.5f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = 0.5f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = 0.5f;
			}
			else if (grid[i].phase == PHASE_POWDER)
			{
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = 0.75f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = 0.75f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = 0.75f;
			}
			else if (grid[i].phase == PHASE_SOLID)
			{
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = 1.0f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = 1.0f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = 1.0f;
			}
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 3 ] = 1.0f;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 4 ] = fx;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 5 ] = fy;
		}
		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, energyColorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);
		postDraw();
		delete [] energyColorGrid;
	}

	else if (visualizer == VISUALIZE_PRESSURE)
	{
		unsigned int nVertsToRenderThisTurn = 1 * totalSize;
		long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;
		float * energyColorGrid = new float[totalNumberOfFields];
		unsigned int x = 0;
		unsigned int y = 0;
		for (unsigned int i = 0; i < totalSize; ++i)
		{
			x = i % sizeX;
			if (!x) { y = i / sizeX; }
			float fx = x;
			float fy = y;

			unsigned int weatherGridI = ((y / weatherGridScale) * weatherGridSizeX + (x / weatherGridScale));

			energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = abs(weatherGrid[weatherGridI].velocityX / maximumDisplayVelocity )  ; //weatherGrid[i].pressure / maximumDisplayPressure;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = abs(weatherGrid[weatherGridI].velocityY / maximumDisplayVelocity )  ; //weatherGrid[i].pressure / maximumDisplayPressure;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = (weatherGrid[weatherGridI].pressure / maximumDisplayPressure  )  ;
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

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, animationGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

		postDraw();
	}

#ifdef THREAD_TIMING_READOUT
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_graphics " << elapsed.count() << " microseconds." << std::endl;
#endif
}

int drawCharacter ( std::string genes , unsigned int identity)
{
	while (crudOps)
	{
		;
	}

	char c = genes[cursor_string];

#ifdef PLANT_DRAWING_READOUT
	printf("char %c cursor %u  ", c, cursor_string);
#endif

	drawActions++;
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

	case 'm': // set germination material
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		int numberModifier = 0.0f;
		while (!numberModifier) {    numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }
		numberModifier = numberModifier % materials.size();
		cursor_germinationMaterial = numberModifier;
		break;
	}

	case 'q': // set energy source.
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		int numberModifier = 0.0f;
		while (!numberModifier) {    numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }
		cursor_energySource = numberModifier % 4; // or however many there are (plus one);
#ifdef PLANT_DRAWING_READOUT
		printf("set energy source to %u\n", cursor_energySource);
#endif
		break;
	}

	case 't': // branch. a sequence that grows at an angle to the main trunk
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = 0.0f;
		while (!numberModifier) {    numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }
		float branchRotation = ((numberModifier - 13.0f) / 13.0f) * const_pi;
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

	case 'a': // array. a motif is repeated a few times from the same place but with an increasing angle offset.
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {    numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		// rotationIncrement describes what fraction of the whole circle the array occupies.
		float arrayTotalAngle =  numberModifier;
		arrayTotalAngle = arrayTotalAngle / 26;
		arrayTotalAngle = arrayTotalAngle * const_tau;

		numberModifier = 0;
		while (!numberModifier) {    numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

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

	case 'n': // rosette. a motif is repeated with perfect radial symmetry in n divisions.
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {    numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		// rotationIncrement describes what fraction of the whole circle the array occupies.
		float repeats =  numberModifier;
		float rotationIncrement = (const_tau) / repeats;

#ifdef PLANT_DRAWING_READOUT
		printf("rosette, divided %f \n", repeats);
#endif
		float trunkRotation = accumulatedRotation; // rotation of the stem that the array is growing on.

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

	case 's': // sequence. a motif is repeated serially a number of times. it comes with a scaling factor that is applied to every recursion.
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}

		// get number of times to repeat the sequence
		int numberModifier = 0;
		while (!numberModifier) {    numberModifier = alphanumeric( genes[cursor_string] ) / 2  ; cursor_string++; if (cursor_string > genesize) {return -1;} }

		int repeats = numberModifier % 8;

		// the character after that is the next thing to be arrayed
		cursor_string++; if (cursor_string > genesize) {return -1;}

		unsigned int sequenceOrigin = cursor_string;
		prevCursor_grid = cursor_grid;
		float prevScalingFactor = scalingFactor;

		numberModifier = 0;
		while (!numberModifier) {    numberModifier = alphanumeric( genes[cursor_string] )  ; cursor_string++; if (cursor_string > genesize) {return -1;} }

		float newScalingFactor = 1.0f - (numberModifier / 26.0f); // the scaling factor should range between 1 and close to 0, but never less than 0 or bigger than 1.
		if (newScalingFactor > 1.0f) {newScalingFactor = 1.0f;}
		if (newScalingFactor < 0.1f) {newScalingFactor = 0.1f;}
		scalingFactor = newScalingFactor;

#ifdef PLANT_DRAWING_READOUT
		printf("sequence. repeats %i, scalingFactor %f\n", repeats, scalingFactor);
#endif

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
			scalingFactor = scalingFactor * scalingFactor;
			cursor_string = sequenceOrigin;
		}
		recursion_level--;
		scalingFactor = prevScalingFactor;
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

	case 'e': // grow previously drawn segments outward by 1 pixel
	{

#ifdef PLANT_DRAWING_READOUT
		printf("extrude\n");
#endif
		cursor_string++; if (cursor_string > genesize) {return -1;}
		v_extrudedParticles.clear();

		if (extrusion_level > extrusion_limit)
		{
			break;
		}

		for (std::list<ProposedLifeParticle>::iterator it = v.begin(); it != v.end(); ++it)
		{
			unsigned int i = (it->position.y * sizeX) + it->position.x;

			if ( (i < (totalSize - sizeX - 1) ) &&  ( i > (0 + sizeX + 1)  )    )
			{
				unsigned int x = i % sizeX;
				unsigned int y = i / sizeX;
				unsigned int squareBelow = i - sizeX;
				unsigned int squareAbove = i + sizeX;
				unsigned int neighbours[] =
				{
					squareBelow - 1,
					squareBelow,
					squareBelow + 1,
					i + 1,
					squareAbove + 1,
					squareAbove,
					squareAbove - 1,
					i - 1,
				};
				for (unsigned int j = 0; j < N_NEIGHBOURS; ++j)
				{
					unsigned int xScan = neighbours[j] % sizeX;
					unsigned int yScan = neighbours[j] / sizeX;

					// if this location is not already in v
					bool present = false;
					for (std::list<ProposedLifeParticle>::iterator vscan = v.begin(); vscan != v.end(); ++vscan)
					{
						if ( vscan->position.x == xScan && vscan->position.y == yScan )
						{
							present = true;
							break;
						}
					}
					for (std::list<ProposedLifeParticle>::iterator escan = v_extrudedParticles.begin(); escan != v_extrudedParticles.end(); ++escan)
					{
						if ( escan->position.x == xScan && escan->position.y == yScan )
						{
							present = true;
							break;
						}
					}

					if (!present)
					{
						v_extrudedParticles.push_back(   ProposedLifeParticle(cursor_color, vec_u2(xScan, yScan), cursor_energySource)  );
					}
				}
			}
		}

		// put the new extruded particles into v but don't clear v. if you hit extrude again, it will grow another layer.
		v.splice(v.end(), v_extrudedParticles );

		for (std::list<ProposedLifeParticle>::iterator it = v.begin(); it != v.end(); ++it)
		{
			unsigned int i = (it->position.y * sizeX) + it->position.x;
			if ( i < totalSize)
			{
				if (grid[i].phase == PHASE_VACUUM || grid[i].phase == PHASE_GAS || grid[i].phase == PHASE_LIQUID)
				{
					setLifeParticle(  genes, identity, i, it->color, it->energySource);
					if (seedGrid[i].stage != STAGE_ANIMAL)
					{
						clearSeedParticle(i);
					}
					energyDebtSoFar -= 1.0f;
				}
			}
		}
		extrusion_level++;
		break;
	}

	case 'c': // paint a circle at the cursor
	{
#ifdef PLANT_DRAWING_READOUT
		printf("draw a circle\n");
#endif
		v.clear();

		cursor_string++; if (cursor_string > genesize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {    numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		// get the circle radius (the next number in the string). nerfed on purpose otherwise there are giant blobs everywhere.
		unsigned int radius = (numberModifier / 2) * scalingFactor;

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
					v.push_back( ProposedLifeParticle(cursor_color, vec_u2(i, j), cursor_energySource ));
				}
			}
		}

		for (std::list<ProposedLifeParticle>::iterator it = v.begin(); it != v.end(); ++it)
		{
			unsigned int i = (it->position.y * sizeX) + it->position.x;
			if ( i < totalSize)
			{
				if (grid[i].phase == PHASE_VACUUM || grid[i].phase == PHASE_GAS || grid[i].phase == PHASE_LIQUID)
				{
					setLifeParticle(  genes, identity, i, it->color, it->energySource);
					energyDebtSoFar -= 1.0f;

					if (seedGrid[i].stage != STAGE_FRUIT && seedGrid[i].stage != STAGE_ANIMAL)
					{
						clearSeedParticle(i);
					}
				}
			}
		}

		extrusion_level = 0;
		break;
	}

	case 'l':
	{
#ifdef PLANT_DRAWING_READOUT
		printf("draw a line (with angle and length noise)");
#endif
		v.clear();

		// rasters a line, taking into account the accumulated rotation
		cursor_string++; if (cursor_string > genesize) {return -1;}

		// set the previous x and y, which will be the start of the line
		prevCursor_grid = cursor_grid;
		int numberModifier = 0;
		numberModifier = alphanumeric( genes[cursor_string] ) ;

#ifdef PLANT_DRAWING_READOUT
		printf("length %i \n", numberModifier);
#endif

		cursor_string++; if (cursor_string > genesize) {return -1;}

		accumulatedRotation += ( RNG() - 0.5 ) * 0.1; // angle noise is not heritable, and just serves to make the world more natural-looking.

		int deltaX = ((numberModifier + ( numberModifier * lengthNoise)) * scalingFactor) * cos(accumulatedRotation);
		int deltaY = ((numberModifier + ( numberModifier * lengthNoise)) * scalingFactor) * sin(accumulatedRotation);

		bool wrap = false;

		cursor_grid.x += deltaX;
		cursor_grid.y += deltaY;

		v.splice(v.end(), EFLA_E( vec_i2(prevCursor_grid.x, prevCursor_grid.y), vec_i2(cursor_grid.x, cursor_grid.y)) );

		for (std::list<ProposedLifeParticle>::iterator it = v.begin(); it != v.end(); ++it)
		{
			unsigned int i = (it->position.y * sizeX) + it->position.x;
			if ( i < totalSize)
			{
				if (grid[i].phase == PHASE_VACUUM || grid[i].phase == PHASE_GAS || grid[i].phase == PHASE_LIQUID)
				{
					setLifeParticle(  genes, identity, i, it->color, it->energySource);
					energyDebtSoFar -= 1.0f;

					if (seedGrid[i].stage != STAGE_FRUIT && seedGrid[i].stage != STAGE_ANIMAL)
					{
						clearSeedParticle(i);
					}
				}
			}
		}
		extrusion_level = 0;
		break;
	}

	case 'r': // set immediate red value
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

	case 'g': // set immediate green value
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

	case 'b': // set immediate blue value
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

	case 'x': // set seed red
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

	case 'y': // set seed green
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

	case 'z': // set seed blue
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

	case 'o': // modify angle
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		float numberModifier = alphanumeric( genes[cursor_string] );
		numberModifier = ((numberModifier - 13) / 13) * const_pi * 0.5;
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


void drawGplantFromChar()
{
// gplant gene codes :

// extrude (grow) the tissue that the growing point sits in

// grow tissues immediately around the growing point

//

// set crystal n

// set crystal condition

// set tissue type to create

// set tissue type color r
// set tissue type color g
// set tissue type color b

// move the growing point in direction




// the possible tissues are:
	// wood, transports energy, strong and inedible, does not photosynthesize
	// leaf, transports and produces energy, edible by predators
	// flower, colorful
	// seed, falls off the plant to make new plants.
	// growing point


}

void drawGplant( unsigned int i )
{

	// this is a gplant.

	// gplants use the crystal rules to grow, making complex shapes from simple rules.

	// they grow over time instead of instantly.

	// emplace tissue of that type

}



void drawPlantFromSeed( std::string genes, unsigned int i )
{
	while (crudOps) {;}

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
	accumulatedRotation = (0.5 * const_pi);
	cursor_germinationMaterial =  0;
	cursor_energySource = ENERGYSOURCE_LIGHT;

	float randomN =  extremelyFastNumberFromZeroTo(100) ;

	lengthNoise = ( (randomN / 100) - 0.5 ) * 0.25 ;

	// color noise helps a forest to look a bit more interesting. It is not genetically heritable.
	randomN =  extremelyFastNumberFromZeroTo(100) ;
	float colorNoiseLevel = ((randomN / 100) - 0.5) * 0.1;

	cursor_color =  Color(
	                    color_defaultColor.r + colorNoiseLevel,
	                    color_defaultColor.g + colorNoiseLevel,
	                    color_defaultColor.b + colorNoiseLevel,
	                    1.0f
	                );

	randomN =  extremelyFastNumberFromZeroTo(100) ;
	float seedColorNoiseLevel = ((randomN / 100) - 0.5) * 0.1;

	cursor_seedColor =  Color(
	                        color_defaultSeedColor.r + seedColorNoiseLevel,
	                        color_defaultSeedColor.g + seedColorNoiseLevel,
	                        color_defaultSeedColor.b + seedColorNoiseLevel,
	                        1.0f
	                    );
	drawActions = 0;

	v.clear();
	v_extrudedParticles.clear();
	v_seeds.clear();

	energyDebtSoFar = 0.0f;

	unsigned int count = 0;

	while ( true )
	{
		if ( drawCharacter (genes , identity) < 0) // this is where the plant drawings get made.
		{
			if (cursor_string > genes.length())
			{
				break;
			}
		}

		if (count > maxGenomeSize)
		{
			break;
		}

		count++;
	}

	for (std::list<vec_u2>::iterator it = v_seeds.begin(); it != v_seeds.end(); ++it)
	{
		unsigned int i = (it->y * sizeX) + it->x;
		if ( i < totalSize)
		{
			if (grid[i].phase == PHASE_VACUUM || grid[i].phase == PHASE_GAS || grid[i].phase == PHASE_LIQUID)
			{
				setSeedParticle(  genes, identity, energyDebtSoFar, i, cursor_germinationMaterial);
#ifdef MUTATION_READOUT
				printf("plant  : ");
#endif
				mutateSentence(&(seedGrid[i].genes));
			}
		}
	}
}

void thread_life()
{
#ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
#endif
	unsigned int x = 0;
	unsigned int y = 0;
	for (unsigned int i = (sizeX + 1); i < (totalSize - (sizeX + 1)); ++i)
	{
		if ( lifeGrid[i].identity)
		{
			unsigned int x = i % sizeX;
			unsigned int y = i / sizeX;
			unsigned int weatherGridI =  (( (y ) / weatherGridScale) * weatherGridSizeX ) + ((x ) / weatherGridScale);
			if (weatherGridI < weatherGridSize)
			{
				if ( lifeGrid[i].energySource == ENERGYSOURCE_LIGHT ) // if the plant is illuminated, it receives energy.
				{
					lifeGrid[i].energy += weatherGrid[weatherGridI].color.a / lightEfficiency;
				}
			}

			unsigned int neighbourMaterialA = MATERIAL_VACUUM;
			unsigned int neighbour = (neighbourOffsets[ extremelyFastNumberFromZeroTo(N_NEIGHBOURS) ] + i) % totalSize;

			// if there is a neighbouring cell from the same plant, equalize energy with it.
			if (lifeGrid[neighbour].identity == lifeGrid[i].identity)
			{
				float equalizedEnergy = ( lifeGrid[neighbour].energy + lifeGrid[i].energy ) / 2;
				lifeGrid[neighbour].energy = equalizedEnergy;
				lifeGrid[i].energy = equalizedEnergy;
			}

			// some cells can extract energy if they are between dissimilar materials.
			if ( lifeGrid[i].energySource == ENERGYSOURCE_MINERAL )
			{
				if (neighbourMaterialA != MATERIAL_VACUUM)
				{
					if (grid[i].material != neighbourMaterialA) // you have found a pair of dissimilar neighbours
					{
						lifeGrid[i].energy +=  mineralEfficiency;
						clearLifeParticle(neighbour);
						neighbourMaterialA = MATERIAL_VACUUM; // reset for another go around.
					}
				}
				else if (grid[i].material != MATERIAL_VACUUM)
				{
					neighbourMaterialA = grid[i].material;
				}
			}

			// some cells can steal energy from neighbouring plants.
			else if (lifeGrid[i].energySource == ENERGYSOURCE_PLANT )
			{
				if (lifeGrid[neighbour].identity != lifeGrid[i].identity )
				{
					if (lifeGrid[neighbour].energy > 0.0f)
					{
						lifeGrid[i].energy += plantEfficiency ;
						clearLifeParticle(neighbour);
					}
				}
			}

			// some cells can consume piles of old seeds on the ground..
			else if (lifeGrid[i].energySource == ENERGYSOURCE_SEED )
			{
				if (seedGrid[neighbour].stage < 0x00)
				{
					if (seedGrid[neighbour].parentIdentity != lifeGrid[i].identity )
					{
						lifeGrid[i].energy += seedEfficiency;
						clearLifeParticle(neighbour);
					}
				}
			}
		}
	}

#ifdef THREAD_TIMING_READOUT
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_life " << elapsed.count() << " microseconds." << std::endl;
#endif
}

void thread_plantDrawing()
{
#ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
#endif
	for (unsigned int i =  0; i < totalSize; ++i)
	{
		if (seedGrid[i].stage == STAGE_SPROUT)
		{
			drawPlantFromSeed(seedGrid[i].genes, i);
			clearSeedParticle(i);
			continue;
		}
	}
#ifdef THREAD_TIMING_READOUT
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_plantDrawing " << elapsed.count() << " microseconds." << std::endl;
#endif
}

void thread_seeds()
{
#ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
#endif
	for (unsigned int i = (sizeX + 1); i < (totalSize - (sizeX + 1)); ++i)
	{
		// SEEDS. Some of the particles on the seed grid are seeds that fall downwards.
		if (seedGrid[i].stage == STAGE_FRUIT)
		{
			unsigned int j = extremelyFastNumberFromZeroTo(4);
			unsigned int neighbour = i + neighbourOffsets[j];
			if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  )
			{
				swapSeedParticle( i, neighbour );
				continue;
			}
			else if (grid[neighbour].material == seedGrid[i].germinationMaterial  || (!useGerminationMaterial)  )
			{

#ifdef PLANT_DRAWING_READOUT
				printf("germinated\n");
#endif
				seedGrid[i].stage = STAGE_SPROUT;
			}
			continue;
		}

		else if (seedGrid[i].stage == STAGE_BUD)
		{
			unsigned int j = extremelyFastNumberFromZeroTo(7);
			unsigned int neighbour = i + neighbourOffsets[j];
			if (lifeGrid[neighbour].identity == seedGrid[i].parentIdentity)
			{
				if (lifeGrid[neighbour].energy > 0)
				{
					seedGrid[i].energy += (lifeGrid[neighbour].energy );
					lifeGrid[neighbour].energy = (lifeGrid[neighbour].energy );
				}
			}

			if (seedGrid[i].energy >= 0)
			{
				seedGrid[i].stage = STAGE_FRUIT;

#ifdef PLANT_DRAWING_READOUT
				printf("fruited\n");
#endif
			}
			continue;
		}

		else if (seedGrid[i].stage == STAGE_ANIMAL)
		{
			animalTurn(i);
			continue;
		}

		else if (seedGrid[i].stage == STAGE_ANIMALEGG)
		{
			// animalTurn(i);
			drawAnimalFromSeed(i);

			continue;
		}




		else if (seedGrid[i].stage == STAGE_GPLANTSHOOT)
		{
			drawGplant(i);
			continue;
		}

		else
		{
			continue;
		}
	}
#ifdef THREAD_TIMING_READOUT
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_seeds " << elapsed.count() << " microseconds." << std::endl;
#endif
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
			std::string exampleSentence = std::string("");

			unsigned int randomPlantIndex = extremelyFastNumberFromZeroTo(7);

			switch (randomPlantIndex)
			{
			case 0:
			{
				// exampleSentence = plant_Pycad;
				break;
			}
			case 1:
			{
				// exampleSentence = plant_Lomondra;
				break;
			}
			case 2:
			{
				// exampleSentence = plant_Worrage;
				break;
			}
			case 3:
			{
				// exampleSentence = plant_MilkWombler;
				break;
			}
			case 4:
			{
				// exampleSentence = plant_SpleenCoral;
				break;
			}
			case 5:
			{
				// exampleSentence = plant_ParbasarbTree;
				break;
			}
			case 6:
			{
				// exampleSentence = plant_LardGrass;
				break;
			}
			case 7:
			{
				exampleSentence = plant_Primordial;
				break;
			}
			}

			unsigned int randomGerminationMaterial = 0;
			if (materials.size() > 0)
			{
				randomGerminationMaterial = extremelyFastNumberFromZeroTo(materials.size() - 1);
				setSeedParticle(exampleSentence, newIdentity() , 0, i, randomGerminationMaterial);
				seedGrid[i].stage = STAGE_FRUIT;
			}
		}
	}
}

void insertRandomAnimal ()
{
	// unsigned int x = 0;
	// unsigned int y = 0;
	unsigned int x = extremelyFastNumberFromZeroTo(sizeX);
	unsigned int y = extremelyFastNumberFromZeroTo(sizeY / 2) + (sizeY / 2);
	unsigned int i = (y * sizeX ) + x;

	// for (unsigned int i = (sizeX + 1); i < (totalSize - (sizeX + 1)); ++i)
	// {
	// 	x = i % sizeX;
	// 	if (!x) { y = i / sizeX; }
	// 	if (x == targetX && y == targetY)
	// 	{
	setAnimal( i, exampleAnimal );
	// 	}
	// }
}


void insertPlayer()
{
	// unsigned int x = 0;
	// unsigned int y = 0;
	unsigned int x = extremelyFastNumberFromZeroTo(sizeX);
	unsigned int y = extremelyFastNumberFromZeroTo(sizeY / 2) + (sizeY / 2);
	unsigned int i = (y * sizeX ) + x;

	// for (unsigned int i = (sizeX + 1); i < (totalSize - (sizeX + 1)); ++i)
	// {
	// 	x = i % sizeX;
	// 	if (!x) { y = i / sizeX; }
	// 	if (x == targetX && y == targetY)
	// 	{
	setAnimal( i, exampleAnimal );
	// 	}
	// }

	playerPosition = i;
	firstPerson = true;
}

// void increaseLampBrightness ()
// {
// 	sunlightBrightness ++;
// 	if (sunlightBrightness > maxLampBrightness) {sunlightBrightness = 0;}
// }

// void decreaseLampBrightness ()
// {
// 	sunlightBrightness--;

// 	if (sunlightBrightness > maxLampBrightness) {sunlightBrightness = 0;}
// }

void save ()
{
	printf("SAVING GAME\n");

	WorldInformation newWorldInfo;
	newWorldInfo.nAnimals   = maxAnimals;
	newWorldInfo.nMaterials = materials.size();

	std::ofstream out884(std::string("save/WorldInformation").c_str());
	out884.write( (char*)(&newWorldInfo), sizeof(WorldInformation));
	out884.close();
	printf("- saved world information\n");

	// transcribe the entire world state to file.
	std::ofstream out6(std::string("save/grid").c_str());
	out6.write( (char*)(grid), sizeof(Particle) *  totalSize);
	out6.close();

	// life grid
	// you must make a 'transportable life grid' which unlike the real lifegrid does not contain any pointer information.
	transportableLifeParticle* transportableLifeGrid = new transportableLifeParticle[totalSize];
	std::memset(transportableLifeGrid, 0x00, sizeof(transportableLifeParticle) * totalSize);

	for (unsigned int i = 0; i < totalSize; ++i)
	{
		transportableLifeGrid[i] = transportableLifeParticle();
		transportableLifeGrid[i].identity = lifeGrid[i].identity;
		transportableLifeGrid[i].energy = lifeGrid[i].energy;
		transportableLifeGrid[i].energySource = lifeGrid[i].energySource;
	}
	std::ofstream out9(std::string("save/transportableLifeGrid").c_str());
	out9.write(  (char*)(transportableLifeGrid), sizeof(transportableLifeParticle) * totalSize);
	out9.close();

	delete [] transportableLifeGrid;

	printf("- saved transportable life grid\n");

	// // seed grid
	// // you must make a 'transportable grid' which unlike the real seedgrid does not contain any pointer information.
	transportableSeed* transportableSeedGrid = new transportableSeed[totalSize];
	std::memset(transportableSeedGrid, 0x00, sizeof(transportableSeed) * totalSize);

	for (unsigned int i = 0; i < totalSize; ++i)
	{
		transportableSeedGrid[i] = transportableSeed();
		transportableSeedGrid[i].parentIdentity = seedGrid[i].parentIdentity;
		transportableSeedGrid[i].energy = seedGrid[i].energy;
		transportableSeedGrid[i].stage = seedGrid[i].stage;
	}
	std::ofstream out2(std::string("save/transportableSeedGrid").c_str());
	out2.write(  (char*)(transportableSeedGrid), sizeof(transportableSeed) * totalSize);
	out2.close();

	delete [] transportableSeedGrid;

	printf("- saved transportable seed grid\n");

	std::ofstream out5("save/lifeGeneGrid");
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		out5 << lifeGrid[i].genes.c_str() << '\n';
	}

	out5.close();

	printf("- saved life gene grid\n");

	std::ofstream out89("save/seedGeneGrid");
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		out89 << seedGrid[i].genes.c_str()  << '\n';
	}
	out89.close();

	printf("- saved seed gene grid\n");

	// color grids
	std::ofstream out3(std::string("save/lifeColorGrid").c_str());
	out3.write(reinterpret_cast<char*>(& (lifeColorGrid[0]) ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	out3.close();

	std::ofstream out41(std::string("save/seedColorGrid").c_str());
	out41.write(reinterpret_cast<char*>(& (seedColorGrid[0]) ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	out41.close();

	printf("- saved color grids\n");

	// you need to save the materials and animals vectors as well somehow, even though their size is unknown.
	std::ofstream out879("save/materials");
	const char* pointer = reinterpret_cast<const char*>(&materials[0]);
	size_t bytes = materials.size() * sizeof(Material);
	out879.write(pointer, bytes);
	out879.close();

	std::ofstream out8579("save/animals");
	pointer = reinterpret_cast<const char*>(&animals[0]);
	bytes = maxAnimals * sizeof(Animal);
	out8579.write(pointer, bytes);
	out8579.close();
}

void load_materials(unsigned int m)
{
	// load the materials
	if (true)
	{
		// resetMaterials();
		materials.clear();
		for (int i = 0; i < m; ++i)
		{
			materials.push_back(Material());
		}

		std::ifstream in56(std::string("save/materials").c_str());
		in56.read( (char *)(&(materials[0])), sizeof(Material) *  m);
		in56.close();
	}

	std::ifstream in6(std::string("save/grid").c_str());
	in6.read( (char *)(&(grid[0])), sizeof(Particle) *  totalSize);
	in6.close();
	printf("loaded material grid and material colors\n");
}

void load_animals(unsigned int m)
{
	if (true)
	{
		clearAnimals();
		// for (int i = 0; i < m; ++i)
		// {
		// 	animals.push_back(Animal());
		// }
		std::ifstream in556(std::string("save/animals").c_str());
		in556.read( (char *)(&(animals[0])), sizeof(Animal) *  m);
		in556.close();
	}
	printf("loaded animals\n");
}

void load_colorgrids()
{
	// not necessary to save or load the material color grid, it will be regenerated in a few seconds.
	// std::ifstream in3(std::string("save/colorGrid").c_str());
	// in3.read( (char *)( &(colorGrid[0])  ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	// in3.close();

	std::ifstream in33(std::string("save/lifeColorGrid").c_str());
	in33.read( (char *)( &(lifeColorGrid[0])  ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	in33.close();

	std::ifstream in41(std::string("save/seedColorGrid").c_str());
	in41.read( (char *)( &(seedColorGrid[0])), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	in41.close();

	printf("- loaded color grids\n");
}

void load_life()
{
	transportableLifeParticle* transportableLifeGrid = new transportableLifeParticle[totalSize];
	std::ifstream in22(std::string("save/transportableLifeGrid").c_str());
	in22.read( (char*)( &(transportableLifeGrid[0])), sizeof(transportableLifeParticle) * totalSize);
	in22.close();
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		lifeGrid[i] = LifeParticle();
		lifeGrid[i].energySource = transportableLifeGrid[i].energySource;
		lifeGrid[i].energy = transportableLifeGrid[i].energy;
		lifeGrid[i].identity = transportableLifeGrid[i].identity;
	}

	delete [] transportableLifeGrid;

	printf("- loaded transportable life grid\n");

	// load life genes
	if (true)
	{
		std::ifstream in5(std::string("save/lifeGeneGrid").c_str());
		std::string line = std::string("");
		unsigned int i = 0;
		for (std::string line; std::getline(in5, line, '\n'); )
		{
			lifeGrid[i].genes.clear();
			if (i < totalSize)
			{
				lifeGrid[i].genes.assign(line);
			}
			in5.clear();
			i++;
		}
		printf("- loaded life genomes\n");
	}
}

void load_seeds()
{
	transportableSeed* transportableSeedGrid = new transportableSeed[totalSize];
	std::ifstream in2(std::string("save/transportableSeedGrid").c_str());
	in2.read( (char*)(  &(transportableSeedGrid[0])), sizeof(transportableSeed) * totalSize);
	in2.close();
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		seedGrid[i].stage = transportableSeedGrid[i].stage;
		seedGrid[i].energy = transportableSeedGrid[i].energy;
		seedGrid[i].parentIdentity = transportableSeedGrid[i].parentIdentity;
	}

	delete [] transportableSeedGrid;

	printf("- loaded transportable seeds\n");

	// load seed genes
	if (true)
	{
		std::ifstream in500(std::string("save/seedGeneGrid").c_str());
		std::string line = std::string("");
		unsigned int i = 0;
		for (std::string line; std::getline(in500, line, '\n'); )
		{
			if (i < totalSize)
			{
				seedGrid[i].genes.assign(line);
			}
			in500.clear();
			i++;
		}
		printf("- loaded seed genomes\n");
	}
}

void load ()
{
	printf("LOAD GAME\n");
	clearGrids() ;
	printf("- cleared grids\n");
	WorldInformation newWorldInfo;
	std::ifstream in336(std::string("save/WorldInformation").c_str());
	in336.read( (char *)(&newWorldInfo), sizeof(WorldInformation) );
	in336.close();
	printf("- loaded world information\n");
	load_colorgrids();
	load_materials(newWorldInfo.nMaterials);
	load_life();
	load_seeds();
	load_animals(newWorldInfo.nAnimals);
}

void eraseFallenSeeds()
{
	// clean up all the fallen seeds in the world. it can be annoying if they accumulate too much.
	for (int i = 0; i < totalSize; ++i)
	{
		if (seedGrid[i].stage == STAGE_FRUIT || seedGrid[i].stage == STAGE_SPROUT )
		{
			clearSeedParticle(i);
		}
	}
}

void dropAllSeeds()
{
	for (int i = 0; i < totalSize; ++i)
	{
		if (seedGrid[i].stage == STAGE_BUD)
		{
			seedGrid[i].energy = 0.0f;
		}
	}
}

void eraseAllLife()
{
	for (int i = 0; i < totalSize; ++i)
	{
		if (seedGrid[i].stage == STAGE_BUD || seedGrid[i].stage == STAGE_FRUIT || seedGrid[i].stage == STAGE_SPROUT )
		{
			clearSeedParticle(i);
		}

		if (seedGrid[i].stage == STAGE_ANIMAL )
		{
			killAnAnimal(i);
		}

		if (lifeGrid[i].identity > 0x00)
		{
			clearLifeParticle(i);
		}
	}
}

void manualErode ()	// every solid square with a vacuum neighbour is crushed to powder.
{
	for (int i = 0; i < totalSize; ++i)
	{
		for (unsigned int j = 0; j < N_NEIGHBOURS; ++j)
		{
			unsigned int neighbour = neighbourOffsets[j] + i;

			neighbour = neighbour % totalSize;

			if (grid[neighbour].phase == PHASE_VACUUM )
			{
				if (grid[i].phase == PHASE_SOLID)
				{
					grid[i].phase = PHASE_POWDER;
					continue;

				}
			}
		}
	}
}

void drawAHill(unsigned int hillXIndex, unsigned int hillWidth)
{
	// a part of the map is located. the ground there is lifted in a triangular manner.
	for (unsigned int j = 0; j < hillWidth; ++j)
	{
		for (unsigned int k = sizeY; k > 0 ; --k)
		{
			unsigned int indexA = (k * sizeX) + (hillXIndex + j - (hillWidth / 2)) + sizeX;
			unsigned int indexB = (k * sizeX) + (hillXIndex + j - (hillWidth / 2)) ;
			unsigned int indexC = extremelyFastNumberFromZeroTo(sizeX);
			if (indexA > totalSize || indexB > totalSize) {continue;}
			swapParticle( indexA, indexB); // centered on hillXIndex, move all cells that are hillWidth in either direction up by 1 square.
			swapParticle( indexB, indexC);
		}
	}
}

