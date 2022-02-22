#include "deepgarden.h"
#include "deepgarden_graphics.h"

#include <ctime>
#include <chrono>
#include <iostream>
#include <fstream>

#include "main.h"

// #define THREAD_TIMING_READOUT 1
// #define PLANT_DRAWING_READOUT 1
// #define ANIMAL_DRAWING_READOUT 1
// #define ANIMAL_BEHAVIOR_READOUT 1
// #define MUTATION_READOUT 1

bool useGerminationMaterial = false;
bool animalReproductionEnabled = true;
bool doWeather = true;
bool carnageMode = false;

#define RENDERING_THREADS 4

const Color color_lightblue 		= Color( 0.1f, 0.3f, 0.65f, 1.0f );
const Color color_yellow    		= Color( 1.0f, 1.0f, 0.0f, 1.0f );
const Color color_lightgrey 		= Color( 0.75f, 0.75f, 0.75f, 1.0f );
const Color color_grey      		= Color( 0.50f, 0.50f, 0.50f, 1.0f );
const Color color_darkgrey  		= Color( 0.25f, 0.25f, 0.25f, 1.0f );
const Color color_black     		= Color( 0.0f, 0.0f, 0.0f, 1.0f );
const Color color_white_clear     	= Color( 1.0f, 1.0f, 1.0f, 0.25f );
const Color color_purple     		= Color( 0.8f, 0.0f, 0.8f, 1.0f );
const Color color_orange			= Color( 1.0f, 0.8f, 0.0f, 1.0f);
const Color color_clear     		= Color( 0.0f, 0.0f, 0.0f, 0.0f );
const Color color_shadow 	    	= Color( 0.0f, 0.0f, 0.0f, 0.5f);
const Color color_defaultSeedColor  = Color( 0.75f, 0.35f, 0.1f, 1.0f );
const Color color_defaultColor     	= Color( 0.35f, 0.35f, 0.35f, 1.0f );

const Color tingeShadow = Color( -1.0f, -1.0f, -1.0f, 0.1f );
const Color phaseTingeSolid =  Color( 1.0f, 1.0f, 1.0f, 0.2f );
const Color phaseTingeLiquid = Color( -1.0f, -1.0f, -1.0f, 0.2f );
const Color phaseTingeGas =    Color( -1.0f, -1.0f, -1.0f, 0.4f );

const Color color_offwhite          = Color( 0.9f, 1.0f, 0.8f, 1.0f );
const Color color_brightred			= Color( 0.9f, 0.1f, 0.0f, 1.0f);
const Color color_darkred			= Color( 0.5f, 0.05f, 0.0f, 1.0f);
const Color color_brown             = Color(  0.25f, 0.1f, 0.0f, 1.0f );

int neighbourOffsets[] =
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

int spriteNeighbourOffsets[] =
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

const unsigned int totalSize = sizeX * sizeY;
const unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

float * colorGrid = new float[totalSize * numberOfFieldsPerVertex ];		// the colorgrid is like a painting of the game world which can be drawn to the screen easily, and updated as the game is played. it concerns the physical material.
float * lifeColorGrid  	= new float[totalSize * numberOfFieldsPerVertex ];  // the same but concerning growing plants. Because plant color information is not easily stored anywhere else, this grid must be preserved in save and load operations.
float * animationGrid 	= new float[totalSize * numberOfFieldsPerVertex ];	// the same, but for the sprites of animals. This is updated every turn, and animals carry their own copy of their sprites, so this does not need to be preserved.
float * seedColorGrid  	= new float[totalSize * numberOfFieldsPerVertex];   // the same, but for the colors of seeds and falling photons.
float * ppGrid       	= new float[totalSize * numberOfFieldsPerVertex];
float * backgroundSky  	= new float[totalSize * numberOfFieldsPerVertex];

// PLANT DRAWING
unsigned int recursion_level = 0;
const unsigned int recursion_limit = 4;
unsigned int extrusion_level = 0;
const unsigned int extrusion_limit = 13;
unsigned int drawActions = 0;
const unsigned int drawActionlimit = 200;
float accumulatedRotation = (0.5 * 3.1415);
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

// ANIMAL DRAWING
unsigned int animalCursorFrame = FRAME_BODY;
unsigned int animalCursorString = 0;
unsigned int animalCursorSegmentRadius = 2;
float animalCursorSegmentAngle = 0.0f;

unsigned int animalCursorX = 0;
unsigned int animalCursorY = 0;

unsigned int animalCursorExtrusionN = 0;
unsigned int animalCursorExtrusionCondition = CONDITION_GREATERTHAN;

Color animalCursorColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
unsigned int animalCursorSegmentNumber = 0;
unsigned int animalCursorEnergySource = ENERGYSOURCE_PLANT;
unsigned int animalRecursionLevel = 0;

unsigned int animalCursorOrgan = ORGAN_MUSCLE;

std::string exampleAnimal = std::string(" uaiajbcmemjccded ");

int defaultTemperature = 300;
int radiantHeatIntensity = 50; // this is a physical constant that determines how much heat radiates from material, and how strongly material heat is coupled to the atmosphere.

float combinedGasLawConstant = 0.001f;

unsigned int sunlightDirection = 2;
unsigned int sunlightEnergy = 10;
unsigned int sunlightHeatCoeff = 1;
unsigned int sunlightTemp = 1000;
unsigned int sunlightPenetrationDepth = 20; // light is slightly reduced traveling through solid things. this affects plants in game as well as being an artistic effect. This number is how far the light goes into solid things.

Color sunlightColor = color_white_clear;

unsigned int nGerminatedSeeds = 0;
unsigned int lampBrightness = 10;

// raw energy values are DIVIDED by these numbers to get the result. So more means less.
unsigned int lightEfficiency   = 10000;
float movementEfficiency = 5000.0f;

// except for these, where the value is what you get from eating a square.
float meatEfficiency    = 1.0f;
float bloodEfficiency   = 1.0f;
float seedEfficiency    = 1.0f;
float mineralEfficiency = 1.0f;
float plantEfficiency   = 1.0f;

vec_u2 playerCursor = vec_u2(0, 0);

float maximumDisplayEnergy = 1.0f;
float maximumDisplayTemperature = 2000.0f;
float maximumDisplayPressure = 2000.0f;
float maximumDisplayVelocity = 1.0f;

unsigned int visualizer = VISUALIZE_MATERIAL;

unsigned int identityCursor = 0;

std::vector<unsigned int> animal_identities;

std::list<vec_u2> v_seeds;

unsigned int animationChangeCount = 0;
unsigned int animationGlobalFrame = FRAME_BODY;

struct Weather
{
	// int temperature;
	// // float pressure;
	// int pressure;
	// int direction;

	float temperature;
	float pressure;
	float velocityX;
	float velocityY;
	unsigned int direction;

	Weather();
};
Weather::Weather()
{
	this->temperature = 300.0f;
	this->pressure = 1.0f;
	this->direction = 0;
	this->velocityX = 0.0f;
	this->velocityY = 0.0f;
}

Weather weatherGrid[totalSize];


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
	unsigned int temperature;
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

