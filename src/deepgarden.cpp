#include "deepgarden.h"
#include "deepgarden_graphics.h"


#include <ctime>
#include <chrono>
#include <iostream>

#define THREAD_TIMING
#define RENDERING_THREADS 4

const unsigned int totalSize = sizeX * sizeY;
// const unsigned int highestI


const unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */
float colorGrid[totalSize * numberOfFieldsPerVertex ];


unsigned int material_grid[totalSize];
unsigned int phase_grid[totalSize];
unsigned int identity_grid[totalSize];
unsigned int light_grid[totalSize];
bool diffs_grid[totalSize];

const color lightColor = color(1.0f, 1.0f, 1.0f, 1.0f);
const color emptyColor = color(0.05f, 0.05f, 0.05f, 1.0f);
const color lifeColor =  color(0.2f, 0.05f, 0.4f, 1.0f);
const color stoneColor = color(0.5f, 0.5f, 0.5f, 1.0f);
const color sandColor =  color(0.3f, 0.3f, 0.3f, 1.0f);
const color goldColor =  color(1.0f, 1.0f, 0.0f, 1.0f);

std::list<unsigned int> identities;

unsigned int recursion_level = 0;
const unsigned int recursion_limit = 5;

// std::string exampleSentence = " Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque viverra nulla eget fermentum commodo. Nullam ac ex at nisl finibus ultrices. Mauris aliquet elementum turpis, sit amet lobortis magna viverra sed. Proin dignissim hendrerit est, ut convallis dui suscipit sed. Nulla libero justo, euismod malesuada tempus nec, fringilla vel est. Pellentesque sed tellus a purus porttitor vulputate et at nunc. Donec non ipsum scelerisque, placerat sapien sit amet, fringilla est. Fusce pretium urna sit amet hendrerit ultrices. Morbi eget eleifend mi, non feugiat mauris. Praesent non condimentum ligula. Mauris consequat mi ac magna placerat pellentesque. Vestibulum non interdum enim, ac vehicula diam. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Aenean porta libero quis libero rutrum imperdiet. Curabitur vulputate feugiat odio non suscipit. Interdum et malesuada fames ac ante ipsum primis in faucibus. Vivamus erat magna, pulvinar sed felis non volutpat. ";
// std::string exampleSentence = "we'cre off toc see the cwizard, cthe wonderful wizcard of oz! cbecause beccause becaucse becausce because, acll of the wocnderful things che does. do do cdod do do docd doo! ";
std::string exampleSentence = "selnslslul lblotsancol lest sstructra plantae csui insnltl lsemina. Pars asutem fungslil quae ssporlls gignit corpuls fructsferum appellatur.";



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



struct lifeform
{
	unsigned int identity;
	unsigned int parent_id;

	int energy;
	std::string genes;

	unsigned int cursor_string;
	unsigned int geneSize;
	vec_u2 cursor_grid = vec_u2(0, 0);
	vec_u2 prevCursor_grid = vec_u2(0, 0);
	float accumulatedRotation ;
	float accumulatedRotation_precomputedSin;
	float accumulatedRotation_precomputedCos;

	vec_u2 origin = vec_u2(0, 0);

	bool germinated;

	lifeform(std::string a)
	{
		this->energy = 0;
		this->genes = a;

		this->cursor_string = 0;
		// this->cursor_grid 		= 0;
		// this->prevCursor_grid 	= 0;
		this->accumulatedRotation  = 0;
		this->accumulatedRotation_precomputedSin = sin(this->accumulatedRotation);
		this->accumulatedRotation_precomputedCos = cos(this->accumulatedRotation);

		this->germinated = false;

		this->identity = newIdentity();
	}



};




std::list<lifeform> creatures;

// variables keep track of the sequence and rotation states.
float accumulatedRotation = 0.0f;
float accumulatedRotation_precomputedSin = sin(accumulatedRotation);
float accumulatedRotation_precomputedCos = cos(accumulatedRotation);

