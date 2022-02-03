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
#define DRAW_ANIMALS 1

#define RENDERING_THREADS 4

const Color color_lightblue 		= Color( 0.1f, 0.3f, 0.65f, 1.0f );
const Color color_yellow    		= Color( 1.0f, 1.0f, 0.0f, 1.0f );
const Color color_lightgrey 		= Color( 0.75f, 0.75f, 0.75f, 1.0f );
const Color color_grey      		= Color( 0.50f, 0.50f, 0.50f, 1.0f );
const Color color_darkgrey  		= Color( 0.25f, 0.25f, 0.25f, 1.0f );
const Color color_black     		= Color( 0.0f, 0.0f, 0.0f, 1.0f );
const Color color_white_clear     	= Color( 1.0f, 1.0f, 1.0f, 0.25f );
const Color color_purple     		= Color( 0.8f, 0.0f, 0.8f, 1.0f );
const Color color_brightred			= Color( 1.0f, 0.1f, 0.0f, 1.0f);
const Color color_orange			= Color( 1.0f, 0.8f, 0.0f, 1.0f);
const Color color_clear     		= Color( 0.0f, 0.0f, 0.0f, 0.0f );
const Color color_shadow 	    	= Color( 0.0f, 0.0f, 0.0f, 0.5f);
const Color color_defaultSeedColor  = Color( 0.75f, 0.35f, 0.1f, 1.0f );
const Color color_defaultColor     	= Color( 0.35f, 0.35f, 0.35f, 1.0f );
const Color tingeShadow = Color( -1.0f, -1.0f, -1.0f, 0.1f );
const Color phaseTingeSolid =  Color( 1.0f, 1.0f, 1.0f, 0.2f );
const Color phaseTingeLiquid = Color( -1.0f, -1.0f, -1.0f, 0.2f );
const Color phaseTingeGas =    Color( -1.0f, -1.0f, -1.0f, 0.4f );


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


const unsigned int totalSize = sizeX * sizeY;
const unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

float * colorGrid = new float[totalSize * numberOfFieldsPerVertex ];		// the colorgrid is like a painting of the game world which can be drawn to the screen easily, and updated as the game is played. it concerns the physical material.
float * lifeColorGrid  	= new float[totalSize * numberOfFieldsPerVertex ];  // the same but concerning growing plants. Because plant color information is not easily stored anywhere else, this grid must be preserved in save and load operations.
float * animationGrid 	= new float[totalSize * numberOfFieldsPerVertex ];	// the same, but for the sprites of animals. This is updated every turn, and animals carry their own copy of their sprites, so this does not need to be preserved.
float * seedColorGrid  	= new float[totalSize * numberOfFieldsPerVertex];   // the same, but for the colors of seeds and falling photons.
float * ppGrid  	= new float[totalSize * numberOfFieldsPerVertex];

// perform a variety of post processing artistic styles.
// things like 'glowing because of temperature' need to be calculated all the time, because things are constantly changing temperature.
float * postProcessingGrid = new float[totalSize * numberOfFieldsPerVertex];




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
std::string plant_Primordial = std::string( "still need to figure this one out." );




// ANIMAL DRAWING
unsigned int animalCursorFrame = FRAME_A;
unsigned int animalCursorString = 0;
int animalCursorSegmentRadius = 5;
float animalCursorSegmentAngle = 0.0f;
unsigned int animalCursor = 0;
int animalCursorLegThickness = 1;
int animalCursorLegLength = 5;
Color animalCursorColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
unsigned int animalCursorSegmentNumber = 0;
float animalCursorEnergyDebt = 0.0f;
unsigned int animaCursorEnergySource = ENERGYSOURCE_PLANT;
float animalCursorLimbLowerAngle = 0.0f;
float animalCursorLimbUpperAngle = 0.5 * 3.1415f;
unsigned int animalRecursionLevel = 0;


std::string exampleAnimal = std::string(" rzgzcl.c.c.cl.c.c.cl.");





// vec_i2 wind = vec_i2(0, 0);
unsigned int wind;
int defaultTemperature = 300;



unsigned int sunlightDirection = 2;
unsigned int sunlightEnergy = 10;
unsigned int sunlightHeatCoeff = 1;
unsigned int sunlightTemp = 1000;
unsigned int sunlightPenetrationDepth = 20; // light is slightly reduced traveling through solid things. this affects plants in game as well as being an artistic effect. This number is how far the light goes into solid things.
// unsigned int sunlightColor = blackbodyLookup(sunlightTemp)
Color sunlightColor = color_white_clear;


unsigned int nGerminatedSeeds = 0;
unsigned int lampBrightness = 10;


// raw energy values are DIVIDED by these numbers to get the result. So more means less.
unsigned int lightEfficiency   = 10000;
unsigned int plantEfficiency   = 1000;

// except for these two, where the value is what you get from eating one.
unsigned int meatEfficiency    = 1000;
unsigned int seedEfficiency    = 10;
unsigned int mineralEfficiency = 10;



vec_u2 playerCursor = vec_u2(0, 0);

bool sprinkleErodingRain = false;



float maximumDisplayEnergy = 1.0f;
float maximumDisplayTemperature = 1000.0f;

unsigned int visualizer = VISUALIZE_MATERIAL;

std::list<unsigned int> identities;

std::list<vec_u2> v_seeds;


unsigned int animationChangeCount = 0;
unsigned int animationGlobalFrame = FRAME_A;


struct Material
{
	unsigned int crystal_n;
	unsigned int crystal_condition;

	Color color = Color(0.0f, 0.0f, 1.0f, 1.0f);