void thread_weather()
{
#ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
#endif
	if (doWeather)
	{

		float airTimestep = 0.5f;
		maximumDisplayPressure = 0.0f;
		maximumDisplayVelocity = 0.0f;

		for (unsigned int x = 1; x < sizeX-1 ; ++x)
		{

			for (unsigned int y = 1; y < sizeY -1; ++y)
			{

				unsigned int i = (y * sizeX) + x;

				float dp = 0.0f;
				float dx = 0.0f;
				float dy = 0.0f;


				float ploss = 0.99f;
				float vloss = 0.99f;

				float advDistanceMult = 1.0f;

				// pressure adjustments from velocity
				dp += (weatherGrid[ i + neighbourOffsets[0] ].velocityX   - weatherGrid[i].velocityX   );
				dp += (weatherGrid[ i + neighbourOffsets[2] ].velocityY   - weatherGrid[i].velocityY   );
				weatherGrid[i].pressure *= ploss;
				weatherGrid[i].pressure += dp * airTimestep;


				// velocity adjustments from pressure
				dx += weatherGrid[i].pressure - weatherGrid[ i + neighbourOffsets[4] ].pressure;
				dy += weatherGrid[i].pressure - weatherGrid[ i + neighbourOffsets[6] ].pressure;
				// float vloss = 0.0f;
				weatherGrid[i].velocityX *= vloss;
				weatherGrid[i].velocityY *= vloss;
				weatherGrid[i].velocityX += dx * airTimestep;
				weatherGrid[i].velocityY += dy * airTimestep;

				// // smoothing 
				// float f = 0.9f;
				int kernelSize = 3;
				int count = 0;
				dp = 0.0f;
				dx = 0.0f;
				dy = 0.0f;
				for (int n = 0; n < N_NEIGHBOURS; ++n)
				{
					unsigned int neighbour = (i + neighbourOffsets[n])  ;//extremelyFastNumberFromZeroTo(N_NEIGHBOURS);
					dp  +=  weatherGrid[ neighbour ].pressure ;
					dx  +=  weatherGrid[ neighbour ].velocityX; // * f;
					dy  +=  weatherGrid[ neighbour ].velocityY; // * f;
					// float pdiff =
					count++;
				}
				dx = dx / count;
				dy = dy / count;
				dp = dp / count;
				weatherGrid[i].pressure  += (dp - weatherGrid[i].pressure)  * 0.5f;
				weatherGrid[i].velocityX += (dx - weatherGrid[i].velocityX) * 0.5f;
				weatherGrid[i].velocityY += (dy - weatherGrid[i].velocityY) * 0.5f;




				// take velocity from far away
				dp = 0.0f;
				dx = 0.0f;
				dy = 0.0f;

				int vxf = weatherGrid[i].velocityX  ;
				int vyf = weatherGrid[i].velocityY ;
				int takeX = x + vxf  ;// - (dx * advDistanceMult));
				int takeY = y + vyf  ;// - (dy * advDistanceMult));

				unsigned int takeI = ((takeY * sizeX) + takeX ) % totalSize;

				dx += weatherGrid[takeI].velocityX;
				dy += weatherGrid[takeI].velocityY;



				weatherGrid[i].velocityX += (dx - weatherGrid[i].velocityX) * 0.9f;
				weatherGrid[i].velocityY += (dy - weatherGrid[i].velocityY) * 0.9f;






				if (weatherGrid[i].pressure > maximumDisplayPressure)
				{
					maximumDisplayPressure = weatherGrid[i].pressure;
				}
				if (weatherGrid[i].velocityX > maximumDisplayVelocity)
				{
					maximumDisplayVelocity = weatherGrid[i].velocityX;
				}
				if (weatherGrid[i].velocityY > maximumDisplayVelocity)
				{
					maximumDisplayVelocity = weatherGrid[i].velocityY;
				}


			}
		}
	}

#ifdef THREAD_TIMING_READOUT
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_weather " << elapsed.count() << " microseconds." << std::endl;
#endif

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

std::vector<Animal> animals;

Animal::Animal()
{
	this->age = 0;
	this->energyFlags = ENERGYSOURCE_PLANT;
	this->direction = 4;
	this->segmentsUsed = 0;
	// this->muscleMass = 0;
	this->biggestMuscle = 0;
	this->biggestEye = 0;
	this->attack = 0;
	this->maxStoredEnergy = 0.0f;
	this->reproductionEnergy = 0.0f;
	this->hitPoints = 1;
	this->maxHitPoints = this->hitPoints;
	this->steady = false;
	this->timesReproduced = 0;
	this->retired = false;
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


// prints the animal to the terminal!
void catScan(unsigned int animalIndex)
{
	if (animalIndex > animals.size()) {return;}
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
	if (animalIndex >= animals.size()  ) {return;}
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
		        i < sizeAnimalSprite  										||  // if i is on the bottom edge of the sprite, or
		        i > ((sizeAnimalSprite - 1)*sizeAnimalSprite)              	||  // if i is on the top edge of the sprite, or
		        i % sizeAnimalSprite == 0                               	||  // if i is on one side of the sprite, or
		        i % sizeAnimalSprite == (sizeAnimalSprite - 1)            	  // if i is on the other edge of the sprite
		    ) &&
		    (
		        a->segments[segmentNumber].frames[ frameOffset + i] == ORGAN_NOTHING					// and the pixel is not already drawn.
		    )
		)
		{
			a->segments[segmentNumber].frames[frameOffset + i] = ORGAN_MARKER_A;						// set the alpha to -1.0f, which does not occur in our drawings normally, and so can be used to mark a pixel with connection to the edge.
		}
	}

	for (unsigned int k = 0; k < (sizeAnimalSprite / 2); ++k)                                           // multiple passes help to ensure there are no daggy bits left
	{
		for (unsigned int i = 0; i < (squareSizeAnimalSprite); ++i)
		{
			if ( a->segments[segmentNumber].frames[ frameOffset + i ] != ORGAN_NOTHING) {continue;}
			unsigned int spriteNeighbours[] =												            // calculate the addresses of the four cardinal neighbours.
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
	if (animalIndex >= animals.size())				{return -1;}

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
		// numberModifier = numberModifier / 26;

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
	if (animalIndex >= animals.size())				{return;}

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
					// animals[animalIndex].defense++;
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
					// animals[animalIndex].hitPoints++;
				}
				else if (organ == ORGAN_EYE )
				{

					animals[animalIndex].totalEye ++;
					// animals[animalIndex].perception++;
					eyeStreak ++;
					if (eyeStreak > animals[animalIndex].biggestEye)
					{
						animals[animalIndex].biggestEye = eyeStreak;
					}
				}
				else if (organ == ORGAN_HEART )
				{

					animals[animalIndex].totalHeart ++;
					// animals[animalIndex].fitness++;
				}
				else if (organ == ORGAN_WEAPON )
				{

					animals[animalIndex].totalWeapon ++;
					// animals[animalIndex].attack++;
				}
				else if (organ == ORGAN_MOUTH )
				{

					animals[animalIndex].totalMouth ++;
					// animals[animalIndex].attack++;


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

	// if (animalIndex < animals.size() )
	// {
	// 	if (segmentIndex < animals[animalIndex].segmentsUsed)
	// 	{
	// 		if (pixelIndex < squareSizeAnimalSprite)
	// 		{
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
		memcpy( &animationGrid[ j__color_offset], 	&color_clear , 	sizeof(Color) );
	}
	// 		}
	// 	}

	// }

}


void setAnimalSpritePixel ( unsigned int animalIndex, unsigned int segmentIndex, unsigned int pixelIndex )
{
	// if (animalIndex < animals.size() )
	// {
	// 	if (segmentIndex < animals[animalIndex].segmentsUsed)
	// 	{
	// 		if (pixelIndex < squareSizeAnimalSprite)
	// 		{
	// find the x, y offset of the pixel from the origin in sprite coordinates.
	int pixelX = (pixelIndex % sizeAnimalSprite) - halfSizeAnimalSprite;
	int pixelY = (pixelIndex / sizeAnimalSprite) - halfSizeAnimalSprite;
	int segmentX = animals[animalIndex].segments[segmentIndex].position % sizeX;
	int segmentY = animals[animalIndex].segments[segmentIndex].position / sizeX;
	int worldX = segmentX + pixelX;
	int worldY = segmentY + pixelY;
	int worldI = ((worldY * sizeX) + worldX ) % totalSize;
	int j__color_offset = (worldI * numberOfFieldsPerVertex) ;


	unsigned int pixelOrgan =  animals[animalIndex].segments[segmentIndex].frames[
	                               (squareSizeAnimalSprite * animals[animalIndex].segments[segmentIndex].animationFrame) + pixelIndex
	                           ];


	if ( pixelOrgan != ORGAN_NOTHING)
	{



		Color organColor = organColorLookup(  pixelOrgan     );

		// if (segmentIndex == 0) { organColor = color_yellow; }
		// if (segmentIndex == 1) { organColor = color_darkred; }
		// if (segmentIndex == 2) { organColor = color_lightblue; }
		// if (segmentIndex == 3) { organColor = color_purple; }


		memcpy( &animationGrid[ j__color_offset], &(	organColor )  , 	sizeof(Color) );




		// also, leave a pheromone trail for others to follow!
		if (seedGrid[worldI].stage == STAGE_NULL)
		{
			seedGrid[worldI].parentIdentity = animalIndex;
		}

	}
	// 		}
	// 	}

	// }
}