unsigned int currentlyPaintingMaterial = MATERIAL_STONE;

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
	if (abs(shortLen) > abs(longLen)) {
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
				// myPixel(surface,j >> 8,y);
				v.push_back( vec_u2(j >> 8, y) );
				j += decInc;
			}
			return v;
		}
		longLen += y;
		for (int j = 0x80 + (x << 8); y >= longLen; --y) {
			// myPixel(surface,j >> 8,y);
			v.push_back( vec_u2(j >> 8, y) );
			j -= decInc;
		}
		return v;
	}

	if (longLen > 0) {
		longLen += x;
		for (int j = 0x80 + (y << 8); x <= longLen; ++x) {
			// myPixel(surface,x,j >> 8);
			v.push_back( vec_u2(x, j >> 8) );
			j += decInc;
		}
		return v;
	}
	longLen += x;
	for (int j = 0x80 + (y << 8); x >= longLen; --x) {
		// myPixel(surface,x,j >> 8);
		v.push_back( vec_u2(x, j >> 8) );
		j -= decInc;
	}

	return v;
}

std::string randomSentence ()
{
	unsigned int randomSentenceLength = 256;

	char bytes[randomSentenceLength];

	for (unsigned int  i = 0; i < randomSentenceLength; ++i)
	{
		bytes[i] = 'a' + rand() % (26 + 1);
		if (RNG() < 0.1)
		{
			bytes[i] = ' ';

		}
	}

	return std::string(bytes);
}


std::string mutateSentence ( std::string  mutableString)
{

	std::string output = std::string("");

	unsigned int n = mutableString.length();

	for (unsigned int i = 0; i < n; ++i)
	{

		char c = mutableString.c_str()[i];

		if (RNG() < 0.01)
		{

			if (RNG() < 0.5)
			{

				// printf("memem\n");
				c = c + 1;

			}
			else {


				c = c - 1;

				// output = output + std::string(c);

			}

		}


		output = output + c;
	}

	return output;

}

void instantiateCreature(std::string sentence, unsigned int x, unsigned int y)
{
	printf("%s\n", sentence.c_str());

	lifeform creature = lifeform(sentence);

	unsigned int i = (y * sizeX) + x;


	creature.energy = -1;
	creature.germinated = false;

	// place a seed at the coordinates
	material_grid[i] = MATERIAL_GOLD;
	phase_grid   [i] = PHASE_LIQUID;
	identity_grid[i] = creature.identity;

	creatures.push_back(
	    creature
	);

	creatures.back().cursor_grid = vec_u2(x, y);
}


int drawCharacter (lifeform * creature)
{

	if (recursion_level > recursion_limit)
	{
		return -1;
	}

	char c = creature->genes[creature->cursor_string];

	std::list<vec_u2> v;

	switch (c)
	{

	case 'e': // extrude
	{
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}

	}

	case 'b': // branch. a sequence that grows at an angle to the main trunk
	{
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}



		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( creature->genes[creature->cursor_string] ); creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;} }

		float rotation = numberModifier;

		// record trunk rotation
		float trunkRotation = accumulatedRotation;

		// figure out new branch rotation
		creature->accumulatedRotation = accumulatedRotation + (rotation / 26.0f) * 2.0f * 3.14159f;
		creature->accumulatedRotation_precomputedSin = sin(accumulatedRotation);
		creature->accumulatedRotation_precomputedCos = cos(accumulatedRotation);

		// record old cursor position
		vec_u2 old_cursorGrid = creature->cursor_grid;

		recursion_level++;
		while (1)
		{
			if ( drawCharacter(creature) < 0)
			{
				break;
			}
		}
		recursion_level--;

		// return to normal rotation
		creature->accumulatedRotation = trunkRotation;
		creature->accumulatedRotation_precomputedSin = sin(accumulatedRotation);
		creature->accumulatedRotation_precomputedCos = cos(accumulatedRotation);

		// return to normal position
		creature->cursor_grid = old_cursorGrid;

		break;
	}

	case 's': // sequence. a motif is repeated serially a number of times.
	{
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}

		// get number of times to repeat the sequence
		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( creature->genes[creature->cursor_string] ); creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;} }

		int repeats = numberModifier;

		// the character after that is the next thing to be arrayed
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}

		unsigned int sequenceOrigin = creature->cursor_string;

		recursion_level++;
		for ( int i = 0; i < repeats; ++i)
		{
			while (1)
			{
				if ( drawCharacter(creature) < 0)
				{
					break;
				}
			}
			creature->cursor_string = sequenceOrigin;
		}
		break;
		recursion_level--;
	}
	case ' ': // break innermost array
	{
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}

		return -1;
		break;
	}

	case '.': // break array and return cursor to origin
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}

		creature->cursor_grid = creature->origin;

		return -1;
		break;

	case 'g': // make a seed
	{

		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}