	unsigned int melting;
	unsigned int boiling;
	int insulativity;
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

struct SeedParticle
{
	unsigned int parentIdentity;
	std::string genes;
	float energy;
	unsigned int stage;
	unsigned int germinationMaterial;
	SeedParticle();
};
SeedParticle::SeedParticle()
{
	this->genes = std::string("");
	this->parentIdentity = 0x00;
	this->energy = 0.0f;
	this->stage = STAGE_NULL;
	this->germinationMaterial = 0;
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

Particle *  grid = new Particle[totalSize];
LifeParticle * lifeGrid = new LifeParticle[totalSize];
SeedParticle * seedGrid = new SeedParticle[totalSize];
std::list<ProposedLifeParticle> v;
std::list<ProposedLifeParticle> v_extrudedParticles;

// struct AnimalSpritePixel
// {
// 	int localPosition ;//= 0;
// 	Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);
// 	AnimalSpritePixel( );
// };

// AnimalSpritePixel::AnimalSpritePixel(
//     // int localPosition, Color color
// )
// {
// 	this->localPosition = 0;
// 	this->color = color_clear ;
// }

struct AnimalSegment
{

	unsigned int position;
	unsigned int animationFrame;
	// std::vector<AnimalSpritePixel> frameA;
	// std::vector<AnimalSpritePixel> frameB;
	// std::vector<AnimalSpritePixel> frameC;

	Color frameA[sizeAnimalSprite * sizeAnimalSprite];
	Color frameB[sizeAnimalSprite * sizeAnimalSprite];
	Color frameC[sizeAnimalSprite * sizeAnimalSprite];

	AnimalSegment();
};

AnimalSegment::AnimalSegment()
{
	this->animationFrame = FRAME_A;
	this->position = 0;

	for (int i = 0; i < (sizeAnimalSprite * sizeAnimalSprite); ++i)
	{
		// this->frameA[i] = AnimalSpritePixel();
		this->frameA[i] = color_clear;
		// this->frameA[i].localPosition = 0;
	}

}

struct Animal
{
	float energy;
	float reproductionCost;
	unsigned int reach;
	unsigned int movementChance;
	// std::vector<AnimalSegment> segments;

	AnimalSegment segments[maxAnimalSegments];

	unsigned int segmentsUsed;

	unsigned int movementFlags;
	unsigned int direction;
	unsigned int energyFlags;
	Animal();
};

std::vector<Animal> animals;

Animal::Animal()
{
	this->energyFlags = ENERGYSOURCE_PLANT;
	this->direction = 4;
	this->energy = 0.0f;
	this->reach = 5;
	this->movementChance = 4;
	this->segmentsUsed = 1;
	// this->segments.push_back(AnimalSegment());

	for (int i = 0; i < maxAnimalSegments; ++i)
	{
		this->segments[i] = AnimalSegment();
	}

	this->movementFlags = MOVEMENT_ONPOWDER | MOVEMENT_ONSOLID;
}


// struct World
// {
// 	unsigned int backgroundTemperature;

// };


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


// return 0 to continue drawing sequence, return 1 to break sequence by one level.
int drawAnimalFromChar (unsigned int i)
{

	std::list<vec_i2> working_vertices;
	std::list<ProposedLifeParticle> segment_particles;



	if (seedGrid[i].parentIdentity < animals.size() )
	{
		Animal * a = &(animals[seedGrid[i].parentIdentity]);
		if (animalCursorString >= seedGrid[i].genes.length()) {return -1;}
		if (animalCursorSegmentNumber >= maxAnimalSegments) {return -1;}
		animalCursorString++;
		char c = seedGrid[i].genes[animalCursorString];
		switch (c)
		{
		// case 'j': // movement modification.
		// {
		// 	animalCursorString++;

		// 	// walks on powder
		// 	if (seedGrid[i].genes[animalCursorString] == 'p')
		// 	{
		// 		a->movementFlags = a->movementFlags |	MOVEMENT_ONPOWDER;
		// 	}

		// 	animalCursorString++;
		// }

		case 's':
		{
#ifdef ANIMAL_DRAWING_READOUT
			printf("sequence\n");
#endif
			break;
		}
		case ' ':
		{
#ifdef ANIMAL_DRAWING_READOUT
			printf("break\n");
#endif
			return -1;

			break;
		}
		case '.':
		{
#ifdef ANIMAL_DRAWING_READOUT
			printf("go to new segment\n");
#endif

			// commit the segment drawing to the sprite
			for (std::list<ProposedLifeParticle>::iterator it = segment_particles.begin(); it != segment_particles.end(); ++it)
			{
				unsigned int i = (it->position.y * sizeAnimalSprite) + it->position.x;
				if ( i < (sizeAnimalSprite * sizeAnimalSprite))
				{
					a->segments[animalCursorSegmentNumber].frameA[i] = it->color;
					a->segments[animalCursorSegmentNumber].frameB[i] = it->color;
					a->segments[animalCursorSegmentNumber].frameC[i] = it->color;
				}
			}

			segment_particles.clear();

			// a->segments.push_back(AnimalSegment());
			animalCursorSegmentNumber++;
			break;
		}


		case 'c':  // commit the working polygon and start a new one.
		{

			// commit the last polygon to the sprite.
			std::list<vec_i2>::iterator it;

			// draw lines connecting the vertices.
			for (it = working_vertices.begin(); it != working_vertices.end(); ++it)
			{
				vec_i2 lineEnd = *(it);
				++it;

				if (it == working_vertices.end())
				{
					it = working_vertices.begin();
					segment_particles.splice(segment_particles.end(), EFLA_E(   lineEnd,  *(it)) );
					break;
				}
				else
				{
					segment_particles.splice(segment_particles.end(), EFLA_E(   lineEnd,  *(it)) );
				}
			}

			working_vertices.clear();

			// draw a n sided polygon in the vertices buffer.
			animalCursorString++; if (animalCursorString > seedGrid[i].genes.length()) { return -1; }
			unsigned int numberModifier = alphanumeric( seedGrid[i].genes[animalCursorString] );
			unsigned int nPolyVertices = numberModifier ;

			for (int i = 0; i < nPolyVertices; ++i)
			{
				float angle = (i * ((1 * 3.1415) / nPolyVertices)  ) + (0.5 * 3.1415);  // only 1 pi so it draws a semi circle.
				working_vertices.push_back ( vec_i2( animalCursorSegmentRadius * cos(angle), animalCursorSegmentRadius * sin(angle)) );
			}




		}

		case 'm':
		{
			// move a vertex, distorting the polygon
			// the first char is which vertex to choose

			animalCursorString++; if (animalCursorString > seedGrid[i].genes.length()) { return -1; }
			unsigned int numberModifier = alphanumeric( seedGrid[i].genes[animalCursorString] );
			unsigned int moveVertex = numberModifier;
			moveVertex = moveVertex % working_vertices.size();

			// the second char is x movement
			animalCursorString++; if (animalCursorString > seedGrid[i].genes.length()) { return -1; }
			numberModifier = alphanumeric( seedGrid[i].genes[animalCursorString] );
			int moveX = numberModifier - 13;

			// the second char is y movement
			animalCursorString++; if (animalCursorString > seedGrid[i].genes.length()) { return -1; }
			numberModifier = alphanumeric( seedGrid[i].genes[animalCursorString] );
			int moveY = numberModifier - 13;

			// working_vertices[moveVertex].x += moveX;
			// working_vertices[moveVertex].y += moveY;

			std::list<vec_i2>::iterator it;
			unsigned int count = 0;
			for (it = working_vertices.begin(); it != working_vertices.end(); ++it)
			{
				if (count == moveVertex)
				{
					it->x += moveX;
					it->y += moveY;
				}

				count++;
			}


		}



// 		case 'c':
// 		{
// #ifdef ANIMAL_DRAWING_READOUT
// 			printf("draw a circle\n");
// #endif
// 			// raster a circle
// 			// int drawingAreaLowerX = -  (sizeAnimalSprite / 2) ; //animalCursorSegmentRadius;
// 			// int drawingAreaLowerY = -  (sizeAnimalSprite / 2) ; //animalCursorSegmentRadius;
// 			// int drawingAreaUpperX = +  (sizeAnimalSprite / 2) ; //animalCursorSegmentRadius;
// 			// int drawingAreaUpperY = +  (sizeAnimalSprite / 2) ; //animalCursorSegmentRadius;
// 			for ( int k = 0; k < sizeAnimalSprite; ++k)
// 			{
// 				for ( int j = 0; j < sizeAnimalSprite; ++j)
// 				{

// 					int animalCursorSegmentRadiusInt = animalCursorSegmentRadius;

// 					int distance  = distanceBetweenPoints( vec_i2(k, j), vec_i2( sizeAnimalSprite / 2, sizeAnimalSprite / 2 ) ) ;

// 					printf("distance %i\n", distance);
// 					if ( distance < animalCursorSegmentRadiusInt )
// 					{
// 						// int pixel =  ((j * sizeX) + k);
// 						int spritePixel = (j * sizeAnimalSprite) + k ;

// 						// a->segments[animalCursorSegmentNumber].frameA[ (j * sizeAnimalSprite) + k ] = AnimalSpritePixel(pixel  , animalCursorColor );
// 						// a->segments[animalCursorSegmentNumber].frameB[ (j * sizeAnimalSprite) + k ] = AnimalSpritePixel(pixel  , animalCursorColor );
// 						// a->segments[animalCursorSegmentNumber].frameC[ (j * sizeAnimalSprite) + k ] = AnimalSpritePixel(pixel  , animalCursorColor );

// 						// printf("smiuuuuueghgh nyueueueughueue %i\n", spritePixel);
// 						a->segments[animalCursorSegmentNumber].frameA[ spritePixel ] = animalCursorColor;
// 						a->segments[animalCursorSegmentNumber].frameB[ spritePixel ] = animalCursorColor;
// 						a->segments[animalCursorSegmentNumber].frameC[ spritePixel ] = animalCursorColor;
// 						// a->segments[animalCursorSegmentNumber].frameA[ spritePixel ].localPosition = pixel;
// 						// a->segments[animalCursorSegmentNumber].frameB[ spritePixel ].localPosition = pixel;
// 						// a->segments[animalCursorSegmentNumber].frameC[ spritePixel ].localPosition = pixel;


// 						printf("X");



// 						animalCursorEnergyDebt += animalCursorColor.a;
// 					}
// 					else
// 					{
// 						printf("O");
// 					}
// 				}
// 				printf("\n");
// 			}
// 			break;
// 		}
// 		case 'l':
// 		{
// #ifdef ANIMAL_DRAWING_READOUT
// 			printf("draw a limb\n");
// #endif
// 			// limb
// 			for (unsigned int frame = 0; frame < 3; ++frame)
// 			{
// 				if (frame == 0 ) {animalCursorFrame = FRAME_A;}
// 				if (frame == 1 ) {animalCursorFrame = FRAME_B;}
// 				if (frame == 2 ) {animalCursorFrame = FRAME_C;}

// 				// draw a line at an angle from the center of the segment
// 				float upperLimbAngle = 0.0f;
// 				float lowerLimbAngle = 0.0f;
// 				// float limbAccumulatedAngle = 0.0f;
// 				float limbAngleOffset = (0.5 * 3.1415); // rotate the global zero to one that makes sense for our maths.

// 				float limbAngleDelta = 0.0f;
// 				// the ANGLE OF THE UPPER LIMB is determined from the vertical
// 				if (  animalCursorFrame == FRAME_A)
// 				{
// 					limbAngleDelta = (0.35f * animalCursorLimbUpperAngle) + animalCursorLimbLowerAngle;

// 				}
// 				else if (  animalCursorFrame == FRAME_B)
// 				{
// 					limbAngleDelta = (0.65f * animalCursorLimbUpperAngle) + animalCursorLimbLowerAngle;
// 					// upperLimbAngle =  limbAngleDelta + limbAngleOffset ;
// 				}

// 				else if (  animalCursorFrame == FRAME_C)
// 				{
// 					limbAngleDelta = (0.0f * animalCursorLimbUpperAngle) + animalCursorLimbLowerAngle;
// 					// upperLimbAngle =  limbAngleDelta + limbAngleOffset ;
// 				}
// 				upperLimbAngle =  limbAngleDelta + limbAngleOffset ;
// 				lowerLimbAngle = upperLimbAngle - (2 * limbAngleDelta) - 3.1415;


// 				// the ANGLE OF THE LOWER LIMB simply reflects that angle

// 				// float lowerLimbAngle = limbAngleOffset -

// 				unsigned int x = a->segments[animalCursorSegmentNumber].position % sizeAnimalSprite;
// 				unsigned int y = a->segments[animalCursorSegmentNumber].position / sizeAnimalSprite;
// 				unsigned int animalCursorX = animalCursor % sizeAnimalSprite;
// 				unsigned int animalCursorY = animalCursor / sizeAnimalSprite;

// 				// limbAccumulatedAngle = limbAngle;
// 				vec_i2 elbow = vec_i2(
// 				                   x + animalCursorX + (animalCursorLegLength * cos(upperLimbAngle)),
// 				                   y + animalCursorY + (animalCursorLegLength * sin(upperLimbAngle) )
// 				               );
// 				// limbAccumulatedAngle += limbAngle;
// 				vec_i2 wrist = vec_i2( elbow.x +   ( animalCursorLegLength * cos(lowerLimbAngle) ) ,
// 				                       elbow.y +   ( animalCursorLegLength * sin(lowerLimbAngle)  )
// 				                     );

// 				std::list<ProposedLifeParticle> v;
// 				v.splice(v.end(), EFLA_E( vec_i2(x, y),  elbow) );
// 				v.splice(v.end(), EFLA_E( elbow,		 wrist) );

// 				for (std::list<ProposedLifeParticle>::iterator it = v.begin(); it != v.end(); ++it)
// 				{
// 					unsigned int i 			 = (it->position.y * sizeAnimalSprite) + it->position.x;
// 					unsigned int shadowIndex = ( (it->position.y - 1) * sizeAnimalSprite) + (it->position.x);
// 					// if ( i < (sizeAnimalSprite*sizeAnimalSprite))
// 					// {

// 					if (i > totalSize) {continue;}

// 					// printf("MINDO KARDEX %u\n" , i);

// 					if (animalCursorFrame == FRAME_A)
// 					{
// 						// a->segments[animalCursorSegmentNumber].frameA.push_back( AnimalSpritePixel(i  ,		   animalCursorColor ));
// 						// a->segments[animalCursorSegmentNumber].frameA.push_back( AnimalSpritePixel(shadowIndex  , color_shadow ));
// 						a->segments[animalCursorSegmentNumber].frameA[i] = animalCursorColor;
// 						// a->segments[animalCursorSegmentNumber].frameA[i].localPosition = i;

// 						if ( shadowIndex < (sizeAnimalSprite * sizeAnimalSprite))
// 						{
// 							a->segments[animalCursorSegmentNumber].frameA[shadowIndex] = addColor( color_shadow , a->segments[animalCursorSegmentNumber].frameA[shadowIndex] );
// 						}
// 						// a->segments[animalCursorSegmentNumber].frameA[shadowIndex].localPosition = shadowIndex;

// 					}
// 					if (animalCursorFrame == FRAME_B)
// 					{
// 						// a->segments[animalCursorSegmentNumber].frameB.push_back( AnimalSpritePixel(i  , animalCursorColor ));
// 						// a->segments[animalCursorSegmentNumber].frameB.push_back( AnimalSpritePixel(shadowIndex  , color_shadow ));
// 						a->segments[animalCursorSegmentNumber].frameB[i] = animalCursorColor;
// 						// a->segments[animalCursorSegmentNumber].frameB[i].localPosition = i;

// 						if ( shadowIndex < (sizeAnimalSprite * sizeAnimalSprite))
// 						{
// 							a->segments[animalCursorSegmentNumber].frameB[shadowIndex] = addColor( color_shadow , a->segments[animalCursorSegmentNumber].frameB[shadowIndex] );
// 						}
// 						// a->segments[animalCursorSegmentNumber].frameB[shadowIndex].localPosition = shadowIndex;
// 					}
// 					if (animalCursorFrame == FRAME_C)
// 					{
// 						// a->segments[animalCursorSegmentNumber].frameC.push_back( AnimalSpritePixel(i  , animalCursorColor ));
// 						// a->segments[animalCursorSegmentNumber].frameC.push_back( AnimalSpritePixel(shadowIndex  , color_shadow ));
// 						a->segments[animalCursorSegmentNumber].frameC[i] = animalCursorColor;
// 						// a->segments[animalCursorSegmentNumber].frameC[i].localPosition = i;

// 						if ( shadowIndex < (sizeAnimalSprite * sizeAnimalSprite))
// 						{
// 							a->segments[animalCursorSegmentNumber].frameB[shadowIndex] = addColor( color_shadow , a->segments[animalCursorSegmentNumber].frameC[shadowIndex] );
// 						}
// 						// a->segments[animalCursorSegmentNumber].frameC[shadowIndex].localPosition = shadowIndex;
// 					}
// 					// }
// 				}
// 			}
// 			break;
// 		}


		case 'r':
		{
			animalCursorString++; if (animalCursorString > seedGrid[i].genes.length()) { return -1; }
			float numberModifier = alphanumeric( seedGrid[i].genes[animalCursorString] );
			numberModifier = numberModifier / 26;
#ifdef ANIMAL_DRAWING_READOUT
			printf("set color R %f, char '%c'\n", numberModifier, seedGrid[i].genes[animalCursorString] );
#endif
			animalCursorColor = Color(  numberModifier, animalCursorColor.g, animalCursorColor.b, 1.0f    );
			// animalCursorString++; if (animalCursorString > seedGrid[i].genes.length()) { return -1; }
			break;
		}

		case 'g':
		{
			animalCursorString++; if (animalCursorString > seedGrid[i].genes.length()) { return -1; }
			float numberModifier = alphanumeric( seedGrid[i].genes[animalCursorString] );
			numberModifier = numberModifier / 26;
#ifdef ANIMAL_DRAWING_READOUT
			printf("set color G %f, char '%c'\n", numberModifier, seedGrid[i].genes[animalCursorString] );
#endif
			animalCursorColor = Color(  animalCursorColor.r, numberModifier, animalCursorColor.b, 1.0f    );
			// animalCursorString++; if (animalCursorString > seedGrid[i].genes.length()) { return -1; }
			break;
		}

		case 'b':
		{
			animalCursorString++; if (animalCursorString > seedGrid[i].genes.length()) { return -1; }
			float numberModifier = alphanumeric( seedGrid[i].genes[animalCursorString] );
			numberModifier = numberModifier / 26;
#ifdef ANIMAL_DRAWING_READOUT
			printf("set color B %f, char '%c'\n", numberModifier, seedGrid[i].genes[animalCursorString] );
#endif
			animalCursorColor = Color(  animalCursorColor.r, animalCursorColor.g, numberModifier, 1.0f    );
			// animalCursorString++; if (animalCursorString > seedGrid[i].genes.length()) { return -1; }
			break;
		}


		default:
		{
			break;
		}
		}

	}

	return 0;
}


// given an animal seed at position i (the method by which they are transported and reproduced), turn it into a complete animal
void drawAnimalFromSeed(unsigned int i)
{
	animalCursorFrame = FRAME_A;
	animalCursorString = 0;
	animalCursorSegmentRadius = 5;
	animalCursorSegmentAngle = 0.0f;
	animalCursor = 0;
	animalCursorLegThickness = 1;
	animalCursorLegLength = 10;
	animalCursorColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
	animalCursorSegmentNumber = 0;
	animalCursorEnergyDebt = 100.0f;

	if (seedGrid[i].parentIdentity < animals.size() && seedGrid[i].stage == STAGE_ANIMAL)
	{
		Animal * a = &(animals[seedGrid[i].parentIdentity]);
		printf("seedGrid[i].parentIdentity %u \n", seedGrid[i].parentIdentity);

		while ( 1)
		{
			if (drawAnimalFromChar(i) < 0 )
			{
				break;
			}
			if (animalCursorString >= seedGrid[i].genes.length()) {break;}
		}
		a->reproductionCost = animalCursorEnergyDebt;
		a->energy = 0;
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

void setErodingRain(  unsigned int i)
{

	seedGrid[i].energy = 0.0f;
	seedGrid[i].stage = STAGE_ERODINGRAIN;
	memcpy( (&seedColorGrid[i * numberOfFieldsPerVertex]) ,  &(color_purple),  sizeof(Color) );
}

void clearSeedParticle( unsigned int i)
{
	seedGrid[i].stage = 0x00;
	seedGrid[i].parentIdentity = 0x00;
	seedGrid[i].energy = 0.0f;
	seedGrid[i].genes = std::string("");
	memcpy( &(seedColorGrid[ i * numberOfFieldsPerVertex ]) , &(color_clear), sizeof(Color) );
}

void toggleErodingRain ()
{
	sprinkleErodingRain = !sprinkleErodingRain ;

	if (!sprinkleErodingRain)
	{

		for (int i = 0; i < totalSize; ++i)
		{
			if (seedGrid[i].stage == STAGE_ERODINGRAIN)
			{
				clearSeedParticle(i);
			}
		}
	}
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


// // swap the particle but duplicate the color
// void swapPhoton(unsigned int a, unsigned int b)
// {
// 	SeedParticle tempSeed = seedGrid[b];
// 	seedGrid[b] = seedGrid[a];
// 	seedGrid[a] = tempSeed;
// 	// float temp_color[4];
// 	unsigned int a_offset = (a * numberOfFieldsPerVertex) ;
// 	unsigned int b_offset = (b * numberOfFieldsPerVertex) ;
// 	// memcpy( temp_color, 				&seedColorGrid[ b_offset ] , 	sizeof(Color) ); // 4x floats of 4 bytes each
// 	memcpy( &seedColorGrid[ b_offset], 	&seedColorGrid[ a_offset] , 	sizeof(Color) );
// 	// memcpy( &seedColorGrid[ a_offset ], temp_color, 					sizeof(Color) );
// }


// travel from the indicated square in the light direction, marking cells as illuminated or dark along your way.
void photate( unsigned int i )
{

	unsigned int currentPosition = i;


	unsigned int blocked = 0;

	// unsigned int applicableBrightness = lampBrightness;


	// printf("nuguet %u \n", applicableBrightness);
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

		    // grid[currentPosition].phase == PHASE_LIQUID ||

		    // grid[currentPosition].phase == PHASE_GAS ||
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


			}



		}


		if (x == 0 || y == 0 || x >= sizeX || y >= sizeY) {break;}

	}



}


void setAnimalSpritePixel ( AnimalSegment  s, unsigned int pixelIndex )
{
	// unsigned int j_offset = (s->position + p.localPosition) ;
	// unsigned int j__color_offset = (j_offset * numberOfFieldsPerVertex) ;


	// get the segment position.
	// s->position


	// find the x, y offset of the pixel from the origin in sprite coordinates.
	int pixelX = (pixelIndex % sizeAnimalSprite) - (sizeAnimalSprite / 2);
	int pixelY = (pixelIndex / sizeAnimalSprite) - (sizeAnimalSprite / 2);

	int segmentX = s.position % sizeX;
	int segmentY = s.position / sizeX;

	int worldX = segmentX + pixelX;
	int worldY = segmentY + pixelY;

	int worldI = (worldY * sizeX) + worldX;


	if ( worldI > totalSize) {return;}

	int j__color_offset = (worldI * numberOfFieldsPerVertex) ;




	if (s.animationFrame == FRAME_A)
	{



		memcpy( &animationGrid[ j__color_offset], 	&( s.frameA[pixelIndex]  ) , 	sizeof(Color) );


	}
	else if (s.animationFrame == FRAME_B)
	{


		memcpy( &animationGrid[ j__color_offset], 	&( s.frameB[pixelIndex]  ) , 	sizeof(Color) );

	}
	else if (s.animationFrame == FRAME_C)
	{

		memcpy( &animationGrid[ j__color_offset], 	&( s.frameC[pixelIndex]  ) , 	sizeof(Color) );


	}




}

void swapAnimalSpritePixel (unsigned int a, unsigned int b)
{
	float temp_color[4];
	unsigned int a_offset = (a * numberOfFieldsPerVertex) ;
	unsigned int b_offset = (b * numberOfFieldsPerVertex) ;
	memcpy( temp_color, 				&animationGrid[ b_offset ] , 		sizeof(Color) ); // 4x floats of 4 bytes each
	memcpy( &animationGrid[ b_offset], 	&animationGrid[ a_offset] , 	sizeof(Color) );
	memcpy( &animationGrid[ a_offset ], temp_color, 						sizeof(Color) );
}

void clearAnimalSpritePixel(unsigned int i)
{
	unsigned int a_offset = (i * numberOfFieldsPerVertex) ;
	memcpy( &animationGrid[ a_offset], 	&(color_clear) , 	sizeof(Color) );
}

void setAnimal(unsigned int i)
{
	seedGrid[i].genes = exampleAnimal;
	seedGrid[i].stage = STAGE_ANIMAL;
	seedGrid[i].energy = 0.0f;
	Animal newAnimal = Animal();
	animals.push_back(newAnimal);
	seedGrid[i].parentIdentity = animals.size() - 1;

#ifdef PLANT_DRAWING_READOUT
	printf("seed with energy debt %f \n", energyDebt);
#endif

	memcpy( (&seedColorGrid[i * numberOfFieldsPerVertex]) ,  &(animalCursorColor),  sizeof(Color) );
	drawAnimalFromSeed(i);
}

void mutateSentence ( std::string * genes )
{
	size_t genesLength = genes->length();

	for (unsigned int i = 0; i < genesLength; ++i)
	{
		// // swap a letter
		if (extremelyFastNumberFromZeroTo(256) == 0)
		{
			// 	// https://stackoverflow.com/questions/20132650/how-to-select-random-letters-in-c
			(*genes)[i] = (char)('a' + rand() % 26);
		}

		// // add a letter
		if (extremelyFastNumberFromZeroTo(256) == 0)
		{
			// char randomCharacter = (char)('a' + rand() % 26);
			// genes->insert(    extremelyFastNumberFromZeroTo( (genes->length() - 1) )  ,  &randomCharacter  );
			genes->push_back(  (char)('a' + rand() % 26)  );
		}

		// // add a punctuation
		// if (extremelyFastNumberFromZeroTo(256) == 0)
		// {

		// 	char randomCharacter = ' '; ;//= (char)('a' + rand() % 26);

		// 	unsigned int randomCharacterIndex = extremelyFastNumberFromZeroTo(3);
		// 	switch (randomCharacterIndex)
		// 	{
		// 	case 0:
		// 	{
		// 		randomCharacter = ' ';
		// 		break;
		// 	}
		// 	case 1:
		// 	{
		// 		randomCharacter = '.';
		// 		break;
		// 	}

		// 	case 3:
		// 	{
		// 		randomCharacter = ',';
		// 		break;
		// 	}
		// 	}
	}
}

void incrementAnimalSegmentPositions (unsigned int animalIndex, unsigned int i, bool falling)
{

	if (animalIndex < animals.size())
	{
		Animal  a = animals[animalIndex];

		if (a.energy > a.reproductionCost)
		{
			unsigned int nSolidNeighbours = 0;
			for (unsigned int j = 0; j < N_NEIGHBOURS; ++j)
			{
				unsigned int neighbour = neighbourOffsets[j] + i;
				if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS )
				{
					printf("animal reproduced\n");
					setAnimal( neighbour );
					mutateSentence(  &(seedGrid[neighbour].genes) );
					a.energy = 0.0f;
					return;
					break;
				}
			}
		}

		bool segmentPhase = 0;

		// Update animal segment positions. Only do this if the animal has actually moved (otherwise it will pile into one square).
		// if (
		// 	// !(a.segments.empty())

		// 	)
		// {
		// 	// printf("KUUUUNNNN %lu\n", a.segments.size());
		if (a.segments[0].position != i)
		{
			// set the position of the 0th segment to the new index, and everyone elses position is shifted forward by 1.
			bool segmentPhase = false;

			if (a.segments[0].animationFrame == FRAME_A) {segmentPhase = true;}

			for (unsigned int j = 0; j < maxAnimalSegments; ++j)
			{

				if ( j >= a.segmentsUsed)

				{
					continue;
				}


				if (falling)
				{
					a.segments[j].animationFrame = FRAME_C;
				}
				else
				{
					if (segmentPhase)
					{
						a.segments[j].animationFrame = FRAME_B;
					}
					else
					{
						a.segments[j].animationFrame = FRAME_A;
					}
					segmentPhase = !segmentPhase;
				}
			}

			for ( int j = (a.segmentsUsed - 1); j > 0; --j)
			{
				a.segments[j].position = a.segments[j - 1].position;
			}

			a.segments[0].position = i;
		}
	}
}

void updateAnimalDrawing(unsigned int i)
{
	if (seedGrid[i].parentIdentity < animals.size())
	{
		Animal * a = &(animals[seedGrid[i].parentIdentity]);

		// std::vector<AnimalSegment>::iterator s;

		unsigned int count = 0;
		// for (s = a->segments.begin(); s !=  a->segments.end(); ++s)
		for (	int j = 0; j < a->segmentsUsed; j++ )
		{
			// AnimalSegment * s = &(a->segments[j]);
			// std::vector<AnimalSpritePixel>::iterator p;


			for (unsigned int k = 0; k < (sizeAnimalSprite * sizeAnimalSprite); ++k)
			{
				setAnimalSpritePixel( a->segments[j], k);
			}

			count++;

		}
	}
}



void setParticle(unsigned int material, unsigned int i)
{
	grid[i].temperature = defaultTemperature;
	grid[i].material = material;
	grid[i].phase = PHASE_POWDER;
	unsigned int a_offset = (i * numberOfFieldsPerVertex);
	memcpy( &colorGrid[ a_offset ], & (materials[material].color), 16 );
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
	Material vacuum = Material();
	vacuum.color = color_clear;
	vacuum.melting = 0;
	vacuum.boiling = 0;
	vacuum.insulativity = 1000;
	materials.push_back(vacuum);

	Material bumdirt = Material();

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
	unsigned int a_offset = (i * numberOfFieldsPerVertex) ;
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
	animationGrid[ a_offset + 0] = 0.0f;
	animationGrid[ a_offset + 1] = 0.0f;
	animationGrid[ a_offset + 2] = 0.0f;
	animationGrid[ a_offset + 3] = 0.0f;
	animationGrid[ a_offset + 4] = fx;
	animationGrid[ a_offset + 5] = fy;
	seedColorGrid[ 	a_offset + 0] = 0.0f;
	seedColorGrid[ 	a_offset + 1] = 0.0f;
	seedColorGrid[ 	a_offset + 2] = 0.0f;
	seedColorGrid[ 	a_offset + 3] = 0.5f;
	seedColorGrid[ 	a_offset + 4] = fx;
	seedColorGrid[ 	a_offset + 5] = fy;

}

void clearColorGridB()
{
	memset( animationGrid, 0.0f, sizeof(float) * numberOfFieldsPerVertex * totalSize );
	unsigned int x = 0;
	unsigned int y = 0;
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		x = i % sizeX;
		y = i / sizeX;
		float fx = x;
		float fy = y;
		unsigned int a_offset = (i * numberOfFieldsPerVertex) ;
		animationGrid[ a_offset + 4] = fx;
		animationGrid[ a_offset + 5] = fy;
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
	else if (temperature < 772)
	{
		// faint red
		return Color(0.16f, 0.0f, 0.0f, 0.03f);
	}
	else if (temperature < 852)
	{
		// blood red
		return  Color(0.33f, 0.0f, 0.0f, 0.11f);
	}
	else if (temperature < 908)
	{
		// dark cherry
		return Color(0.5f, 0.0f, 0.0f, 0.20f);
	}
	else if (temperature < 963)
	{
		// medium cherry
		return Color(0.66f, 0.0f, 0.0f, 0.27f);
	}
	else if (temperature < 1019)
	{
		// cherry
		return Color(0.833f, 0.0f, 0.0f, 0.36f);
	}
	else if (temperature < 1060)
	{
		// bright cherry
		return  Color(1.0f, 0.0f, 0.0f, 0.45f);
	}
	else if (temperature < 1116)
	{
		// salmon (??)
		return  Color(1.0f, 0.25f, 0.0f, 0.53f);
	}
	else if (temperature < 1188)
	{
		// dark orange
		return Color(1.0f, 0.5f, 0.0f, 0.61f);
	}
	else if (temperature < 1213)
	{
		// orange
		return Color(1.0f, 0.75f, 0.0f, 0.70f);
	}
	else if (temperature < 1272)
	{
		// lemon
		return Color(1.0f, 1.0f, 0.0f, 0.78f);
	}
	else if (temperature < 1352)
	{
		// light yellow
		return  Color(1.0f, 1.0f, 0.5f, 0.86f);
	}
	else if (temperature < 5000)
	{
		// white
		return  Color(1.0f, 1.0f, 1.0f, 0.91f);
	}
	else if (temperature < 10000)
	{
		// cool white
		return  Color(0.95f, 0.95f, 1.0f, 1.0f);
	}
	else
	{
		// blue
		return Color(0.9f, 0.9f, 1.0f, 1.0f);
	}
}

void createRandomWorld()
{

	clearGrids();

	// materials.clear();
	// materials.push_back(bumdirt);

	resetMaterials();

	sunlightColor = blackbodyLookup(sunlightTemp);

	defaultTemperature = RNG() * 1500;


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



	for (unsigned int k = 0; k < materials.size(); ++k)
	{
		/* code */

		unsigned int availability = extremelyFastNumberFromZeroTo(50);


		for (unsigned int i = 0; i < sizeX; ++i)
		{

			if (extremelyFastNumberFromZeroTo(100) == 0)
			{
				unsigned int radius = extremelyFastNumberFromZeroTo(availability );

				// float frandomX = RNG() * sizeX;
				// float frandomY = RNG() * sizeY;

				// unsigned int randomX = frandomX; //extremelyFastNumberFromZeroTo(sizeX );
				// unsigned int randomY = frandomY; //extremelyFastNumberFromZeroTo(sizeY );

				// printf("ranom x %u, random y %u\n", randomX,  randomY);

				// unsigned int randomi = (randomY * sizeX) + randomX;





				paintMaterialCircle(i + (50 * sizeX * (k + 1)  ), radius , k , 10000000 );
			}

		}
	}

	// // setup the x and y positions in the color grid. these never change so you can just calculate them once.
	// unsigned int x = 0;
	// unsigned int y = 0;
	// for (unsigned int i = 0; i < totalSize; ++i)
	// {
	// 	x = i % sizeX;
	// 	y = i / sizeX;
	// 	// float fx = x;
	// 	// float fy = y;


	// 	if (i > sizeX && i < 50 * sizeX)

	// 	{

	// 		if (RNG() < availability)
	// 		{

	// 			setParticle( k, i);
	// 		}
	// 	}
	// }









	// setAnimal( ( 60 * sizeX ) + 50);




	// // set everything to a random temperature
	// float frandomtemp = RNG() * 1000;
	// unsigned int randomTemp = frandomtemp;
	// for (unsigned int i = 0; i < totalSize; ++i)
	// {
	// 	grid[i].temperature = randomTemp;
	// }



}







void initialize ()
{
	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	srand((unsigned int)time(NULL));
	// setupExtremelyFastNumberGenerators();

	cursor_seedColor = color_yellow;
	clearGrids();



	resetMaterials();

	// createRandomWorld();


	// load();



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

	grid[i].temperature = 10000000;
}





// updates a location on the color grid with a material's new color information, based on phase and temperature.
void materialPostProcess(unsigned int i)
{

	// return;
	unsigned int x = i % sizeX;
	unsigned int y = i / sizeX;

	// float fx = x;
	// float fy = y;

	// figure out the blackbody temperature.
	// https://www.iforgeiron.com/uploads/monthly_2015_08/ForgingTemperatureColors.jpg.2948a8585818155020d14fe17038cf33.jpg
	// it is drawn directly over the top of the

	Color ppColor =  color_clear;




	// if (seedGrid[i].stage == STAGE_NULL && seedGrid[i].energy == 0x00)
	// {
	// 	ppColor = addColor(ppColor, tingeShadow);
	// }
	// Color ppColor = color_clear;


	if (grid[i].phase != PHASE_VACUUM)
	{

		if (grid[i].material  < materials.size())
		{
			ppColor =  materials[ grid[i].material ].color  ;//materialColor(grid[i].material);  //

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

	// else
	// {
	// 	ppColor = color_clear;
	// }

	colorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = ppColor.r;
	colorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = ppColor.g;
	colorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = ppColor.b;
	colorGrid[ (i * numberOfFieldsPerVertex) + 3 ] = ppColor.a;
	// colorGrid[ (i * numberOfFieldsPerVertex) + 4 ] = fx;
	// colorGrid[ (i * numberOfFieldsPerVertex) + 5 ] = fy;
}














// unsigned int currentPosition = i;
// 		unsigned int squareBelow = currentPosition - sizeX;











void thread_temperature2_sector ( unsigned int from, unsigned int to )
{



	// printf("from %u to %u \n", from, to);

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
				// if neighbour is a vacuum, radiate heat away into space. more so if it is hotter.
				int crntmp = grid[currentPosition].temperature;
				int dftmp = defaultTemperature;
				int radiantHeat = (crntmp - dftmp) / 50;
				grid[currentPosition].temperature -= radiantHeat;
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
					neighbour = neighbourOffsets[wind] + currentPosition;
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







		float ffrom = (i  - 0.5  ) ; ///(n_threads+1) )    * totalSize );
		float fto   = (i + 0.5) ;///(n_threads+1) )    * totalSize );

		ffrom = ffrom / (n_threads );
		fto = fto / (n_threads );

		// printf("natural from %f to %f \n", ffrom, fto);


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

// void physics_sector (unsigned int from, unsigned int to)
// {
// #ifdef THREAD_TIMING_READOUT
// 	auto start = std::chrono::steady_clock::now();
// #endif

// 	for (unsigned int i = from; i < to; ++i)
// 	{
// 		unsigned int currentPosition = i;
// 		unsigned int squareBelow = currentPosition - sizeX;

// 		if (grid[currentPosition].phase  == PHASE_POWDER)
// 		{
// 			for (unsigned int k = 0; k < 1; ++k) // instead of checking all of them every turn, you can check half or less and it doesn't make a difference for gameplay.
// 			{
// 				unsigned int index = extremelyFastNumberFromZeroTo(2);
// 				// chemistry(currentPosition, neighbours[index]);
// 				if ((grid[neighbourOffsets[index] + i].phase == PHASE_VACUUM) ||  (grid[neighbourOffsets[index] + i].phase == PHASE_GAS) ||  (grid[neighbourOffsets[index] + i].phase == PHASE_LIQUID)  )
// 				{
// 					swapParticle(currentPosition, neighbourOffsets[index] + i);
// 					currentPosition = neighbourOffsets[index] + i;
// 					break;
// 				}
// 			}
// 		}

// 		// movement instructions for LIQUIDS
// 		else if (grid[currentPosition].phase == PHASE_LIQUID)
// 		{
// 			unsigned int offset = extremelyFastNumberFromZeroTo(4);
// 			for (unsigned int k = 0; k < 5; ++k)
// 			{
// 				unsigned int index = (k + offset) % 5;
// 				if ((grid[neighbourOffsets[index] + i].phase == PHASE_VACUUM) || (grid[neighbourOffsets[index] + i].phase == PHASE_GAS) ||  (grid[neighbourOffsets[index] + i].phase == PHASE_LIQUID)     )
// 				{
// 					swapParticle(currentPosition, neighbourOffsets[index] + i);
// 					currentPosition = neighbourOffsets[index] + i;
// 					break;
// 				}
// 			}
// 		}

// 		// movement instructions for GASES
// 		else if (grid[currentPosition].phase == PHASE_GAS)
// 		{
// 			for (unsigned int j = 0; j < 3; ++j)
// 			{
// 				unsigned int index = extremelyFastNumberFromZeroTo(7);

// 				// alternate between wind movement and random scatter movement, to look more natural.
// 				if (extremelyFastNumberFromZeroTo(1) == 0)
// 				{
// 					// the wind pushes in a certain direction.
// 					// however wind is expressed as a vec_i2, and we need it as a single number.
// 					// this is basically a very fast lookup table to tell you the answer.
// 					if (wind.y > 0)
// 					{
// 						index = 5; // directly up
// 						if (wind.x > 0)
// 						{
// 							index = 4; // up and to the right
// 						}
// 						else if (wind.x < 0)
// 						{
// 							index = 7; // and so on, allowing the index to remain at its original random value if the wind in that axis was 0.
// 						}
// 					}
// 					else if (wind.y < 0)
// 					{
// 						index = 1; // directly down
// 						if (wind.x > 0)
// 						{
// 							index = 3;
// 						}
// 						else if (wind.x < 0)
// 						{
// 							index = 1;
// 						}
// 					}
// 					else
// 					{
// 						if (wind.x > 0)
// 						{
// 							index = 4;
// 						}
// 						else if (wind.x < 0)
// 						{
// 							index = 0;
// 						}
// 					}
// 				}

// 				if (grid[neighbourOffsets[index] + i].phase  == PHASE_VACUUM || (grid[neighbourOffsets[index] + i].phase == PHASE_GAS) )
// 				{
// 					swapParticle(currentPosition, neighbourOffsets[index] + i);
// 					break;
// 				}
// 			}

// 		}
// 	}
// #ifdef THREAD_TIMING_READOUT
// 	auto end = std::chrono::steady_clock::now();
// 	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
// 	std::cout << "thread_physics_sector " << elapsed.count() << " microseconds." << std::endl;
// #endif
// }

void thread_physics ()
{

#ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
#endif


	if (extremelyFastNumberFromZeroTo(100) == 0)
	{
// make sure randomness is random
		seedExtremelyFastNumberGenerators();



	}


	// blow the wind
	if (extremelyFastNumberFromZeroTo(100) == 0)
	{
		// int16_t windRand =  extremelyFastNumberFromZeroTo( 2);
		// wind.x = windRand - 1;
		// windRand =  extremelyFastNumberFromZeroTo( 2);
		// wind.y = windRand - 1;




		wind = extremelyFastNumberFromZeroTo(7);
	}

	// sprinkle eroding rain
	if (sprinkleErodingRain)
	{
		for (int i = (sizeY - 2) * sizeX; i < (sizeY - 1)*sizeX; ++i)
		{
			// if (extremelyFastNumberFromZeroTo(lampBrightness) == 0)
			// {
			setErodingRain(   i);
			// }
		}
	}

	else
	{
		for (int i = (sizeY - 2) * sizeX; i < (sizeY - 1)*sizeX; ++i)
		{




			if (extremelyFastNumberFromZeroTo(4) == 0)
			{
				// setPhoton(   i);
				photate(i);

			}
		}
	}


	unsigned int processChunkSize = (totalSize / 10);
	for (unsigned int i = 0; i < processChunkSize; ++i)
	{
		// if (extremelyFastNumberFromZeroTo(10) == 0)
		// {

		unsigned int x = extremelyFastNumberFromZeroTo(sizeX - 1);
		unsigned int y = extremelyFastNumberFromZeroTo(sizeY - 1);


		materialPostProcess(  (y * sizeX) + x  );
		// }
	}

// 	unsigned int quad1 = totalSize / 4;
// 	unsigned int quad2 = totalSize / 2;
// 	unsigned int quad3 = (totalSize / 4) * 3;
// 	boost::thread t11{ physics_sector, (sizeX + 1), quad1 };
// 	boost::thread t12{ physics_sector, (quad1 + 1), quad2 };
// 	boost::thread t13{ physics_sector, (quad2 + 1), quad3 };
// 	boost::thread t14{ physics_sector, (quad3 + 1), (totalSize - sizeX - 1) };
// 	t11.join();
// 	t12.join();
// 	t13.join();
// 	t14.join();

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
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 0 ] = grid[i].temperature / maximumDisplayTemperature;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 1 ] = grid[i].temperature / maximumDisplayTemperature;
			energyColorGrid[ (i * numberOfFieldsPerVertex) + 2 ] = grid[i].temperature / maximumDisplayTemperature;
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

	else
	{
		unsigned int nVertsToRenderThisTurn = 1 * totalSize;
		long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;




		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, colorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, lifeColorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);



		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, animationGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

		glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, seedColorGrid, GL_DYNAMIC_DRAW );
		glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);



		// glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, ppGrid, GL_DYNAMIC_DRAW );
		// glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);


		postDraw();


		for (unsigned int i = 0; i < totalSize; ++i)
		{
			animationGrid[ (i * numberOfFieldsPerVertex ) + 3] = 0.0f;
		}

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

	case 'm':
	{

		cursor_string++; if (cursor_string > genesize) {return -1;}

		int numberModifier = 0.0f;
		while (!numberModifier) {	numberModifier = alphanumeric( genes[cursor_string] ); cursor_string++; if (cursor_string > genesize) {return -1;} }


		/*
		#define MATERIAL_WATER 		 8
		#define MATERIAL_QUARTZ 	 9
		#define MATERIAL_AMPHIBOLE 	 10
		#define MATERIAL_OLIVINE 	 11
		*/

		//
		numberModifier = numberModifier % materials.size();

		// if 		(numberModifier == 0) 	{cursor_germinationMaterial = MATERIAL_WATER;}
		// else if (numberModifier == 1)   {cursor_germinationMaterial = MATERIAL_QUARTZ;}
		// else if (numberModifier == 2)   {cursor_germinationMaterial = MATERIAL_AMPHIBOLE;}
		// else if (numberModifier == 3)   {
		cursor_germinationMaterial = numberModifier;
		// }


		// printf("set germination material to %u\n", cursor_germinationMaterial);


		// set germination material
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
	cursor_germinationMaterial =  0;  //MATERIAL_VACUUM;//MATERIAL_QUARTZ;

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
				// seedGrid[i].germinationMaterial = cursor_germinationMaterial;
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

			// unsigned int squareBelow = i - sizeX;
			// unsigned int squareAbove = i + sizeX;
			// unsigned int neighbours[] =
			// {
			// 	squareBelow - 1,
			// 	squareBelow,
			// 	squareBelow + 1,
			// 	i + 1,
			// 	squareAbove + 1,
			// 	squareAbove,
			// 	squareAbove - 1,
			// 	i - 1,
			// };





			// if the plant is illuminated, it receives energy.
			if ( lifeGrid[i].energySource == ENERGYSOURCE_LIGHT )
			{

				if (seedGrid[i].stage == STAGE_NULL)
				{
					lifeGrid[i].energy += seedGrid[i].energy / lightEfficiency;
				}

			}






			unsigned int neighbourMaterialA = MATERIAL_VACUUM;

			// for (unsigned int j = 0; j < N_NEIGHBOURS; ++j)
			// {


			unsigned int neighbour = neighbourOffsets[ extremelyFastNumberFromZeroTo(N_NEIGHBOURS) ] + i;

			// hot plants light on fire
			// if (true)
			// {
			// 	if (grid[neighbour].material != PHASE_VACUUM)
			// 	{
			// 		if (grid[neighbour].temperature > 600 )
			// 		{
			// 			// printf("a plant lit on fire\n");
			// 			clearLifeParticle(i);
			// 			setParticle(MATERIAL_FIRE, i);
			// 			grid[i].phase = PHASE_GAS;
			// 			grid[i].temperature = 1000;
			// 			break;
			// 		}
			// 	}
			// }





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
					if (grid[i].material != neighbourMaterialA)
					{
						// you have found a pair of dissimilar neighbours
						// if (extremelyFastNumberFromZeroTo(64) == 0x00)
						// {
						lifeGrid[i].energy +=  mineralEfficiency;
						// }
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
						// if (extremelyFastNumberFromZeroTo(64) == 0x00)
						// {
						float takenAmount = lifeGrid[neighbour].energy / plantEfficiency;
						lifeGrid[i].energy += takenAmount ;
						lifeGrid[neighbour].energy -= takenAmount;
						// }
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
						// if (extremelyFastNumberFromZeroTo(64) == 0x00)
						// {
						lifeGrid[i].energy += seedEfficiency;
						// }
					}
				}
			}
			// }
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
// #ifdef THREAD_TIMING_READOUT
	auto start = std::chrono::steady_clock::now();