void clearAnimalDrawing(unsigned int i)
{
	if (seedGrid[i].parentIdentity < animals.size())
	{
		unsigned int animalIndex = seedGrid[i].parentIdentity;

		for (	unsigned int j = 0; j < animals[animalIndex].segmentsUsed; j++ )
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
	// reset everything to the beginning state
	animalCursorFrame = 0;
	animalCursorString = 0;
	animalCursorSegmentRadius = 2;
	animalCursorSegmentAngle = 0.0f;
	animalCursorX = (sizeAnimalSprite / 2); // return the drawing cursor to the center
	animalCursorY = (sizeAnimalSprite / 2);
	animalCursorColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
	animalCursorSegmentNumber = 0;
	animalCursorOrgan = ORGAN_MUSCLE;

	// Animal newAnimal = Animal();
	// animals.push_back(newAnimal);

	unsigned int animalIndex = 0; //= animals.size() - 1;

	bool foundAnID = false;

	for (unsigned int animal = 0; animal < animals.size(); ++animal)
	{
		if (animals[animal].retired)
		{
			foundAnID = true;
			animalIndex = animal;
			animals[animalIndex] = Animal();

#ifdef ANIMAL_DRAWING_READOUT
			printf("Recycle animal ID %u\n", animalIndex);
#endif
			break;
		}
	}

	if (!foundAnID) // if there was no slot available, make a new one
	{
		Animal newAnimal = Animal();


		std::vector<Animal>::iterator it;
		it = animals.end();
		animals.insert(it, newAnimal);
		animalIndex = animals.size() - 1;

		// animals.push_back(newAnimal);

		// instead of push_back, use an iterator to insert
		// https://stackoverflow.com/questions/60393716/segmentation-fault-when-using-vector-push-back-inside-of-certain-loops
		// it still bugs out with insert if you are adding huge amounts of animals quickly



#ifdef ANIMAL_DRAWING_READOUT
		printf("New animal with ID %u \n", animalIndex);
#endif
	}

	seedGrid[i].parentIdentity = animalIndex;



	if (animalIndex < animals.size() && seedGrid[i].stage == STAGE_ANIMAL)
	{
		seedGrid[i].drawingInProgress = true;

		// to improve code stability, this operation passes the animal and genome by literal value instead of by reference
		// it is operated on in a factory-like way and then the original is overwritten with the modified copy
		// Animal  a = Animal();
		animals[animalIndex] = Animal();
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

void setPhoton(  unsigned int i)
{
	seedGrid[i].stage = STAGE_PHOTON;
	seedGrid[i].energy = sunlightEnergy;
	memcpy( (&seedColorGrid[i * numberOfFieldsPerVertex]) ,  &(color_white_clear),  sizeof(Color) );
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

// travel from the indicated square in the light direction, marking cells as illuminated or dark along your way.
void photate( unsigned int i )
{
	unsigned int currentPosition = i;
	unsigned int blocked = 0;
	while (true)
	{
		unsigned int x = currentPosition % sizeX;
		unsigned int y = currentPosition / sizeX;
		currentPosition = neighbourOffsets[sunlightDirection] + currentPosition;
		if (currentPosition > totalSize) {break;}
		unsigned int b_offset = (currentPosition * numberOfFieldsPerVertex) ;
		if (
		    grid[currentPosition].phase == PHASE_SOLID ||
		    grid[currentPosition].phase == PHASE_POWDER ||
		    lifeGrid[currentPosition].identity > 0x00 ||
		    seedGrid[currentPosition].stage != STAGE_NULL
		)
		{
			blocked ++;
		}

		if (!blocked)
		{
			seedGrid[currentPosition].energy = lampBrightness;
			memcpy( &seedColorGrid[ b_offset], 	&color_clear , 	sizeof(Color) );
		}
		else
		{
			if (seedGrid[currentPosition].stage == STAGE_NULL)
			{
				seedGrid[currentPosition].energy = lampBrightness  / blocked;
				memcpy( &seedColorGrid[ b_offset], 	&color_shadow , 	sizeof(Color) );
				unsigned int a_offset = (b_offset) + 3;
				seedColorGrid[a_offset] = 1 / (seedGrid[currentPosition].energy ) ;

				// 0 at 700, 100% at 1000
				if (grid[currentPosition].temperature > 600 )
				{
					seedColorGrid[a_offset] = seedColorGrid[a_offset] / 2;
					if (grid[currentPosition].temperature > 1000)
					{
						seedColorGrid[a_offset] = 0;
					}
				}
				if (seedColorGrid[a_offset] > 0.5f) {seedColorGrid[a_offset] = 0.5f;}
				if (grid[currentPosition].phase == PHASE_VACUUM && lifeGrid[currentPosition].identity == 0x00) {seedColorGrid[a_offset] = 0.0f;}
			}
		}
		if (x == 0 || y == 0 || x >= sizeX || y >= sizeY) {break;}
	}
}


void setAnimal(unsigned int i, std::string genes)
{
	seedGrid[i].genes = genes;
	seedGrid[i].stage = STAGE_ANIMAL;
	memcpy( (&seedColorGrid[i * numberOfFieldsPerVertex]) ,  &(color_clear),  sizeof(Color) );
	drawAnimalFromSeed(i);
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

	// weatherGrid[i].temperature = defaultTemperature;
	// weatherGrid[i].pressure = 1000;
}


void killAnAnimal(unsigned int i)
{

	// delete the animal from the relevant grids, leaving behind a huge mess

	if (seedGrid[i].parentIdentity >= animals.size())
	{
		return;
	}

	unsigned int animalIndex = seedGrid[i].parentIdentity;

	for (	 int j = animals[animalIndex].segmentsUsed - 1; j >= 0 ; j-- )
	{
		for ( int k = 0; k < squareSizeAnimalSprite; ++k)
		{
			// clearAnimalSpritePixel( animalIndex, j, k);

			int pixelIndex = (squareSizeAnimalSprite * FRAME_BODY) + k;

			if (j > animals[animalIndex].segmentsUsed) {continue;}
			if (pixelIndex > squareSizeAnimalSprite) {continue;}
			if ( animals[animalIndex].segments[j].frames[pixelIndex] != ORGAN_NOTHING)
			{



				// int worldAnimalX = i % sizeX;
				// int worldAnimalY = i / sizeX;
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

	animals[animalIndex].retired = true;
	clearAnimalDrawing(i);
	clearSeedParticle(i);


}


void incrementAnimalSegmentPositions (unsigned int animalIndex, unsigned int i, bool falling)
{
	if (animalIndex < animals.size())
	{

		if (animals[animalIndex].segmentsUsed > 0 )
		{





			// Update animal segment positions. Only do this if the animal has actually moved (otherwise it will pile into one square).
			if (animals[animalIndex].segments[0].position != i)
			{
				for (  int j = (animals[animalIndex].segmentsUsed - 1); j >= 0; --j) // when using lots of minuses and value comparisons like this its a good idea to use int. this would hang forever if j was unsigned
				{
					for (unsigned int k = 0; k < (squareSizeAnimalSprite); ++k)
					{
						clearAnimalSpritePixel(animalIndex, j, k);
					}
				}

				// set the position of the 0th segment to the new index, and everyone elses position is shifted forward by 1.
				// bool segmentPhase = false;
				// if (a->segments[0].animationFrame == FRAME_BODY) {segmentPhase = true;}
				// for (unsigned int j = 0; j < animals[animalIndex].segmentsUsed; ++j)
				// {
				// 	if (falling)
				// 	{
				// 		animals[animalIndex].segments[j].animationFrame = FRAME_C;
				// 	}
				// 	else
				// 	{
				// 		if (animals[animalIndex].segments[j].animationFrame == FRAME_B) { animals[animalIndex].segments[j].animationFrame = FRAME_BODY;}
				// 		else {animals[animalIndex].segments[j].animationFrame = FRAME_B;}
				// 	}
				// }


				for (  int j = (animals[animalIndex].segmentsUsed - 1); j > 0; --j)
				{
					animals[animalIndex].segments[j].position = animals[animalIndex].segments[j - 1].position;


				}
				animals[animalIndex].segments[0].position = i;

				for (  int j = (animals[animalIndex].segmentsUsed - 1); j >= 0; --j)
				{
					for (unsigned int k = 0; k < (squareSizeAnimalSprite); ++k)
					{
						// if(j == 0)
						// {

						setAnimalSpritePixel( animalIndex, j, k);
						// }
					}
				}
			}
		}
	}
}



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

void resetMaterials()
{
	materials.clear();

	// materials[0] is supposed to be vacuum.
	Material vacuum = Material();
	vacuum.color = color_clear;
	vacuum.melting = 0;
	vacuum.boiling = 0;
	vacuum.insulativity = 1000;
	materials.push_back(vacuum);

// materials[1] is supposed to be soil.
	Material bumdirt = Material();
	bumdirt.color = color_lightgrey;
	bumdirt.melting = 1000;
	bumdirt.boiling = 2200;
	bumdirt.insulativity = 10;
	materials.push_back(bumdirt);

// materials[2] is supposed to be blood.
	Material blood = Material();
	blood.color = color_darkred;
	blood.melting = 0;
	blood.boiling = 100;
	blood.insulativity = 33;
	materials.push_back(blood);

// materials[3] is supposed to be bones.
	Material bone = Material();
	bone.color = color_offwhite;
	bone.melting = 1000;
	bone.boiling = 2200;
	bone.insulativity = 400;
	materials.push_back(bone);

	unsigned int nNewMaterials = 5;
	for (unsigned int k = 0; k < nNewMaterials; ++k)
	{
		Material newMaterial = Material();
		newMaterial.boiling = RNG() * 3000;
		newMaterial.melting = RNG() * newMaterial.boiling;
		newMaterial.crystal_n = extremelyFastNumberFromZeroTo(4);

		unsigned int randomCondition =  extremelyFastNumberFromZeroTo(4);
		if (randomCondition == 0) {newMaterial.crystal_condition = CONDITION_GREATERTHAN; }
		else if (randomCondition == 1) {newMaterial.crystal_condition = CONDITION_EQUAL; }
		// else if (randomCondition == 2) {newMaterial.crystal_condition = CONDITION_LESSTHAN; }
		// else if (randomCondition == 3) {newMaterial.crystal_condition = CONDITION_EVENNUMBER; }
		// else if (randomCondition == 4) {newMaterial.crystal_condition = CONDITION_ODDNUMBER; }
		else if (randomCondition == 2) {newMaterial.crystal_condition = CONDITION_EDGE; }
		else if (randomCondition == 3) {newMaterial.crystal_condition = CONDITION_CORNER; }
		// else if (randomCondition == 7) {newMaterial.crystal_condition = CONDITION_ROW; }
		// else if (randomCondition == 8) {newMaterial.crystal_condition = CONDITION_LEFTN; }
		else if (randomCondition == 4) {newMaterial.crystal_condition = CONDITION_NOTLEFTRIGHTN; }
		// else if (randomCondition == 10) {newMaterial.crystal_condition = CONDITION_NOTLRNEIGHBOURS; }

		newMaterial . color = color_grey;
		newMaterial.color.r = RNG();
		newMaterial.color.g = RNG();
		newMaterial.color.b = RNG();

		materials.push_back(newMaterial);
	}

	// a change to the materials list is dramatic. The material of a grid square is used as an array index to look up a material which may not exist.
	// this function fills the entire grid with vacuum. it is better than segfault.
	for (int i = 0; i < totalSize; ++i)
	{
		setParticle(MATERIAL_VACUUM,  i);
		grid[i].phase = PHASE_VACUUM;
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
	unsigned int    a_offset      = (i * numberOfFieldsPerVertex) ;
	backgroundSky[ 	a_offset + 0] = 0.0f;
	backgroundSky[ 	a_offset + 1] = 0.0f;
	backgroundSky[ 	a_offset + 2] = 0.0f;
	backgroundSky[ 	a_offset + 3] = 0.0f;
	backgroundSky[ 	a_offset + 4] = fx;
	backgroundSky[ 	a_offset + 5] = fy;
	colorGrid[ 		a_offset + 0] = 0.0f;
	colorGrid[ 		a_offset + 1] = 0.0f;
	colorGrid[ 		a_offset + 2] = 0.0f;
	colorGrid[ 		a_offset + 3] = 0.0f;
	colorGrid[ 		a_offset + 4] = fx;
	colorGrid[ 		a_offset + 5] = fy;
	lifeColorGrid[ 	a_offset + 0] = 0.0f;
	lifeColorGrid[ 	a_offset + 1] = 0.0f;
	lifeColorGrid[ 	a_offset + 2] = 0.0f;
	lifeColorGrid[ 	a_offset + 3] = 0.0f;
	lifeColorGrid[ 	a_offset + 4] = fx;
	lifeColorGrid[ 	a_offset + 5] = fy;
	animationGrid[  a_offset + 0] = 0.0f;
	animationGrid[  a_offset + 1] = 0.0f;
	animationGrid[  a_offset + 2] = 0.0f;
	animationGrid[  a_offset + 3] = 0.0f;
	animationGrid[  a_offset + 4] = fx;
	animationGrid[  a_offset + 5] = fy;
	seedColorGrid[ 	a_offset + 0] = 0.0f;
	seedColorGrid[ 	a_offset + 1] = 0.0f;
	seedColorGrid[ 	a_offset + 2] = 0.0f;
	seedColorGrid[ 	a_offset + 3] = 0.5f;
	seedColorGrid[ 	a_offset + 4] = fx;
	seedColorGrid[ 	a_offset + 5] = fy;
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
		weatherGrid[i] = Weather();

		clearParticle(i);
		clearSeedParticle(i);
		clearLifeParticle(i);

		// RGBA color occupies the first 4 places.
		// also, initialize the color alpha to 1.
		clearColorGrids(i);
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

Color blackbodyLookup( unsigned int temperature )
{
	if (temperature > 0 && temperature < 600 )
	{
		return color_clear;;
	}
	else if (temperature < 772) // faint red
	{
		return Color(0.16f, 0.0f, 0.0f, 0.03f);
	}
	else if (temperature < 852) // blood red
	{
		return  Color(0.33f, 0.0f, 0.0f, 0.11f);
	}
	else if (temperature < 908) // dark cherry
	{
		return Color(0.5f, 0.0f, 0.0f, 0.20f);
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

void createRandomWorld()
{
	clearGrids();
	resetMaterials();
	sunlightColor = blackbodyLookup(sunlightTemp);
	defaultTemperature = RNG() * 1500;


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

	// compute the background sky
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		unsigned int a_offset  = i * numberOfFieldsPerVertex;
		memcpy( &(backgroundSky[ a_offset ]), &color_black, 					sizeof(Color) );
		if (extremelyFastNumberFromZeroTo(100) ==  0)
		{
			// create a background star with random blackbody color and alpha
			unsigned int randomColorTemperature = extremelyFastNumberFromZeroTo(5000);
			Color randomStarColor = blackbodyLookup(randomColorTemperature);
			float randomStarAlpha = RNG();
			randomStarAlpha = randomStarAlpha * randomStarAlpha * randomStarAlpha * randomStarAlpha * randomStarAlpha * randomStarAlpha * randomStarAlpha; // cubing the value or more shifts the distribution lower while preserving the range.
			randomStarAlpha = randomStarAlpha / 2;
			randomStarColor.a = randomStarAlpha;
			memcpy( &(backgroundSky[ a_offset ]), &randomStarColor, 					sizeof(Color) );
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
	angle += ( 3.1415f);
	angle = angle / (2 * 3.1415f);
	angle *= 8.0f;
	result = angle;
	if (result == 8) {result = 0;}
	return result;
}

// should the animal move into that position?
bool animalCanMove(unsigned int i, unsigned int neighbour)
{
	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex < animals.size() && neighbour < totalSize)
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
	if (animalIndex < animals.size() && direction < N_NEIGHBOURS )
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


// unsigned int getRandomWalkableNeighbour(unsigned int i, unsigned int animalIndex, unsigned int direction, unsigned int startPosition)
// {

// 	unsigned int randomN = extremelyFastNumberFromZeroTo(N_NEIGHBOURS);
// 	for (int j = 0; j < N_NEIGHBOURS; ++j)
// 	{

// 	}

// }

bool animalEat(unsigned int currentPosition , unsigned int neighbour )
{

	unsigned int animalIndex  = seedGrid[currentPosition].parentIdentity;

	if (animalIndex > animals.size()) {return false;}
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
		// if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_ANIMAL ) == ENERGYSOURCE_ANIMAL || carnageMode  ) // always attack other animals in carnage mode.. you don't have to eat them if you dont want to.
		// {





		if (
		    (animals[animalIndex].energyFlags & ENERGYSOURCE_ANIMAL ) == ENERGYSOURCE_ANIMAL         // if it eats other animals,
		)
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
		if (

		    (animals[animalIndex].personalityFlags & PERSONALITY_AGGRESSIVE ) == PERSONALITY_AGGRESSIVE  ||    // if the animal is aggressive, or
		    (animals[animalIndex].energyFlags & ENERGYSOURCE_ANIMAL ) == ENERGYSOURCE_ANIMAL ||                // if it eats other animals, or
		    carnageMode
		)
		{
			// attack the other animal and either try to kill it or take conquest of it.
			if (seedGrid[neighbour].parentIdentity < animals.size())
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
				// seedGrid[currentPosition].energy  += damageInflictedB;
				animals[animalIndexB].hitPoints -= damageInflictedB;
				eaten = true;
				if (animals[animalIndexB].hitPoints < 0) // the adversary is vanquished mortally
				{
					// if you are a carnivore, kill and eat the opponent
					// it will explode into blood particles that you can then consume;
#ifdef ANIMAL_BEHAVIOR_READOUT
					printf("animal %u killed animal %u\n", animalIndex, animalIndexB);
#endif
					killAnAnimal(neighbour);
				}
				else if (animals[animalIndexB].hitPoints < (animals[animalIndexB].maxHitPoints / 3))
				{


					// force the opponent to bear your offspring
#ifdef ANIMAL_BEHAVIOR_READOUT
					printf("animal %u conquered %u and impregnated it!\n", animalIndex, animalIndexB );
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
			// clearSeedParticle(neighbour);
			// break;
		}
	}



	return eaten;

}


void animalFeed(unsigned int i)
{

	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex < animals.size())
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

				// if (lookForMoveSquares)
				// {
				// if ((animals[animalIndex].segments[segmentIndex].frames[pixelIndex] == ORGAN_MUSCLE   ) )
				// {

				// 	unsigned int worldX = segmentX + spriteX - halfSizeAnimalSprite;
				// 	unsigned int worldY = segmentY + spriteY - halfSizeAnimalSprite;
				// 	unsigned int worldI = ((worldY * sizeX) + worldX ) % totalSize;
				// 	if (worldI != i)
				// 	{
				// 		if ( animalCanMove(i, worldI) )
				// 		{
				// 			int diffX = spriteDMostPointX - spriteX;                          // you found one! compare the different to the best so far.
				// 			int diffY = spriteDMostPointY - spriteY;                         // but don't tell anyone I do math like this.
				// 			int diffSum = abs(diffX) + abs(diffY);
				// 			if (diffSum < closestSum  )
				// 			{
				// 				closestWalkableMuscle = worldI;
				// 				closestSum = diffSum;
				// 			}
				// 			// foundValidCloseSquareThisSegment = true;
				// 		}
				// 	}
				// }
				// }

				// // feed while you're here
				// else
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

			// if (foundValidCloseSquareThisSegment) { lookForMoveSquares = false;}
		}
	}


}




// // for a given animal and desired direction, find the square furthest most in that direction, within its muscle area, that an animal may move
// // at the moment this is exhaustively iterative, may as well use it for other iterative tasks such as feeding
// unsigned int getDMostWalkableMuscleSquare( unsigned int i, unsigned int animalIndex, unsigned int direction)
// {

// 	// printf("getDMostWalkableMuscleSquare direction %u\n", direction);
// 	// for the given direction, find the point in the sprite that is closest to it.
// 	unsigned int spriteDMostPointX = 0;
// 	unsigned int spriteDMostPointY = 0;

// 	if (direction == 0)
// 	{
// 		spriteDMostPointX = 0;
// 		spriteDMostPointY = halfSizeAnimalSprite;
// 	}

// 	if (direction == 1)
// 	{
// 		spriteDMostPointX = 0;
// 		spriteDMostPointY = 0;
// 	}

// 	if (direction == 2)
// 	{
// 		spriteDMostPointX = halfSizeAnimalSprite;
// 		spriteDMostPointY = 0;
// 	}

// 	if (direction == 3)
// 	{
// 		spriteDMostPointX = sizeAnimalSprite;
// 		spriteDMostPointY = 0;
// 	}

// 	if (direction == 4)
// 	{
// 		spriteDMostPointX = sizeAnimalSprite;
// 		spriteDMostPointY = halfSizeAnimalSprite;
// 	}

// 	if (direction == 5)
// 	{
// 		spriteDMostPointX = sizeAnimalSprite;
// 		spriteDMostPointY = sizeAnimalSprite;
// 	}

// 	if (direction == 6)
// 	{
// 		spriteDMostPointX = halfSizeAnimalSprite;
// 		spriteDMostPointY = sizeAnimalSprite;
// 	}

// 	if (direction == 7)
// 	{
// 		spriteDMostPointX = 0;
// 		spriteDMostPointY = sizeAnimalSprite;
// 	}


// 	unsigned int closestWalkableMuscle = i;
// 	unsigned int closestSum = sizeAnimalSprite + sizeAnimalSprite;

// 	// bool foundValidCloseSquareThisSegment = false;
// 	// bool lookForMoveSquares = true;

// 	for (unsigned int segmentIndex = 0; segmentIndex < animals[animalIndex].segmentsUsed; ++segmentIndex)
// 	{

// 		unsigned int segmentX = animals[animalIndex].segments[segmentIndex].position % sizeX;
// 		unsigned int segmentY = animals[animalIndex].segments[segmentIndex].position / sizeX;
// 		for (unsigned int spriteI = 0; spriteI < squareSizeAnimalSprite; ++spriteI)
// 		{

// 			unsigned int spriteX = spriteI % sizeAnimalSprite;
// 			unsigned int spriteY = spriteI / sizeAnimalSprite;
// 			unsigned int pixelIndex = (squareSizeAnimalSprite * FRAME_BODY) + spriteI;

// 			// if (lookForMoveSquares)
// 			// {
// 			if ((animals[animalIndex].segments[segmentIndex].frames[pixelIndex] == ORGAN_MUSCLE   ) )
// 			{

// 				unsigned int worldX = segmentX + spriteX - halfSizeAnimalSprite;
// 				unsigned int worldY = segmentY + spriteY - halfSizeAnimalSprite;
// 				unsigned int worldI = ((worldY * sizeX) + worldX ) % totalSize;
// 				if (worldI != i)
// 				{
// 					if ( animalCanMove(i, worldI) )
// 					{
// 						int diffX = spriteDMostPointX - spriteX;                          // you found one! compare the different to the best so far.
// 						int diffY = spriteDMostPointY - spriteY;                         // but don't tell anyone I do math like this.
// 						int diffSum = abs(diffX) + abs(diffY);
// 						if (diffSum < closestSum  )
// 						{
// 							closestWalkableMuscle = worldI;
// 							closestSum = diffSum;
// 						}
// 						// foundValidCloseSquareThisSegment = true;
// 					}
// 				}
// 			}
// 			// }

// 			// // feed while you're here
// 			// else if ((animals[animalIndex].segments[segmentIndex].frames[pixelIndex] == ORGAN_MOUTH   ) )
// 			// {
// 			// 	unsigned int worldX = segmentX + spriteX - halfSizeAnimalSprite;
// 			// 	unsigned int worldY = segmentY + spriteY - halfSizeAnimalSprite;
// 			// 	unsigned int worldI = ((worldY * sizeX) + worldX ) % totalSize;
// 			// 	if (worldI != i)
// 			// 	{
// 			// 		animalEat(i, worldI);
// 			// 	}
// 			// }
// 		}

// 		// if (foundValidCloseSquareThisSegment) { lookForMoveSquares = false;}
// 	}

// 	// printf("the D most square is. %u i is %u\n", closestWalkableMuscle, i);
// 	return closestWalkableMuscle;
// }

void initialize ()
{
	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	srand((unsigned int)time(NULL));
	cursor_seedColor = color_yellow;
	clearGrids();
	resetMaterials();

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

void setNeutralTemp ()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		grid[i].temperature = 300;
	}
}

void setExtremeTempPoint (unsigned int x , unsigned  int y)
{
	unsigned int i = ((y * sizeX) + x) % totalSize;
	weatherGrid[i].temperature = 10000000.0f;
	weatherGrid[i].pressure += 1000.0f;

	// weatherGrid[i].velocityY = 10000000.0f;
}

// updates a location on the color grid with a material's new color information, based on phase and temperature.
void materialPostProcess(unsigned int i)
{
	unsigned int x = i % sizeX;
	unsigned int y = i / sizeX;
	Color ppColor =  color_clear;
	if (grid[i].phase != PHASE_VACUUM)
	{
		if (grid[i].material  < materials.size())
		{
			ppColor =  materials[ grid[i].material ].color ;
		}
		if (grid[i].phase == PHASE_GAS)
		{
			ppColor = addColor(ppColor, phaseTingeGas);
		}
		else if (grid[i].phase == PHASE_LIQUID)
		{
			ppColor = addColor(ppColor, phaseTingeLiquid);
		}
		else if (grid[i].phase == PHASE_SOLID)
		{
			ppColor = addColor(ppColor, phaseTingeSolid);
		}
		ppColor = addColor(ppColor, blackbodyLookup( grid[i].temperature ) );
	}
	colorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = ppColor.r;
	colorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = ppColor.g;
	colorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = ppColor.b;
	colorGrid[ (i * numberOfFieldsPerVertex) + 3 ] = ppColor.a;
}

void thread_temperature2_sector ( unsigned int from, unsigned int to )
{
	for (unsigned int i = from; i < to; ++i)
	{
		unsigned int currentPosition = i;
		if (grid[currentPosition].phase != PHASE_VACUUM)
		{
			// exchange heat with a neighbour.
			unsigned int thermoNeighbour = neighbourOffsets[extremelyFastNumberFromZeroTo(7)] + currentPosition ;
			if (grid[thermoNeighbour].phase != PHASE_VACUUM)
			{
				int avgTemp = (((grid[currentPosition].temperature ) - (grid[thermoNeighbour].temperature)) ) ;
				avgTemp = avgTemp / materials[ grid[currentPosition].material ].insulativity;
				grid[thermoNeighbour].temperature += avgTemp;
				grid[currentPosition].temperature -= avgTemp;
			}
			else
			{
				if (doWeather)
				{

					// float fgridtemp = grid[currentPosition].temperature;
					//  float favgTemp = (fgridtemp  + weatherGrid[thermoNeighbour].temperature)/2 ;

					// weatherGrid[thermoNeighbour].temperature = avgTemp;
					// grid[currentPosition].temperature = avgTemp;
				}
				else
				{
					// if neighbour is a vacuum, radiate heat away into space. more so if it is hotter.
					int crntmp = grid[currentPosition].temperature;
					int dftmp = defaultTemperature;
					int radiantHeat = (crntmp - dftmp) / radiantHeatIntensity;
					grid[currentPosition].temperature -= radiantHeat;
					float fradiantHeat = radiantHeat;
					weatherGrid[currentPosition].temperature += fradiantHeat;
				}
			}
			if (extremelyFastNumberFromZeroTo(4) == 0) // only check phase sometimes bcoz its lots of work.
			{
				// Phase change logic, which also includes crystallization.
				if (grid[currentPosition].phase == PHASE_SOLID)
				{
					if  (grid[currentPosition].temperature > materials[grid[currentPosition].material].boiling)
					{
						grid[currentPosition].phase = PHASE_GAS;
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
					else if  (grid[currentPosition].temperature < materials[grid[currentPosition].material].melting)
					{
						grid[currentPosition].phase = PHASE_POWDER;
					}
					else
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
							if (grid[neighbourOffsets[j] + currentPosition].phase == PHASE_SOLID )
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
								if (grid[neighbourOffsets[j] + currentPosition].phase == PHASE_LIQUID ||
								        grid[neighbourOffsets[j] + currentPosition].phase == PHASE_POWDER )
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
								if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
							}
						}
						else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_EQUAL)
						{
							if (nSolidNeighbours == materials[grid[currentPosition].material].crystal_n)
							{
								if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
							}
						}

						// These conditions produce boring results, so i do not care to include them, as even checking for them slows down the program.
						// But their information should be known in this code.
						// else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_LESSTHAN)
						// {
						// 	if (nSolidNeighbours < materials[grid[currentPosition].material].crystal_n)
						// 	{
						// 		if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
						// 	}
						// }
						// else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_EVENNUMBER)
						// {
						// 	if (nSolidNeighbours % 2 == 0)
						// 	{
						// 		if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
						// 	}
						// }
						// else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_ODDNUMBER)
						// {
						// 	if (nSolidNeighbours % 2 == 1)
						// 	{
						// 		if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
						// 	}
						// }

						else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_CORNER)
						{
							if (longestSolidStreak == 3 && (longestSolidStreakOffset % 2) == 0 )
							{
								if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
							}
						}
						else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_EDGE)
						{
							if (longestSolidStreak == 3 && (longestSolidStreakOffset % 2) == 1 )
							{
								if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
							}
						}
						else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_ROW)
						{
							if (longestSolidStreak == materials[grid[currentPosition].material].crystal_n )
							{
								if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
							}
						}
						else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_LEFTN)
						{
							if (	grid[currentPosition + (materials[grid[currentPosition].material].crystal_n) ].phase == PHASE_SOLID)
							{
								if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
							}
						}
						else if (materials[grid[currentPosition].material].crystal_condition == CONDITION_NOTLEFTRIGHTN)
						{
							if (
							    grid[currentPosition + (materials[grid[currentPosition].material].crystal_n) ].phase != PHASE_SOLID &&
							    grid[currentPosition - (materials[grid[currentPosition].material].crystal_n) ].phase != PHASE_SOLID
							)
							{
								if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
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
									if (nAttachableNeighbours > 0) {	grid[currentPosition].phase = PHASE_SOLID; }
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
				else if (grid[currentPosition].phase == PHASE_GAS)
				{
					if (grid[currentPosition].temperature < materials[grid[currentPosition].material].boiling)
					{
						grid[currentPosition].phase = PHASE_LIQUID;
					}
				}
			}

			// movement instructions for POWDERS
			if (grid[currentPosition].phase  == PHASE_POWDER)
			{
				unsigned int neighbour = neighbourOffsets[ (  1 +  extremelyFastNumberFromZeroTo(2) )  ] + currentPosition;
				if ((    grid[neighbour].phase == PHASE_VACUUM) ||
				        (grid[neighbour].phase == PHASE_GAS) ||
				        (grid[neighbour].phase == PHASE_LIQUID)  )
				{
					swapParticle(currentPosition, neighbour);
					currentPosition = neighbour;
				}
			}

			// movement instructions for LIQUIDS
			else if (grid[currentPosition].phase == PHASE_LIQUID)
			{
				unsigned int neighbour = neighbourOffsets[ (  0 +  extremelyFastNumberFromZeroTo(4) )  ] + currentPosition;
				if ((    grid[neighbour].phase == PHASE_VACUUM) ||
				        (grid[neighbour].phase == PHASE_GAS) ||
				        (grid[neighbour].phase == PHASE_LIQUID)     )
				{
					swapParticle(currentPosition, neighbour);
					currentPosition = neighbour;
				}
			}

			// movement instructions for GASES
			else if (grid[currentPosition].phase == PHASE_GAS)
			{
				unsigned int neighbour = neighbourOffsets[ extremelyFastNumberFromZeroTo(7) ] + currentPosition;

				// alternate between wind movement and random scatter movement, to look more natural.
				if (extremelyFastNumberFromZeroTo(1) == 0)
				{
					neighbour = neighbourOffsets[ weatherGrid[currentPosition].direction ] + currentPosition;
				}
				if (grid[neighbour].phase  == PHASE_VACUUM || (grid[neighbour].phase == PHASE_GAS) )
				{
					swapParticle(currentPosition, neighbour);
				}
			}
		}
	}
}

void thread_temperature2 ()
{
#ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
#endif
	unsigned int n_threads = 1;

	std::list<boost::thread> threads;

	for (unsigned int i = 0; i <= n_threads; ++i)
	{
		float ffrom = (i  - 0.5  ) ;
		float fto   = (i + 0.5)    ;
		ffrom = ffrom / (n_threads );
		fto = fto / (n_threads );
		if (ffrom < 0) {ffrom = 0;}
		if (fto < 0) {fto = 0;}
		ffrom *= totalSize;
		fto *= totalSize;
		unsigned int from = ffrom;
		unsigned int to = fto;
		if (from < (sizeX + 1)) {from = (sizeX + 1);}
		if (to > totalSize - (sizeX + 1)) {to = totalSize - (sizeX + 1);}
		boost::thread t99{  thread_temperature2_sector, from  , to  } ;
		threads.push_back( std::move(t99) );
	}
	for (auto& t : threads)
	{
		t.join();
	}

#ifdef THREAD_TIMING_READOUT
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

void thread_physics ()
{
#ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
#endif
	if (extremelyFastNumberFromZeroTo(100) == 0)
	{
		// make sure randomness is really random
		seedExtremelyFastNumberGenerators();
	}

	// shine the sunlight
	for (int i = (sizeY - 2) * sizeX; i < (sizeY - 1)*sizeX; ++i)
	{
		if (extremelyFastNumberFromZeroTo(4) == 0)
		{
			photate(i);
		}
	}

	unsigned int processChunkSize = (totalSize / 10);
	for (unsigned int i = 0; i < processChunkSize; ++i)
	{
		unsigned int x = extremelyFastNumberFromZeroTo(sizeX - 1);
		unsigned int y = extremelyFastNumberFromZeroTo(sizeY - 1);
		materialPostProcess(  (y * sizeX) + x  );
	}
#ifdef THREAD_TIMING_READOUT
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_physics " << elapsed.count() << " microseconds." << std::endl;
#endif
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

	if (animalIndex > animals.size()) {return false;}
	if (currentPosition == neighbour) {return false;}


	// while (true) //this statement is only here so it can be expediently broken out of
	// {
	if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_SEED ) == ENERGYSOURCE_SEED   )
	{
		if (seedGrid[neighbour].stage == STAGE_BUD || seedGrid[neighbour].stage == STAGE_FRUIT ||  seedGrid[neighbour].stage == STAGE_SPROUT )
		{
			seedGrid[currentPosition].energy  += seedEfficiency ;
// #ifdef ANIMAL_BEHAVIOR_READOUT
// 				printf("animal %u ate a seed for %f. Has %f reproduces at %f.\n", animalIndex, seedEfficiency, seedGrid[currentPosition].energy , animals[animalIndex].reproductionEnergy );
// #endif
			// clearSeedParticle(neighbour);
			// eaten = true;
			// break;
			return true;
		}
	}
	if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_PLANT ) == ENERGYSOURCE_PLANT   )
	{
		if (lifeGrid[neighbour].identity > 0x00)
		{
// 				seedGrid[currentPosition].energy  += plantEfficiency;
// #ifdef ANIMAL_BEHAVIOR_READOUT
// 				printf("animal %u ate a plant for %f. Has %f reproduces at %f.\n", animalIndex, plantEfficiency, seedGrid[currentPosition].energy , animals[animalIndex].reproductionEnergy );
// #endif
// 				clearLifeParticle(neighbour);
// 				eaten = true;
// 				break;
			return true;
		}
	}
	if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_MINERAL ) == ENERGYSOURCE_MINERAL   )
	{
		if (grid[neighbour].phase != PHASE_VACUUM)
		{
			seedGrid[currentPosition].energy  += mineralEfficiency;
// #ifdef ANIMAL_BEHAVIOR_READOUT
// 				printf("animal %u ate a mineral for %f. Has %f reproduces at %f.\n" , animalIndex, mineralEfficiency, seedGrid[currentPosition].energy , animals[animalIndex].reproductionEnergy );
// #endif
// 				clearParticle( neighbour);
// 				eaten = true;
// 				break;
			return true;
		}
	}
	// if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_ANIMAL ) == ENERGYSOURCE_ANIMAL || carnageMode  ) // always attack other animals in carnage mode.. you don't have to eat them if you dont want to.
	// {



	if (  (animals[animalIndex].energyFlags & ENERGYSOURCE_ANIMAL ) == ENERGYSOURCE_ANIMAL || carnageMode  ) // always attack other animals in carnage mode.. you don't have to eat them if you dont want to.
	{
		// or, if there is meat laying around you can eat.
		if (grid[neighbour].material == MATERIAL_BLOOD)
		{

// #ifdef ANIMAL_BEHAVIOR_READOUT
// 				printf(" Animal %u scavenged some meat for %f.\n" , animalIndex , bloodEfficiency );
// #endif


// 				seedGrid[currentPosition].energy  += bloodEfficiency;
// 				clearParticle(neighbour);
// 				eaten = true;
			// break;
			return true;
		}

	}
	// break;
	// }

	return false;



}