// unsigned int currentlyPaintingMaterial = MATERIAL_STONE;


		instantiateCreature( mutateSentence ( creature->genes) , creature->cursor_grid.x, creature->cursor_grid.y);

		break;

	}

	case 'c': // paint a circle at the cursor
	{
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}

		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( creature->genes[creature->cursor_string] ); creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;} }

		// get the circle radius (the next number in the string.)
		unsigned int radius = numberModifier;

		// define the range of pixels you'll draw- so you don't have to navigate the entire, massive grid.
		unsigned int drawingAreaLowerX = creature->cursor_grid.x - radius;
		unsigned int drawingAreaLowerY = creature->cursor_grid.y - radius;
		unsigned int drawingAreaUpperX = creature->cursor_grid.x + radius;
		unsigned int drawingAreaUpperY = creature->cursor_grid.y + radius;

		// raster a circle
		for (unsigned int i = drawingAreaLowerX; i < drawingAreaUpperX; ++i)
		{
			for (unsigned int j = drawingAreaLowerY; j < drawingAreaUpperY; ++j)
			{
				if (  magnitude_int (  i - creature->cursor_grid.x , j - creature->cursor_grid.y )  < radius )
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
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}

		// set the previous x and y, which will be the start of the line
		creature->prevCursor_grid = creature->cursor_grid;

		bool chosenSignX = false;
		int signX = 1;

		while (!chosenSignX)
		{
			creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}
			signX = alphanumeric( creature->genes[creature->cursor_string]);
			if (signX > 13) { signX = 1; } else { signX = -1; }
			chosenSignX = true;
		}

		bool chosenSignY = false;
		int signY = 1;

		while (!chosenSignY)
		{
			creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}
			signY = alphanumeric( creature->genes[creature->cursor_string]);
			if (signY > 13) { signY = 1; }
			else {
				signY = 1; // signY = -1; // i made it this way so the plants always grow up. instead of into the ground.
			}
			chosenSignY = true;
		}

		unsigned int numberModifier = 0;

		while (!numberModifier)
		{
			creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}
			numberModifier = alphanumeric( creature->genes[creature->cursor_string] );
		}

		bool inRange = true;

		if (creature->cursor_grid.x + numberModifier  > sizeX && signX > 0)
		{
			creature->cursor_grid.x = sizeX - 1;
			inRange = false;
		}
		if (numberModifier > creature->cursor_grid.x && signX < 0 )
		{
			creature->cursor_grid.x = 0;
			inRange = false;
		}
		if (inRange)
		{
			if (signX > 0)
			{
				creature->cursor_grid.x += (numberModifier );
			}
			else
			{
				creature->cursor_grid.x -= (numberModifier );
			}
		}
		numberModifier = 0;

		while (!numberModifier)
		{
			creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}
			numberModifier = alphanumeric( creature->genes[creature->cursor_string] );
		}

		inRange = true;
		if (creature->cursor_grid.y + numberModifier  > sizeY && signY > 0)
		{
			creature->cursor_grid.y = sizeY - 1;
			inRange = false;
		}
		if (numberModifier > creature->cursor_grid.y && signY < 0 )
		{
			creature->cursor_grid.y = 0;
			inRange = false;
		}

		if (inRange)
		{
			if (signY > 0)
			{
				creature->cursor_grid.y += (numberModifier );
			}
			else
			{
				creature->cursor_grid.y -= (numberModifier );
			}
		}

		vec_f2 rotatedPoint = rotatePointPrecomputed( vec_f2(creature->prevCursor_grid.x, creature->prevCursor_grid.y),  accumulatedRotation_precomputedSin,  accumulatedRotation_precomputedCos, vec_f2(  creature->cursor_grid.x, creature->cursor_grid.y));

		if 		(rotatedPoint.x > 0 && rotatedPoint.x < sizeX - 1) 	{ creature->cursor_grid.x = rotatedPoint.x;}
		else if (rotatedPoint.x < 0) 								{ creature->cursor_grid.x = 0;}
		else if (rotatedPoint.x > sizeX - 1) 						{ creature->cursor_grid.x = sizeX - 1; }

		if 		(rotatedPoint.y > 0 && rotatedPoint.y < sizeY - 1) 	{ creature->cursor_grid.y = rotatedPoint.y;}
		else if (rotatedPoint.y < 0) 								{ creature->cursor_grid.y = 0;}
		else if (rotatedPoint.y > sizeY - 1) 						{ creature->cursor_grid.y = sizeY - 1; }

		v = EFLA_E( creature->prevCursor_grid, creature->cursor_grid);

		break;
	}

	default:
	{
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}
	}
	}

	// printf("committing pixels to fofefe\n");

	// paint in the points with material
	unsigned int n_points = 0;
	for (std::list<vec_u2>::iterator it = v.begin(); it != v.end(); ++it)
	{

		unsigned int i = (it->y * sizeX) + it->x;

		phase_grid[i] 	= PHASE_SOLID;
		material_grid[i] = currentlyPaintingMaterial;
		identity_grid[i] = creature->identity;
		n_points ++;
	}

	creature->energy -= n_points;

	return 0;
}