// #endif
	for (unsigned int i =  0; i < totalSize; ++i)
	{

		// if (crudOps) {printf("stopped plantdrawing because of crud\n"); return;}


		if (seedGrid[i].stage == STAGE_SPROUT)
		{
			// if (seedGrid[i].parentIdentity > 0)
			// {
			// if (grid[i].material == seedGrid[i].germinationMaterial)
			// {


			// printf("Drawing plant... ");
			drawPlantFromSeed(seedGrid[i].genes, i);
			// printf("done.\n");

			clearSeedParticle(i);
			nGerminatedSeeds ++;
			continue;
			// }
			// }
		}
	}
// #ifdef THREAD_TIMING_READOUT
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_plantDrawing " << elapsed.count() << " microseconds." << std::endl;
// #endif
}

unsigned int walkAnAnimal(unsigned int i)
{
	unsigned int currentPosition = i;
	unsigned int squareBelow = currentPosition - sizeX;

	if (seedGrid[i].parentIdentity < animals.size())
	{
		Animal * a = &(animals[seedGrid[i].parentIdentity]);

		bool moved = false;
		// bool haveAWalkableNeighbour = false;

		// int reach_int = a->reach;

		if (extremelyFastNumberFromZeroTo(a->movementChance) == 0)
		{
			for (unsigned int move = 0; move < a->reach; ++move)
			{
				// bool movedThisTurn = false;
				// bool ateANeighbour = false;

				// check the neighbour cells starting in the direction of travel.
				// then, check the cells closest to the direction of travel.
				// then, the cells at right angles to the direction of travel. and so on. this is vital to coherent movement.
				int sign = 1;
				unsigned int neighbour = (currentPosition + neighbourOffsets[a->direction]) % totalSize;
				for (unsigned int j = 0; j < N_NEIGHBOURS; ++j)
				{
					// starting at a place in the neighbours array, move n steps to the right, then n+1 steps left.. access it with neighbourOffsets[k];
					int k = ( (a->direction) + (j * sign)) % N_NEIGHBOURS;
					k += (extremelyFastNumberFromZeroTo(2) - 1); // also, do it with some jitter or else you will get stuck constantly.
					k = k % N_NEIGHBOURS;
					if (k < 0) {k += N_NEIGHBOURS;}
					sign *= -1;

					neighbour = (currentPosition + neighbourOffsets[k] ) % totalSize;

					// if one of the neighbouring cells is a material type and phase that the animal can exist within
					if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS || grid[neighbour].phase == PHASE_LIQUID)
					{
						break;
					}
				}

				// if the neighbour is a food the animal can eat, eat it
				if (  (a->energyFlags & ENERGYSOURCE_LIGHT ) == ENERGYSOURCE_LIGHT   )
				{
					if (seedGrid[neighbour].stage == STAGE_NULL )
					{
						a->energy += seedGrid[neighbour].energy;
					}
				}

				else if (  (a->energyFlags & ENERGYSOURCE_SEED ) == ENERGYSOURCE_SEED   )
				{
					if (seedGrid[neighbour].stage == STAGE_BUD || seedGrid[neighbour].stage == STAGE_FRUIT ||  seedGrid[neighbour].stage == STAGE_SPROUT )
					{
						a->energy += 10.0f ;
						clearSeedParticle(neighbour);
					}
				}

				else if (  (a->energyFlags & ENERGYSOURCE_PLANT ) == ENERGYSOURCE_PLANT   )
				{

					if (lifeGrid[neighbour].identity > 0x00)
					{
						if (lifeGrid[neighbour].energy > 0.0f)
						{
							a->energy += lifeGrid[neighbour].energy;
							lifeGrid[neighbour].energy = 0.0f;
							clearLifeParticle(neighbour);
						}
					}
				}

				else if (  (a->energyFlags & ENERGYSOURCE_MINERAL ) == ENERGYSOURCE_MINERAL   )
				{
					if (grid[neighbour].phase != PHASE_VACUUM)
					{
						a->energy += 1.0f;
						clearParticle( neighbour);
					}
				}

				else if (  (a->energyFlags & ENERGYSOURCE_ANIMAL ) == ENERGYSOURCE_ANIMAL   )
				{

					if (seedGrid[neighbour].stage == STAGE_ANIMAL )
					{
						a->energy += animals[seedGrid[neighbour].parentIdentity].energy;
						printf("an animal ate another animal!\n");
						clearSeedParticle(neighbour);
					}
				}






				if (  (a->movementFlags & MOVEMENT_ONPOWDER ) == MOVEMENT_ONPOWDER   )
				{
					for (int l = 0; l < N_NEIGHBOURS; ++l) // and it has a neighbour of a type and phase the animal can walk on
					{
						int neighboursNeighbour = (neighbour + neighbourOffsets[l]);
						if (neighboursNeighbour != currentPosition && neighboursNeighbour != i)
						{
							if (  grid[ neighboursNeighbour ] .phase == PHASE_POWDER )
							{
								currentPosition = neighbour; // say that this cell is the current position, and then break.
								moved = true;
								// break;
							}
						}
					}
					// }
					// break;
				}



				if (  (a->movementFlags & MOVEMENT_INPLANTS ) == MOVEMENT_INPLANTS   )
				{
					// if (grid[neighbour].phase == PHASE_VACUUM) // if one of the neighbouring cells is a material type and phase that the animal can exist within
					// {
					if (lifeGrid[neighbour].identity > 0x00)
					{
						currentPosition = neighbour; // say that this cell is the current position, and then break.
						moved = true;
						// break;
					}
					// }

					// break;
				}


				if (  (a->movementFlags & MOVEMENT_INLIQUID ) == MOVEMENT_INLIQUID   )
				{
					if (grid[neighbour].phase == PHASE_LIQUID) // if one of the neighbouring cells is a material type and phase that the animal can exist within
					{
						currentPosition = neighbour; // say that this cell is the current position, and then break.
						moved = true;
						// break;
					}

					// break;
				}


				if (  (a->movementFlags & MOVEMENT_INAIR ) == MOVEMENT_INAIR   )
				{
					if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS) // if one of the neighbouring cells is a material type and phase that the animal can exist within
					{
						currentPosition = neighbour; // say that this cell is the current position, and then break.
						moved = true;
						// break;
					}

					// break;
				}


				if (  (a->movementFlags & MOVEMENT_ONSOLID ) == MOVEMENT_ONSOLID   )
				{
					if (grid[neighbour].phase == PHASE_VACUUM) // if one of the neighbouring cells is a material type and phase that the animal can exist within
					{
						for (int l = 0; l < N_NEIGHBOURS; ++l) // and it has a neighbour of a type and phase the animal can walk on
						{
							int neighboursNeighbour = (neighbour + neighbourOffsets[l]);
							if (neighboursNeighbour != currentPosition && neighboursNeighbour != i) {


								if (  grid[ neighboursNeighbour ] .phase == PHASE_SOLID )
								{
									currentPosition = neighbour; // say that this cell is the current position, and then break.
									moved = true;
									// break;
								}
							}
						}
					}

					// break;
				}
				// }
			}
		}
		// else
		// {
		// 	for (int l = 0; l < N_NEIGHBOURS; ++l)
		// 	{
		// 		int neighbour = (neighbourOffsets[l] + currentPosition);
		// 		if (  grid[ neighbour ] .phase == PHASE_POWDER )
		// 		{
		// 			haveAWalkableNeighbour = true;
		// 			break;
		// 		}
		// 	}
		// }

		// after having repeated <reach> moves, swap the cell and update the segment positions.
		if (	moved )
		{
			printf("MOVED TO %u from %u\n", currentPosition, i);
			swapSeedParticle(i, currentPosition);
			incrementAnimalSegmentPositions( seedGrid[i].parentIdentity, i, false );
		}
		else
		{


			if (((a->movementFlags & MOVEMENT_INAIR) !=  MOVEMENT_INAIR)    )
			{
				if (grid[squareBelow].phase == PHASE_VACUUM || grid[squareBelow].phase == PHASE_GAS )
				{
					swapSeedParticle(currentPosition, squareBelow);
					incrementAnimalSegmentPositions( seedGrid[i].parentIdentity, currentPosition, true );
					printf("fell down\n");
				}
			}
			else
			{
				printf("stuck / nothing\n");
			}
		}
	}

	else
	{
		clearSeedParticle(i);
	}

	unsigned int result = currentPosition;
	return result;
}