// returns the world pixel that is the most ideal legal move
unsigned int animalDirectionFinding (unsigned int i)
{
	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex > animals.size()) { return i;}
	unsigned int decidedLocation;
	bool decided = false;
	bool runAway = false;
	bool hungry = false;
	if (seedGrid[i].energy > (animals[animalIndex].reproductionEnergy / 2))
	{
		hungry = true;
	}

	if (animalIndex < animals.size())
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
			if (worldRandomI == i) {continue;} // do not observe the self, it leads to madness

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
					if (!hungry) // if the animal is not hungry
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


			// printf("not decided\n");

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

		// if (animals[animalIndex].muscleMass > 0 )
		// {
		// return getDMostWalkableMuscleSquare( i, animalIndex, animals[animalIndex]. direction);
		// }
		// else
		// {

		// unsigned int nextMove = i;
		// for (unsigned int step = 0; step < animals[animalIndex]. biggestMuscle; ++step)
		// {
		// nextMove =

		// printf("DIRECTIONFINDING DIRECTION %u\n", animals[animalIndex].direction );


		// }

		unsigned int currentPosition = i;

		for (unsigned int move = 0; move < animals[animalIndex].mobility; ++move)
		{
			currentPosition = getDMostWalkableSquare( i, animalIndex, animals[animalIndex]. direction, currentPosition);
		}

		return currentPosition;
		// return getDMostWalkableSquare( i, animalIndex, animals[animalIndex]. direction, i);
		;


		// }




	}
	return 0;
}

