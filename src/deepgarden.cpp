#include "deepgarden.h"
#include "deepgarden_graphics.h"

static unsigned int material_grid[sizeX][sizeY];
static unsigned int identity_grid[sizeX][sizeY];
static float color_grid_r[sizeX][sizeY];
static float color_grid_g[sizeX][sizeY];
static float color_grid_b[sizeX][sizeY];

color lightColor = color(1.0f, 1.0f, 1.0f, 1.0f);
color emptyColor = color(0.05f, 0.05f, 0.05f, 1.0f);
color lifeColor = color(0.2f, 0.05f, 0.4f, 1.0f);
color stoneColor = color(0.5f, 0.5f, 0.5f, 1.0f);
color goldColor = color(1.0f, 1.0f, 0.0f, 1.0f);

unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

// unsigned int creature->geneSize = 0;
// unsigned int cursor_string = 0;
// unsigned int prevCursor_string = 0;

// bool total_break = false; 			// used to return to recursion depth 0
unsigned int recursion_depth = 0;	// how many breakable sequences are applied right now. like the number indents to the left of this line of code

std::list<unsigned int> identities;

unsigned int newIdentity ()
{
	unsigned int identityCursor = 0;
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

struct vec_u2
{
	unsigned int x;
	unsigned int y;

	vec_u2(unsigned int a, unsigned int b)
	{
		this->x = a;
		this->y = b;
	}
};

struct vec_i2
{
	int x;
	int y;

	vec_i2( int a,  int b)
	{
		this->x = a;
		this->y = b;
	}
};

struct vec_f2
{
	float x;
	float y;

	vec_f2( float a,  float b)
	{
		this->x = a;
		this->y = b;
	}
};

struct lifeform
{
	unsigned int identity;
	int energy;
	std::string genes;

	unsigned int cursor_string;
	unsigned int geneSize;
	vec_u2 cursor_grid;
	vec_u2 prevCursor_grid;
	float accumulatedRotation ;
	float accumulatedRotation_precomputedSin;
	float accumulatedRotation_precomputedCos;

	bool germinated;

	lifeform(std::string a)
	{
		this->energy = 0;
		this->genes = a;

		this->cursor_string = 0;
		this->cursor_grid 		= vec_u2(0, 0);
		this->prevCursor_grid 	= vec_u2(0, 0);
		this->accumulatedRotation  = 0;
		this->accumulatedRotation_precomputedSin = sin(this->accumulatedRotation);
		this->accumulatedRotation_precomputedCos = cos(this->accumulatedRotation);

		this->germinated = false;

		this->identity = newIdentity();
	}


	
};




std::list<lifeform> creatures;

// vec_u2 cursor_grid 		= vec_u2(0, 0);
// vec_u2 prevCursor_grid 	= vec_u2(0, 0);

// variables keep track of the sequence and rotation states.
// unsigned int remainingSequence = 0;
float accumulatedRotation = 0.0f;
float accumulatedRotation_precomputedSin = sin(accumulatedRotation);
float accumulatedRotation_precomputedCos = cos(accumulatedRotation);

unsigned int currentlyPaintingMaterial = MATERIAL_LIFE_SOLID;

float RNG()
{
	static std::default_random_engine e;
	static std::uniform_real_distribution<> dis(0, 1);
	return dis(e);
}

// std::string exampleSentence = " Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque viverra nulla eget fermentum commodo. Nullam ac ex at nisl finibus ultrices. Mauris aliquet elementum turpis, sit amet lobortis magna viverra sed. Proin dignissim hendrerit est, ut convallis dui suscipit sed. Nulla libero justo, euismod malesuada tempus nec, fringilla vel est. Pellentesque sed tellus a purus porttitor vulputate et at nunc. Donec non ipsum scelerisque, placerat sapien sit amet, fringilla est. Fusce pretium urna sit amet hendrerit ultrices. Morbi eget eleifend mi, non feugiat mauris. Praesent non condimentum ligula. Mauris consequat mi ac magna placerat pellentesque. Vestibulum non interdum enim, ac vehicula diam. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Aenean porta libero quis libero rutrum imperdiet. Curabitur vulputate feugiat odio non suscipit. Interdum et malesuada fames ac ante ipsum primis in faucibus. Vivamus erat magna, pulvinar sed felis non volutpat. ";
// std::string exampleSentence = "we'cre off toc see the cwizard, cthe wonderful wizcard of oz! cbecause beccause becaucse becausce because, acll of the wocnderful things che does. do do cdod do do docd doo! ";
std::string exampleSentence = "selnslslul lblotsancol lest sstructra plantae csui insnltl lsemina. Pars asutem fungslil quae ssporlls gignit corpuls fructsferum appellatur.";

float magnitude_int( int x,  int y)
{
	float mag = sqrt(x * x + y * y);
	return mag;
}

vec_f2 rotatePointPrecomputed( vec_f2 center, float s, float c, vec_f2 point)
{
	// translate point back to origin:
	point.x -= center.x;
	point.y -= center.y;

	// rotate point
	float xnew = point.x * c - point.y * s;
	float ynew = point.x * s + point.y * c;

	// translate point back:
	point.x = xnew + center.x;
	point.y = ynew + center.y;
	return vec_f2(point.x, point.y);
};

// lookup table for character alphanumeric values
int alphanumeric (char c)
{
	int val = 0;

	switch (c)
	{
	case 'a': { val = 1; break; }
	case 'b': { val = 2; break; }
	case 'c': { val = 3; break; }
	case 'd': { val = 4; break; }
	case 'e': { val = 5; break; }
	case 'f': { val = 6; break; }
	case 'g': { val = 7; break; }
	case 'h': { val = 8; break; }
	case 'i': { val = 9; break; }
	case 'j': { val = 10; break; }
	case 'k': { val = 11; break; }
	case 'l': { val = 12; break; }
	case 'm': { val = 13; break; }
	case 'n': { val = 14; break; }
	case 'o': { val = 15; break; }
	case 'p': { val = 16; break; }
	case 'q': { val = 17; break; }
	case 'r': { val = 18; break; }
	case 's': { val = 19; break; }
	case 't': { val = 20; break; }
	case 'u': { val = 21; break; }
	case 'v': { val = 22; break; }
	case 'w': { val = 23; break; }
	case 'x': { val = 24; break; }
	case 'y': { val = 25; break; }
	case 'z': { val = 26; break; }

	case 'A': { val = 1; break; }
	case 'B': { val = 2; break; }
	case 'C': { val = 3; break; }
	case 'D': { val = 4; break; }
	case 'E': { val = 5; break; }
	case 'F': { val = 6; break; }
	case 'G': { val = 7; break; }
	case 'H': { val = 8; break; }
	case 'I': { val = 9; break; }
	case 'J': { val = 10; break; }
	case 'K': { val = 11; break; }
	case 'L': { val = 12; break; }
	case 'M': { val = 13; break; }
	case 'N': { val = 14; break; }
	case 'O': { val = 15; break; }
	case 'P': { val = 16; break; }
	case 'Q': { val = 17; break; }
	case 'R': { val = 18; break; }
	case 'S': { val = 19; break; }
	case 'T': { val = 20; break; }
	case 'U': { val = 21; break; }
	case 'V': { val = 22; break; }
	case 'W': { val = 23; break; }
	case 'X': { val = 24; break; }
	case 'Y': { val = 25; break; }
	case 'Z': { val = 26; break; }

	case '1': { val = 1; break; }
	case '2': { val = 2; break; }
	case '3': { val = 3; break; }
	case '4': { val = 4; break; }
	case '5': { val = 5; break; }
	case '6': { val = 6; break; }
	case '7': { val = 7; break; }
	case '8': { val = 8; break; }
	case '9': { val = 9; break; }
	// case '0': { val = 0; break; }

	case '`': { val = 1; break; }
	case '~': { val = 2; break; }
	case '!': { val = 3; break; }
	case '@': { val = 4; break; }
	case '#': { val = 5; break; }
	case '$': { val = 6; break; }
	case '%': { val = 7; break; }
	case '^': { val = 8; break; }
	case '&': { val = 9; break; }
	case '*': { val = 10; break; }
	case '(': { val = 11; break; }
	case ')': { val = 12; break; }
	case '-': { val = 13; break; }
	case '+': { val = 14; break; }
	case '[': { val = 15; break; }
	case ']': { val = 16; break; }
	case '{': { val = 17; break; }
	case '}': { val = 18; break; }
	case '|': { val = 19; break; }
	case ';': { val = 20; break; }
	case ':': { val = 21; break; }
	case '<': { val = 22; break; }
	case '>': { val = 23; break; }
	case ',': { val = 24; break; }
	case '.': { val = 25; break; }
	case '/': { val = 26; break; }
	case '?': { val = 27; break; }
	}

	return val;
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

int drawCharacter (lifeform * creature)
{
	// creature->prevCursor_string = creature->cursor_string;

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

		printf("branching. old rotation %f , new %f\n", trunkRotation, accumulatedRotation );

		recursion_depth++;
		while (1)
		{
			if ( drawCharacter(creature) < 0)
			{
				break;
			}
		}
		recursion_depth--;

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

		printf("entering sequence");

		// get number of times to repeat the sequence
		int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( creature->genes[creature->cursor_string] ); creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;} }

		int repeats = numberModifier;

		printf("repeats %u\n", repeats);

		// the character after that is the next thing to be arrayed
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}

		unsigned int sequenceOrigin = creature->cursor_string;

		recursion_depth ++;
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
		recursion_depth--;
		break;
	}
	case ' ': // break innermost array
	{
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}

		printf("ending array");

		return -1;
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
			if (signY > 13) { signY = 1; } else { signY = -1; }
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
		printf("drawing a line\n");

		printf("offset x: %i\n", numberModifier);

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

		// printf("offset y: %i\n", numberModifier);

		vec_f2 rotatedPoint = rotatePointPrecomputed( vec_f2(creature->prevCursor_grid.x, creature->prevCursor_grid.y),  accumulatedRotation_precomputedSin,  accumulatedRotation_precomputedCos, vec_f2(  creature->cursor_grid.x, creature->cursor_grid.y));

		// printf("rotatedPOint %f %f \n", rotatedPoint.x, rotatedPoint.y);

		if 		(rotatedPoint.x > 0 && rotatedPoint.x < sizeX - 1) 	{ creature->cursor_grid.x = rotatedPoint.x;}
		else if (rotatedPoint.x < 0) 								{ creature->cursor_grid.x = 0;}
		else if (rotatedPoint.x > sizeX - 1) 							{ creature->cursor_grid.x = sizeX - 1; }

		if 		(rotatedPoint.y > 0 && rotatedPoint.y < sizeY - 1) 	{ creature->cursor_grid.y = rotatedPoint.y;}
		else if (rotatedPoint.y < 0) 								{ creature->cursor_grid.y = 0;}
		else if (rotatedPoint.y > sizeY - 1) 							{ creature->cursor_grid.y = sizeY - 1; }

		v = EFLA_E( creature->prevCursor_grid, creature->cursor_grid);

		break;
	}

	default:
	{
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}
	}

	}

	// paint in the points with material
	unsigned int n_points = 0;
	for (std::list<vec_u2>::iterator it = v.begin(); it != v.end(); ++it)
	{
		material_grid[it->x][it->y] = currentlyPaintingMaterial;
		identity_grid[it->x][it->y] = creature->identity;
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

void instantiateCreature(std::string sentence, unsigned int x, unsigned int y)
{

	lifeform creature = lifeform(sentence);


	// place a seed at the coordinates
	material_grid[x][y] = MATERIAL_LIFE_POWDER;
	identity_grid[x][y] = creature.identity;

	creatures.push_back(
	    creature
	);



}

void initialize ()
{
	memset( material_grid, MATERIAL_VACUUM, (sizeof(unsigned int) * sizeX * sizeY) );

	material_grid[0][0] = MATERIAL_GOLD;



	for (unsigned int x = 0; x < sizeX; ++x)
	{
		for (unsigned int y = 0; y < 25; ++y)
		{

			material_grid[x][y] = MATERIAL_STONE;

		}
	}

	instantiateCreature(exampleSentence, 100, 100);
}

void deepgardenGraphics()
{
	preDraw();

	unsigned int nVertsToRenderThisTurn = 4 * sizeX * sizeY;
	unsigned int nIndicesToUseThisTurn 	= 5 * sizeX * sizeY;
	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

	// Create the buffer.
	unsigned int g_vertex_buffer_cursor = 0;

	float * vertex_buffer_data  = new float[totalNumberOfFields];

	unsigned int index_buffer_cursor = 0;
	unsigned int index_buffer_content = 0;
	unsigned int index_buffer_data[nIndicesToUseThisTurn];

	for (unsigned int x = 0; x < sizeX; ++x)
	{
		for (unsigned int y = 0; y < sizeY; ++y)
		{
			color colorToUse = emptyColor;

			if ((material_grid[x][y] & (MATERIAL_PHOTON))   == (MATERIAL_PHOTON))     {    colorToUse = lightColor;  }
			if ((material_grid[x][y] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))   {    colorToUse = emptyColor; }
			if ((material_grid[x][y] & (MATERIAL_LIFE_SOLID))   == (MATERIAL_LIFE_SOLID))     {    colorToUse = lifeColor;  }
			if ((material_grid[x][y] & (MATERIAL_LIFE_POWDER))   == (MATERIAL_LIFE_POWDER))     {    colorToUse = lifeColor;  }
			if ((material_grid[x][y] & (MATERIAL_LIFE_LIQUID))   == (MATERIAL_LIFE_LIQUID))     {    colorToUse = lifeColor;  }
			if ((material_grid[x][y] & (MATERIAL_LIFE_GAS))   == (MATERIAL_LIFE_GAS))     {    colorToUse = lifeColor;  }

			if ((material_grid[x][y] & (MATERIAL_STONE)) == (MATERIAL_STONE))    {    colorToUse = stoneColor; }
			if ((material_grid[x][y] & (MATERIAL_GOLD)) == (MATERIAL_GOLD))    {    colorToUse = goldColor; }

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

	delete [] vertex_buffer_data;

	postDraw();
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

void deepgardenLoop()
{

	// iterate creatures and grow them if appropriate
	for (std::list<lifeform>::iterator creature = creatures.begin(); creature != creatures.end(); ++creature)
	{
		if (creature->energy > 0) 
		{
			creature->energy -= drawNextSequence ( &(*creature) );
		}
	}

	// shine the lamp
	unsigned int topRowIndex = sizeY - 2;
	for (unsigned int x = 0; x < sizeX - 1; ++x)
	{
		if (RNG() < 0.005)
		{
			material_grid[x][topRowIndex] = MATERIAL_PHOTON;
		}
	}

	for (unsigned int x = 1; x < sizeX - 1; ++x)
	{
		for (unsigned int y = 1; y < sizeY - 1; ++y)
		{
			// move physical blocks on the material grid

			// solids do not move

			// movement instructions for POWDERS
			if ((material_grid[x][y] & (MATERIAL_LIFE_POWDER)) == (MATERIAL_LIFE_POWDER))
			{
				for (int i = -1; i < 2; ++i)
				{
					if ((material_grid[x + i][y - 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
					{
						material_grid[x + i][y - 1] = MATERIAL_LIFE_POWDER;
						material_grid[x][y] = MATERIAL_VACUUM;

						// swap identity values of two cells
						unsigned int temp_id = identity_grid[x + i][y - 1];
						identity_grid[x + i][y - 1] = identity_grid[x][y];
						identity_grid[x][y] = temp_id;

					}

					// begin to grow
					else if ((material_grid[x + i][y - 1] & (MATERIAL_STONE)) == (MATERIAL_STONE))
					{
						lifeform * sprout = getCreatureByID(identity_grid[x][y]);
						if (sprout != nullptr) 
						{
							sprout->germinated = true;
						}
					}
				}
			}

			// movement instructions for LIQUIDS
			if ((material_grid[x][y] & (MATERIAL_LIFE_LIQUID)) == (MATERIAL_LIFE_LIQUID))
			{
				for (int i = -1; i < 2; ++i)
				{
					for (int j = -1; j < 1; ++j)
					{
						if ((material_grid[x + i][y + j] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
						{
							material_grid[x + i][y + j] = MATERIAL_LIFE_POWDER;
							material_grid[x][y] = MATERIAL_VACUUM;

							// swap identity values of two cells
							unsigned int temp_id = identity_grid[x + i][y + j];
							identity_grid[x + i][y + j] = identity_grid[x][y];
							identity_grid[x][y] = temp_id;

						}
					}
				}
			}

			// movement instructions for GASES
			if ((material_grid[x][y] & (MATERIAL_LIFE_GAS)) == (MATERIAL_LIFE_GAS))
			{
				for (int i = -1; i < 2; ++i)
				{
					for (int j = -1; j < 2; ++j)
					{
						if ((material_grid[x + i][y + j] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
						{
							material_grid[x + i][y + j] = MATERIAL_LIFE_GAS;
							material_grid[x][y] = MATERIAL_VACUUM;

							// swap identity values of two cells
							unsigned int temp_id = identity_grid[x + i][y + j];
							identity_grid[x + i][y + j] = identity_grid[x][y];
							identity_grid[x][y] = temp_id;
						}
					}
				}
			}

			// movement instructions for PHOTONS
			if ((material_grid[x][y] & (MATERIAL_PHOTON)) == (MATERIAL_PHOTON))
			{
				// move down towards the earth
				if ((material_grid[x][y - 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					material_grid[x][y - 1] = MATERIAL_PHOTON;
					material_grid[x][y] = MATERIAL_VACUUM;
				}
				// disappear if you hit something
				else
				{
					material_grid[x][y] = MATERIAL_VACUUM;

					lifeform * fellOnThisThing = getCreatureByID(identity_grid[x][y - 1]);

					if (fellOnThisThing != nullptr)
					{
						fellOnThisThing->energy++;
					}
				}
			}
		}
	}
}