// fade the color in random seedgrid squares if the particle there is not a seed. this makes photon trails fade over time.
void updateSeedgridColor (unsigned int i)
{


	if (seedGrid[i].stage == STAGE_NULL)
	{



// SeedParticle tempSeed = seedGrid[b];
		// seedGrid[b] = seedGrid[a];
		// seedGrid[a] = tempSeed;
		// float temp_color[4];
		unsigned int a_offset = (i * numberOfFieldsPerVertex) ;
		// unsigned int b_offset = (b * numberOfFieldsPerVertex) ;
		// memcpy( temp_color, 				&seedColorGrid[ b_offset ] , 	sizeof(Color) ); // 4x floats of 4 bytes each
		// memcpy( &seedColorGrid[ b_offset], 	&seedColorGrid[ a_offset] , 	sizeof(Color) );
		// memcpy( &seedColorGrid[ a_offset ], temp_color, 					sizeof(Color) );
		// seedGrid[i].e

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





		// // PHOTONS. Some of the particles on the seed grid are particles of light that fall downwards.
		// if (seedGrid[i].stage == STAGE_PHOTON)
		// {
		// 	// unsigned int squareBelow = i - sizeX;

		// 	unsigned int neighbour = neighbourOffsets[sunlightDirection] + i;

		// 	// if they touch a plant...
		// 	if (lifeGrid[neighbour].identity > 0)
		// 	{
		// 		// which has a photosynthetic leaf...
		// 		if (lifeGrid[neighbour].energySource == ENERGYSOURCE_LIGHT)
		// 		{
		// 			if (extremelyFastNumberFromZeroTo(8) == 0)
		// 			{
		// 				// it consumes the photon as energy
		// 				lifeGrid[neighbour].energy += sunlightEnergy;
		// 				clearSeedParticle(i);
		// 				continue;
		// 			}
		// 			else
		// 			{
		// 				// if the photon falls on a seed, it cannot occupy the same place, so the photon is destroyed.
		// 				if (seedGrid[neighbour].stage != 0x00)
		// 				{
		// 					clearSeedParticle(i);
		// 					continue;
		// 				}
		// 				else
		// 				{



		// 					swapPhoton( i, neighbour);
		// 					continue;
		// 				}
		// 			}
		// 		}
		// 	}

		// 	// the photon falls downward if nothing is below it. They can travel into water.
		// 	if (grid[neighbour].phase != PHASE_VACUUM && grid[neighbour].phase != PHASE_GAS && grid[neighbour].phase != PHASE_LIQUID
		// 	   )
		// 	{
		// 		grid[neighbour].temperature += (sunlightEnergy * sunlightHeatCoeff);
		// 		clearSeedParticle(i);
		// 		continue;
		// 	}
		// 	else
		// 	{
		// 		// if it falls on material, it is destroyed.
		// 		if (seedGrid[neighbour].stage != 0x00)
		// 		{
		// 			clearSeedParticle(i);
		// 			continue;
		// 		}
		// 		swapPhoton( i, neighbour);
		// 		continue;
		// 	}
		// 	continue;
		// }



		// SEEDS. Some of the particles on the seed grid are seeds that fall downwards.
		// if (seedGrid[i].parentIdentity > 0 )
		// {
		if (seedGrid[i].stage == STAGE_FRUIT)
		{
			if (extremelyFastNumberFromZeroTo(1) == 0) 		// get blown by the wind only some of the time
			{
				// while (true) 								// this while loop is just here so you can 'break' out of it to end this sequence quickly.
				// {
				unsigned int neighbour = neighbourOffsets[wind] + i;
				// if (wind.x > 0)
				// {
				// 	neighbour = i + 1;
				// 	if ((grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  ) && seedGrid[neighbour].stage == 0x00  )
				// 	{
				// 		swapSeedParticle( i, neighbour );
				// 		continue;
				// 	}
				// }
				// else if (wind.x < 0)
				// {
				// 	neighbour = i - 1;
				if ((grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  ) && seedGrid[neighbour].stage == 0x00  )
				{
					swapSeedParticle( i, neighbour );
					continue;
				}
				// }
				// continue;
				// }

				// while (true) 								// this while loop is just here so you can 'break' out of it to end this sequence quickly.
				// {
				// 	unsigned int neighbour;
				// 	if (wind.y > 0)
				// 	{
				// 		neighbour = i + sizeX;
				// 		if ((grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  ) && seedGrid[neighbour].stage == 0x00  )
				// 		{
				// 			swapSeedParticle( i, neighbour );
				// 			continue;
				// 		}
				// 	}
				// 	else if (wind.y < 0)
				// 	{
				// 		neighbour = i - sizeX;
				// 		if ((grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  ) && seedGrid[neighbour].stage == 0x00  )
				// 		{
				// 			swapSeedParticle( i, neighbour );
				// 			continue;
				// 		}					//
				// 	}
				// 	continue;
				// }
			}

			unsigned int j = extremelyFastNumberFromZeroTo(4);
			unsigned int neighbour;

			if (		j == 0)		{ neighbour = i - sizeX - 1 ;	}
			else if (	j == 1)		{ neighbour = i - sizeX	 	;	}
			else if (	j == 2)		{ neighbour = i - sizeX + 1 ;	}
			else if (	j == 3)		{ neighbour = i + 1 		;	}
			else if (	j == 4)		{ neighbour = i - 1  		;	}

			if (grid[neighbour].phase == PHASE_VACUUM || grid[neighbour].phase == PHASE_GAS  )
			{
				swapSeedParticle( i, neighbour );
				continue;
			}
			else if (grid[neighbour].material == seedGrid[i].germinationMaterial)
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
			unsigned int neighbour;

			if (		j == 0)		{ neighbour = i - sizeX - 1 ;	}
			else if (	j == 1)		{ neighbour = i - sizeX	 	;	}
			else if (	j == 2)		{ neighbour = i - sizeX + 1 ;	}
			else if (	j == 3)		{ neighbour = i + 1	    	;	}
			else if (	j == 4)		{ neighbour = i - 1 		;	}
			else if (	j == 5)		{ neighbour = i + sizeX - 1 ;	}
			else if (	j == 6)		{ neighbour = i + sizeX		;	}
			else if (	j == 7)		{ neighbour = i + sizeX + 1 ;	}

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
		// continue;
		// }



		else if (seedGrid[i].stage == STAGE_ERODINGRAIN)
		{
			unsigned int currentPosition = i;
			bool movedThisTurn = false;
			unsigned int neighbours[] =
			{

				currentPosition - 1,
				currentPosition - sizeX - 1,
				currentPosition - sizeX,
				currentPosition - sizeX + 1,
				currentPosition + 1
			};




			unsigned int offset = extremelyFastNumberFromZeroTo(4);
			unsigned int rainNeighbours = 0;
			for (unsigned int k = 0; k < 5; ++k)
			{
				unsigned int index = (k + offset) % 5;
				if ((grid[neighbours[index]].phase == PHASE_VACUUM) || (grid[neighbours[index]].phase == PHASE_GAS )      )
				{

					if (seedGrid[currentPosition].energy >= 1.0f)
					{

						// setParticle( MATERIAL_QUARTZ, currentPosition);
						// seedGrid[currentPosition].energy -= 1.0f;
						// continue;
					}


					movedThisTurn = true;
					currentPosition = neighbours[index];
					// seedGrid[currentPosition].parentIdentity = 0x00;


					if (seedGrid[currentPosition].energy < 1.0f)
					{


						unsigned int neighbours2[] =
						{
							currentPosition - 1,
							currentPosition - sizeX - 1,
							currentPosition - sizeX,
							currentPosition - sizeX + 1,
							currentPosition + 1
						};

						for (unsigned int h = 0; h < 5; ++h)
						{

							unsigned int index2 = (h + offset) % 5;

							if (neighbours2[index2] > totalSize) {continue;}

							if (grid[neighbours2[index2]].phase == PHASE_SOLID )
							{

								clearParticle(neighbours2[index2]);
								seedGrid[currentPosition].energy += 1.0f;
								break;

							}
						}


					}







					break;
				}
			}

			swapSeedParticle( i, currentPosition);

			continue;
		}


		else if (seedGrid[i].stage == STAGE_ANIMAL)
		{


			if (seedGrid[i].parentIdentity < animals.size())
			{
				Animal * a = &(animals[seedGrid[i].parentIdentity]);

				if (extremelyFastNumberFromZeroTo(64) == 0)
				{

					a->direction = extremelyFastNumberFromZeroTo(7);

				}

				walkAnAnimal(i);

#ifdef DRAW_ANIMALS
				updateAnimalDrawing(i);
#endif

			}
			continue;
		}

		else
		{

			// if (extremelyFastNumberFromZeroTo(100) == 0)
			// {
			// 	updateSeedgridColor(i);
			// }
			continue;

		}

	}

#ifdef THREAD_TIMING_READOUT
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
		animationGrid[ (i * numberOfFieldsPerVertex) + 0 ] = lifeColorGrid[(i * numberOfFieldsPerVertex) + 0];
		animationGrid[ (i * numberOfFieldsPerVertex) + 1 ] = lifeColorGrid[(i * numberOfFieldsPerVertex) + 1];
		animationGrid[ (i * numberOfFieldsPerVertex) + 2 ] = lifeColorGrid[(i * numberOfFieldsPerVertex) + 2];
		animationGrid[ (i * numberOfFieldsPerVertex) + 3 ] = lifeColorGrid[(i * numberOfFieldsPerVertex) + 3] = 0.5f;

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
			std::string exampleSentence = std::string("");

			unsigned int randomPlantIndex = 5;//extremelyFastNumberFromZeroTo(7);

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



			// cursor_germinationMaterial = randomGerminationMaterial;
			setSeedParticle(exampleSentence, newIdentity() , 0, i, 0);
			seedGrid[i].stage = STAGE_FRUIT;


			unsigned int randomGerminationMaterial = 0;
			if (materials.size() > 0)
			{
				randomGerminationMaterial = extremelyFastNumberFromZeroTo(materials.size() - 1);

				std::string materialGene = std::string("m");
				materialGene.push_back( numeralphabetic(randomGerminationMaterial) );

				seedGrid[i].genes.insert(0, materialGene);
				// seedGrid[i].genes += materialGene;

				printf("%s\n", seedGrid[i].genes.c_str());
			}



			// seedGrid[i].germinationMaterial = randomGerminationMaterial;
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

			setAnimal( i );


		}

	}


}