int drawNextSequence (lifeform * creature)
{

	int n_points = 0;

	if (creature->genes.length() <= 0)
	{
		return n_points;
	}

	creature->geneSize = creature->genes.length() - 1;

	if (creature->geneSize == 1)
	{
		return n_points;
	}

	while (1)
	{

		int drawingPoints = drawCharacter(creature) ;

		if (drawingPoints == -1)
		{
			break;
		}
		else
		{
			n_points += drawingPoints;
		}
	}

	return n_points;
}



void initialize ()
{

	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	srand((unsigned int)time(NULL));

	memset( phase_grid, PHASE_VACUUM, (sizeof(unsigned int) * totalSize) );
	memset( material_grid, MATERIAL_VACUUM, (sizeof(unsigned int) * totalSize) );
	memset( identity_grid, 0x00, (sizeof(unsigned int) * totalSize) );
	memset( light_grid, 0x00, (sizeof(unsigned int) * totalSize) );
	memset( diffs_grid, true, (sizeof(bool) * totalSize) );
	memset ( colorGrid, 0x00, sizeof(float ) * numberOfFieldsPerVertex * totalSize );


	// setup the x and y positions in the color grid. these never change so you can just calculate them once.
	unsigned int x = 0;
	unsigned int y = 0;
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		x = i % sizeX;
		if (!x) { y = i / sizeX; }

		float fx = x;
		float fy = y;



		// RGBA color occupies the first 4 places.
		// also, initialize the color alpha to 1.
		colorGrid[ (i * numberOfFieldsPerVertex) + 3] = 1.0f;
		colorGrid[ (i * numberOfFieldsPerVertex) + 4] = fx;
		colorGrid[ (i * numberOfFieldsPerVertex) + 5] = fy;


		// sprinkle some material on it to make a default scene.
		if (i > (10 * sizeX) && i < (50 * sizeX))
		{
			if (RNG() < 0.5)
			{
				material_grid[i] = MATERIAL_IRON;
				phase_grid[i] = PHASE_POWDER;
				memcpy(&colorGrid[i * numberOfFieldsPerVertex], &sandColor, 16  );
			}
			else {
				material_grid[i] = MATERIAL_OXYGEN;
				phase_grid[i] = PHASE_GAS;
				memcpy(&colorGrid[i * numberOfFieldsPerVertex], &lifeColor, 16  );
			}
		}

	}







	//instantiateCreature(randomSentence(), 100, 100);
}