void animalReproduce (unsigned int i)
{
	unsigned int animalIndex = seedGrid[i].parentIdentity;
	if (animalIndex > animals.size()) {return;}

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
	if (animalIndex > animals.size()) {return;}
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
	if (animalIndex > animals.size() ) {return;}


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
	if (animalIndex > animals.size()) {return;}

	animals[animalIndex].age++;

	bool moved = false;
	animals[animalIndex].steady = false;
	unsigned int recommendedMovePosition = i;


	// for (unsigned int pulse = 0; pulse < animals[animalIndex].fitness; ++pulse)
	// {

	// 	unsigned int randomOrgan = extremelyFastNumberFromZeroTo(animals[animalIndex].totalArea);

	// 	unsigned int organRegionA = 0;
	// 	unsigned int organRegionB =

	// 	    if ( randomOrgan > organRegion && randomOrgan < organRegion)


	// 	}



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

// 	// reproduce
// 	if (seedGrid[i].energy > animals[animalIndex].reproductionEnergy && animals[animalIndex].age > (animals[animalIndex].reproductionEnergy + 100)  && animalReproductionEnabled)
// 	{
// 		animalReproduce(i);
// 	}

// 	// die
// 	if (seedGrid[i].energy < 0.0f)
// 	{
// #ifdef ANIMAL_BEHAVIOR_READOUT
// 		printf("animal %u ran out of energy and died.\n ", animalIndex  );
// #endif
// 		killAnAnimal(i);
// 		return;
// 	}


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
		incrementAnimalSegmentPositions(animalIndex, recommendedMovePosition, false );
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
			if (grid[i].phase == PHASE_VACUUM)
			{
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = weatherGrid[i].temperature / maximumDisplayTemperature;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = weatherGrid[i].temperature / maximumDisplayTemperature;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = weatherGrid[i].temperature / maximumDisplayTemperature;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 3 ] = 1.0f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 4 ] = fx;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 5 ] = fy;
			}
			else
			{
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = grid[i].temperature / maximumDisplayTemperature;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = grid[i].temperature / maximumDisplayTemperature;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = grid[i].temperature / maximumDisplayTemperature;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 3 ] = 1.0f;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 4 ] = fx;
				energyColorGrid[ (i * numberOfFieldsPerVertex) + 5 ] = fy;
			}
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
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = (weatherGrid[i].velocityX / maximumDisplayVelocity ) + 0.5f ; //weatherGrid[i].pressure / maximumDisplayPressure;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = (weatherGrid[i].velocityY / maximumDisplayVelocity ) + 0.5f ; //weatherGrid[i].pressure / maximumDisplayPressure;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = (weatherGrid[i].pressure / maximumDisplayPressure  )  ;
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

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, backgroundSky, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, colorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, lifeColorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

		// for (unsigned int i = 0; i < totalSize; ++i)
		// {
		// 	if (seedGrid[i].stage == STAGE_ANIMAL)
		// 	{
		// 		clearAnimalDrawing(i);
		// 	}
		// }

		for (unsigned int i = 0; i < totalSize; ++i)
		{
			if (seedGrid[i].stage == STAGE_ANIMAL)
			{
				animalTurn(i);
			}
		}

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, animationGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, seedColorGrid, GL_DYNAMIC_DRAW );
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
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }
		numberModifier = numberModifier % materials.size();
		cursor_germinationMaterial = numberModifier;
		break;
	}

	case 'q': // set energy source.
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}
		int numberModifier = 0.0f;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }
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

	case 'a': // array. a motif is repeated a few times from the same place but with an increasing angle offset.
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

	case 'n': // rosette. a motif is repeated with perfect radial symmetry in n divisions.
	{
		cursor_string++; if (cursor_string > genesize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

		// rotationIncrement describes what fraction of the whole circle the array occupies.
		float repeats =  numberModifier;
		float rotationIncrement = (2 * 3.1415) / repeats;

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
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ) / 2  ; cursor_string++; if (cursor_string > genesize) {return -1;} }

		int repeats = numberModifier % 8;

		// the character after that is the next thing to be arrayed
		cursor_string++; if (cursor_string > genesize) {return -1;}

		unsigned int sequenceOrigin = cursor_string;
		prevCursor_grid = cursor_grid;
		float prevScalingFactor = scalingFactor;

		numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] )  ; cursor_string++; if (cursor_string > genesize) {return -1;} }

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
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }

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
	accumulatedRotation = (0.5 * 3.1415);
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
			x = i % sizeX;
			if (!x) { y = i / sizeX; }

			// if the plant is illuminated, it receives energy.
			if ( lifeGrid[i].energySource == ENERGYSOURCE_LIGHT )
			{
				if (seedGrid[i].stage == STAGE_NULL)
				{
					lifeGrid[i].energy += seedGrid[i].energy / lightEfficiency;
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
						// float takenAmount = lifeGrid[neighbour].energy / plantEfficiency;
						lifeGrid[i].energy += plantEfficiency ;
						clearLifeParticle(neighbour);
						// lifeGrid[neighbour].energy -= plantEfficiency;
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
			nGerminatedSeeds ++;
			continue;
		}
	}