void increaseLampBrightness ()
{
	// printf("increaseLampBrightness\n");
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

	std::ofstream out4(std::string("save/animationGrid").c_str());
	out4.write(reinterpret_cast<char*>(& (animationGrid[0]) ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	out4.close();

	std::ofstream out41(std::string("save/seedColorGrid").c_str());
	out41.write(reinterpret_cast<char*>(& (seedColorGrid[0]) ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	out41.close();

	printf("- saved color grids\n");


	// you need to save the materials and animals vectors as well somehow, even though their size is unknown.

	std::ofstream out879("save/materials");
	// for (unsigned int i = 0; i < materials.size(); ++i)
	// {
	// 	out879 << materials[i]  << '\n';
	// }

	// for (const auto &p : materials)
	// {
	// 	out879 << p << '\n';
	// }

	const char* pointer = reinterpret_cast<const char*>(&materials[0]);
	size_t bytes = materials.size() * sizeof(Material);
	out879.write(pointer, bytes);

	out879.close();

	std::ofstream out8579("save/animals");
	// // for (unsigned int i = 0; i < animals.size(); ++i)
	// // {
	// // 	out8579 << animals[i]  << '\n';
	// // }

	// for (const auto &p : animals)
	// {
	// 	out8579 << p << '\n';
	// }


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

		// materials.clear();

		// materials.push_back(bumdirt);

		resetMaterials();

		for (int i = 0; i < m; ++i)
		{

			printf("- material \n" );

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

		// animals.push_back(bumdirt);

		for (int i = 0; i < m; ++i)
		{

			// printf("- animal \n" );

			animals.push_back(Animal());
		}


		std::ifstream in556(std::string("save/animals").c_str());
		in556.read( (char *)(&(animals[0])), sizeof(Animal) *  m);
		in556.close();
	}


	// std::ifstream in6(std::string("save/grid").c_str());
	// in6.read( (char *)(&(grid[0])), sizeof(Particle) *  totalSize);
	// in6.close();


	printf("loaded animals\n");

}



void load_colorgrids()
{
// std::ifstream in1(std::string("save/lifeGrid").c_str());
	// in1.read(  (char *)(lifeGrid) , sizeof(LifeParticle) * totalSize);
	// in1.close();


	std::ifstream in3(std::string("save/colorGrid").c_str());
	in3.read( (char *)( &(colorGrid[0])  ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	in3.close();

	std::ifstream in33(std::string("save/lifeColorGrid").c_str());
	in33.read( (char *)( &(lifeColorGrid[0])  ), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	in33.close();

	std::ifstream in4(std::string("save/animationGrid").c_str());
	in4.read( (char *)( &(animationGrid[0])), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	in4.close();

	std::ifstream in41(std::string("save/seedColorGrid").c_str());
	in41.read( (char *)( &(seedColorGrid[0])), sizeof(float) * numberOfFieldsPerVertex *  totalSize);
	in41.close();

	printf("- loaded color grids\n");
}



void load_life()
{



	transportableLifeParticle* transportableLifeGrid = new transportableLifeParticle[totalSize];
	// std::memset(transportableLifeGrid, 0x00, sizeof(transportableLifeParticle) * totalSize);



	std::ifstream in22(std::string("save/transportableLifeGrid").c_str());
	in22.read( (char*)( &(transportableLifeGrid[0])), sizeof(transportableLifeParticle) * totalSize);
	in22.close();



	for (unsigned int i = 0; i < totalSize; ++i)
	{

		lifeGrid[i] = LifeParticle();

		// 	printf("veguntus %u\n", i);
		// printf("- ennelmennel %u, %u\n", lifeGrid[i].energySource, i);
		// printf("- bee %u\n", transportableLifeGrid[i].energySource);


		lifeGrid[i].energySource = transportableLifeGrid[i].energySource;
		lifeGrid[i].energy = transportableLifeGrid[i].energy;
		lifeGrid[i].identity = transportableLifeGrid[i].identity;
	}

	delete [] transportableLifeGrid;

	printf("- loaded transportable life grid\n");






	// 	// read lengths of both files
	// if (true)
	// {
	// 	std::ifstream in5(std::string("save/lifeGeneGrid").c_str());
	// 	std::string line = std::string("");
	// 	unsigned int i = 0;
	// 	for (std::string line; std::getline(in5, line, '\n'); )
	// 	{
	// 		i++;
	// 	}

	// 	printf("lifeGeneGrid lines: %u\n", i);

	// 	std::ifstream in500(std::string("save/seedGeneGrid").c_str());
	// 	i = 0;
	// 	for (std::string line; std::getline(in500, line, '\n'); )
	// 	{
	// 		i++;
	// 	}
	// 	printf("seedGeneGrid lines: %u\n", i);
	// }

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
	// std::memset(transportableSeedGrid, 0x00, sizeof(transportableSeed) * totalSize);

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

	// // read lengths of both files
	// if (true)
	// {
	// 	std::ifstream in5(std::string("save/lifeGeneGrid").c_str());
	// 	std::string line = std::string("");
	// 	unsigned int i = 0;
	// 	for (std::string line; std::getline(in5, line, '\n'); )
	// 	{
	// 		i++;
	// 	}

	// 	printf("lifeGeneGrid lines: %u\n", i);

	// 	std::ifstream in500(std::string("save/seedGeneGrid").c_str());
	// 	i = 0;
	// 	for (std::string line; std::getline(in500, line, '\n'); )
	// 	{
	// 		i++;
	// 	}
	// 	printf("seedGeneGrid lines: %u\n", i);
	// }

	// // load life genes
	// if (true)
	// {
	// 	std::ifstream in5(std::string("save/lifeGeneGrid").c_str());
	// 	std::string line = std::string("");
	// 	unsigned int i = 0;
	// 	for (std::string line; std::getline(in5, line, '\n'); )
	// 	{
	// 		lifeGrid[i].genes.clear();
	// 		if (i < totalSize)
	// 		{
	// 			lifeGrid[i].genes.assign(line);
	// 		}
	// 		in5.clear();
	// 		i++;
	// 	}
	// 	printf("- loaded life genomes\n");
	// }

	// // load seed genes
	// if (true)
	// {
	// 	std::ifstream in500(std::string("save/seedGeneGrid").c_str());
	// 	std::string line = std::string("");
	// 	unsigned int i = 0;
	// 	for (std::string line; std::getline(in500, line, '\n'); )
	// 	{
	// 		if (i < totalSize)
	// 		{
	// 			seedGrid[i].genes.assign(line);
	// 		}
	// 		in500.clear();
	// 		i++;
	// 	}
	// 	printf("- loaded seed genomes\n");
	// }




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