void swap (unsigned int a, unsigned int b)
{
	unsigned int temp_phase = phase_grid[b];
	phase_grid[b] = phase_grid[a];
	phase_grid[a] = temp_phase;

	unsigned int temp_id = identity_grid[b];
	identity_grid[b] = identity_grid[a];
	identity_grid[a] = temp_id;

	unsigned int temp_mat = material_grid[b];
	material_grid[b] = material_grid[a];
	material_grid[a] = temp_mat;

	float temp_color[4];
	unsigned int a_offset = (a * numberOfFieldsPerVertex);
	unsigned int b_offset = (b * numberOfFieldsPerVertex);
	memcpy( temp_color, &colorGrid[ b_offset ] , 16 ); // 4x floats of 4 bytes each
	memcpy( &colorGrid[ b_offset], &colorGrid[ a_offset] , 16 );
	memcpy( &colorGrid[ a_offset ], temp_color, 16 );

	// diffs_grid[a] = true;
	// diffs_grid[b] = true;

}

lifeform * getCreatureByID( unsigned int id )
{
	for (std::list<lifeform>::iterator creature = creatures.begin(); creature != creatures.end(); ++creature)
	{
		if (creature->identity == id)
		{
			return &(*creature);
		}
	}
	return nullptr;
}



bool chemistry(unsigned int a, unsigned int b)
{
	// perform chemistry
	if (identity_grid[a] )
	{
		if (material_grid[b] != MATERIAL_VACUUM)
		{
			lifeform * creature = getCreatureByID(identity_grid[a]);
			if (creature != nullptr)
			{
				if (identity_grid[b] != creature->parent_id)
				{
					unsigned int x = a % sizeX;
					unsigned int y = a / sizeX;
					creature->cursor_grid = vec_u2(x, y);
					creature->origin = vec_u2(x, y);
					creature->germinated = true;
					creature->energy = 0;
				}
			}
		}
	}


	switch (material_grid[a])
	{
	case MATERIAL_OXYGEN:
	{

		switch (material_grid[b])
		{
		case MATERIAL_IRON:
			material_grid[b] = MATERIAL_STONE;
			material_grid[a] = MATERIAL_VACUUM;
			phase_grid[a] = PHASE_VACUUM;
			identity_grid[a] = 0x00;
			return true;
		}



		break;
	}



	}

	return false;

}





void thread_particledrawing ()
{
	// iterate creatures and grow them if appropriate
	for (std::list<lifeform>::iterator creature = creatures.begin(); creature != creatures.end(); ++creature)
	{
		// printf("creeeaea | %i\n", creature->energy);
		if (creature->energy >= 0)
		{
			creature->energy -= drawNextSequence ( &(*creature) );
		}
	}
}

void thread_optics ()
{

#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	// unsigned int lightLimit = (sizeY - 1 ) * sizeX;
	unsigned int lightRow = totalSize - sizeX;  // by having the light start from the second-to-top row, we can avoid checking to see if the loop index is out of bounds later on, which adds up to a huge saving.


	for (int i = lightRow; i < totalSize; ++i)
	{
		if (RNG() < 0.005)
		{
			light_grid[i] = LIGHT;
		}
	}

	// propagate light downwards.

	for (unsigned int i = sizeX; i < totalSize; ++i)
	{
		// if (squareBelow < totalSize)
		// {




		if (light_grid[i])
		{
			if ((phase_grid[i] & (PHASE_SOLID | PHASE_POWDER)) == (PHASE_SOLID | PHASE_POWDER) )
			{

				if (identity_grid[i] )
				{
					lifeform * creature = getCreatureByID(identity_grid[i]);
					if (creature != nullptr)
					{
						creature->energy += 100;
					}
				}
				light_grid[i] = 0x00;
				continue;


			}



		}


		unsigned int squareBelow = i - sizeX;
		unsigned int temp_light = light_grid[squareBelow];
		light_grid[squareBelow] = light_grid[i];
		light_grid[i] = temp_light;


		// }
	}

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_optics " << elapsed.count() << " microseconds." << std::endl;
#endif
}