#ifdef THREAD_TIMING_READOUT
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_plantDrawing " << elapsed.count() << " microseconds." << std::endl;
#endif
}


// fade the color in random seedgrid squares if the particle there is not a seed. this makes photon trails fade over time.
void updateSeedgridColor (unsigned int i)
{
	if (seedGrid[i].stage == STAGE_NULL)
	{
		unsigned int a_offset = (i * numberOfFieldsPerVertex) ;
		seedColorGrid[ a_offset  + 3] *= 0.5;
	}
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
			if (extremelyFastNumberFromZeroTo(1) == 0) 		// get blown by the wind only some of the time
			{
				unsigned int neighbour = neighbourOffsets[ weatherGrid[i].direction ] + i;
				if ((grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  ) && seedGrid[neighbour].stage == 0x00  )
				{
					swapSeedParticle( i, neighbour );
					continue;
				}
			}

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



			// if (		j == 0)		{ neighbour = i - sizeX - 1 ;	}
			// else if (	j == 1)		{ neighbour = i - sizeX	 	;	}
			// else if (	j == 2)		{ neighbour = i - sizeX + 1 ;	}
			// else if (	j == 3)		{ neighbour = i + 1	    	;	}
			// else if (	j == 4)		{ neighbour = i - 1 		;	}
			// else if (	j == 5)		{ neighbour = i + sizeX - 1 ;	}
			// else if (	j == 6)		{ neighbour = i + sizeX		;	}
			// else if (	j == 7)		{ neighbour = i + sizeX + 1 ;	}

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
			case 5:
			{
				exampleSentence = plant_ParbasarbTree;
				break;
			}
			case 6:
			{
				exampleSentence = plant_LardGrass;
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
			setAnimal( i, exampleAnimal );
		}
	}
}