// return a random integer in the range. It is inclusive of both end values.
unsigned int randomIntegerInRange (unsigned int from, unsigned int to)
{
	return from + ( std::rand() % ( to - from + 1 ) );
}

void thread_physics ()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif


	for (unsigned int i = 0; i < totalSize; ++i)
	{

		if (true)
		{

			unsigned int squareBelow = i - sizeX;
			unsigned int squareAbove = i + sizeX;

			unsigned int neighbours[] =
			{
				i - 1,
				i + 1,
				squareBelow + 1,
				squareBelow,
				squareBelow - 1,
				squareAbove + 1,
				squareAbove,
				squareAbove - 1

			};


			// bool did_anything = false;

			if ((phase_grid[i] & (PHASE_POWDER)) == (PHASE_POWDER))
			{
				// unsigned int neighbours[] =
				// {
				// 	squareBelow + 1,
				// 	squareBelow,
				// 	squareBelow - 1
				// };

				for (unsigned int k = 2; k < 5; ++k) // instead of checking all of them every turn, you can check half or less and it doesn't make a difference for gameplay.
				{
					// unsigned int k = rand() % (3 + 2); // it's a powder, only check 2th to 4th place in the neighbours array.

					// unsigned int k = randomIntegerInRange(2, 4);
					if (neighbours[k] > totalSize) {continue;}
					// if (
					chemistry(i, neighbours[k]);
					// ) {did_anything = true;}

					if ((phase_grid[neighbours[k]] & (PHASE_VACUUM)) == (PHASE_VACUUM))
					{
						swap(i, neighbours[k]);
						// did_anything = true;
					}


					// if (did_anything) {diffs_grid[neighbours[k]] =true; }

					// chemistry(i, neighbours[k]) ;
					break;
				}
			}

			// movement instructions for LIQUIDS
			else if ((phase_grid[i] & (PHASE_LIQUID)) == (PHASE_LIQUID))
			{
				// unsigned int neighbours[] =
				// {
				// 	i - 1,
				// 	i + 1,
				// 	squareBelow + 1,
				// 	squareBelow,
				// 	squareBelow - 1
				// };

				for (unsigned int k = 0; k < 5; ++k)
				{
					// unsigned int k = rand() % (4 + 1);
					// unsigned int k = randomIntegerInRange(0, 4);
					if (neighbours[k] > totalSize) {continue;}

					// if (
					chemistry(i, neighbours[k]) ;
					// ) {did_anything = true;}
					if ((phase_grid[neighbours[k]] & (PHASE_VACUUM)) == (PHASE_VACUUM))
					{
						swap(i, neighbours[k]);
						// chemistry(i, neighbours[k]) ;
						// did_anything = true;
						// diffs_grid[neighbours[k]]=true;
						// diffs_grid[i]=true;
					}


					// if (did_anything) {diffs_grid[neighbours[k]] =true; }

					// chemistry(i, neighbours[k]) ;
					break;

				}
			}

			// movement instructions for GASES
			else if ((phase_grid[i] & (PHASE_GAS)) == (PHASE_GAS))
			{
				// unsigned int squareAbove = i + sizeX;
				// unsigned int neighbours[] =
				// {
				// 	i - 1,
				// 	i + 1,
				// 	squareBelow + 1,
				// 	squareBelow,
				// 	squareBelow - 1
				// 	squareAbove + 1,
				// 	squareAbove,
				// 	squareAbove - 1,

				// };

				for (unsigned int k = 0; k < 8; ++k)
				{
					// unsigned int k = rand() % (7 + 1);
					// unsigned int k = randomIntegerInRange(0, 7);
					if (neighbours[k] > totalSize) {continue;}

					// if (
					chemistry(i, neighbours[k]);
					// ) {did_anything = true;}
					if ((phase_grid[neighbours[k]] & (PHASE_VACUUM)) == (PHASE_VACUUM))
					{
						swap(i, neighbours[k]);
						// chemistry(i, neighbours[k]) ;
						// did_anything = true;
					}

					// if (did_anything) {diffs_grid[neighbours[k]] =true; }

					// chemistry(i, neighbours[k]) ;
					break;
				}
			}

			// if (!did_anything)
			// {
			// 	diffs_grid[i] = false;
			// }

		}
	}

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_physics " << elapsed.count() << " microseconds." << std::endl;
#endif
}



// void thread_chemistry ()
// {

// 	for (unsigned int i = 0; i < totalSize; ++i)
// 	{

// 		if (diffs_grid[i])
// 		{
// 			unsigned int squareBelow = i - sizeX;
// 			unsigned int squareAbove = i + sizeX;

// 			// in the particle motion, i believe it is necessary to randomise the order neighbours are interacted with, because it does not look natural otherwise.
// 			// here for the sake of speed i am not using randomness, i hope it will not lead to bad effects.
// 			// instead, the neighbours are presented in order of their proximity to the cell.
// 			unsigned int neighbours[] =
// 			{
// 				squareBelow,
// 				i - 1,
// 				i + 1,
// 				squareBelow + 1,
// 				squareBelow - 1,
// 				squareAbove + 1,
// 				squareAbove,
// 				squareAbove - 1,
// 			};

// 			for (unsigned int k = 0; k < 8; ++k)
// 			{
// 				if (neighbours[k] > totalSize) {continue;}
// 				chemistry(i, neighbours[k]) ;
// 				break;
// 			}
// 		}
// 	}
// }



void chooseColor ( color * colorToUse, unsigned int index )
{


	if ((light_grid[index] & (LIGHT)) == (LIGHT))    {    *colorToUse = lightColor; return; }


	if ((material_grid[index] & (MATERIAL_OXYGEN))   == (MATERIAL_OXYGEN))     {    *colorToUse = lifeColor;  return; }
	if ((material_grid[index] & (MATERIAL_IRON))     == (MATERIAL_IRON))       {    *colorToUse = sandColor;  return; }
	if ((material_grid[index] & (MATERIAL_STONE))    == (MATERIAL_STONE))      {    *colorToUse = stoneColor; return; }
	if ((material_grid[index] & (MATERIAL_GOLD))     == (MATERIAL_GOLD))       {    *colorToUse = goldColor;  return; }


}


void draw_sector (unsigned int from, unsigned int to, float * vertex_buffer_data)
{
	unsigned int x = from % sizeX;
	unsigned int y = from / sizeX;

	unsigned int g_vertex_buffer_cursor = from;

	for (unsigned int i = from; i < to; ++i)
	{
		// if (diffs_grid[i])
		// {
		x = i % sizeX;
		if (!x) { y = i / sizeX; }

		color colorToUse = emptyColor;

		chooseColor ( &colorToUse, i ) ;

		vertToBuffer ( vertex_buffer_data, &g_vertex_buffer_cursor, colorToUse, x,  y);

		// }
	}
}

void thread_graphics()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif
	preDraw();


	unsigned int nVertsToRenderThisTurn = 1 * totalSize;
	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

	// float * vertex_buffer_data  = new float[totalNumberOfFields];



	// boost::thread t7{ draw_sector, 0 									, totalSize, vertex_buffer_data};
	// boost::thread t8{ draw_sector, ( nVertsToRenderThisTurn * 0.25 ) 	, ( nVertsToRenderThisTurn * 0.5  )  , vertex_buffer_data};
	// // boost::thread t9{ draw_sector, ( nVertsToRenderThisTurn * 0.5  ) 	, ( nVertsToRenderThisTurn * 0.75 )  , vertex_buffer_data};
	// boost::thread t10{ draw_sector, ( nVertsToRenderThisTurn * 0.75 ) , ( nVertsToRenderThisTurn  )        , vertex_buffer_data};


	// t7.join();
	// t8.join();
	// t9.join();
	// t10.join();


	glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, colorGrid, GL_DYNAMIC_DRAW );
	glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

	// delete [] vertex_buffer_data;

	postDraw();

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_graphics " << elapsed.count() << " microseconds." << std::endl;
#endif
}