void increaseLampBrightness ()
{
	lampBrightness ++;
	lampBrightness = lampBrightness % maxLampBrightness;
}

void decreaseLampBrightness ()
{
	lampBrightness--;
	lampBrightness = lampBrightness % maxLampBrightness;
}

void save ()
{
	printf("SAVING GAME\n");

	WorldInformation newWorldInfo;
	newWorldInfo.nAnimals = animals.size();
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

	std::ofstream out7(std::string("save/colorGrid").c_str());
	out7.write(reinterpret_cast<char*>(& (colorGrid[0]) ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	out7.close();

	// color grids
	std::ofstream out3(std::string("save/lifeColorGrid").c_str());
	out3.write(reinterpret_cast<char*>(& (lifeColorGrid[0]) ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	out3.close();

	// std::ofstream out4(std::string("save/animationGrid").c_str());
	// out4.write(reinterpret_cast<char*>(& (animationGrid[0]) ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	// out4.close();

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
	bytes = animals.size() * sizeof(Animal);
	out8579.write(pointer, bytes);
	out8579.close();
}


void load_materials(unsigned int m)
{
	// load the materials
	if (true)
	{
		resetMaterials();
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
		animals.clear();
		for (int i = 0; i < m; ++i)
		{
			animals.push_back(Animal());
		}
		std::ifstream in556(std::string("save/animals").c_str());
		in556.read( (char *)(&(animals[0])), sizeof(Animal) *  m);
		in556.close();
	}
	printf("loaded animals\n");
}

void load_colorgrids()
{
	std::ifstream in3(std::string("save/colorGrid").c_str());
	in3.read( (char *)( &(colorGrid[0])  ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	in3.close();

	std::ifstream in33(std::string("save/lifeColorGrid").c_str());
	in33.read( (char *)( &(lifeColorGrid[0])  ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	in33.close();

	// std::ifstream in4(std::string("save/animationGrid").c_str());
	// in4.read( (char *)( &(animationGrid[0])), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	// in4.close();

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
			// clearAnimalDrawing(i);
			// clearSeedParticle(i);
			killAnAnimal(i);
		}

		if (lifeGrid[i].identity > 0x00)
		{
			clearLifeParticle(i);
		}
	}
}

void manualErode ()
{
	// remove every cell with a vacuum neighbour.
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

void drawRandomLandscape()
{
	unsigned int randomHillX = extremelyFastNumberFromZeroTo(sizeX);
	unsigned int randomHillWidth = extremelyFastNumberFromZeroTo(500);
	for (int i = 0; i < extremelyFastNumberFromZeroTo(500); ++i)
	{
		unsigned int hillWidthNoise = (RNG() - 0.5) * 100;
		drawAHill(randomHillX, randomHillWidth + hillWidthNoise);
	}
}

void drawLandscapeFromString(std::string genes)
{
	unsigned int i = 0;
	while (i < genes.length())
	{
		char c = genes[i];
		switch (c)
		{
		case 'h': // a hill
			break;
		}
	}

}