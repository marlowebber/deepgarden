#include "deepgarden.h"
#include "deepgarden_graphics.h"

unsigned int totalSize = sizeX * sizeY;


static unsigned int material_grid[totalSize];
static unsigned int phase_grid[totalSize];
static unsigned int identity_grid[totalSize];
static unsigned int temperature_grid[totalSize];



color lightColor = color(1.0f, 1.0f, 1.0f, 1.0f);
color emptyColor = color(0.05f, 0.05f, 0.05f, 1.0f);
color lifeColor = color(0.2f, 0.05f, 0.4f, 1.0f);
color stoneColor = color(0.5f, 0.5f, 0.5f, 1.0f);
color goldColor = color(1.0f, 1.0f, 0.0f, 1.0f);

unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

std::list<unsigned int> identities;



// std::string exampleSentence = " Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque viverra nulla eget fermentum commodo. Nullam ac ex at nisl finibus ultrices. Mauris aliquet elementum turpis, sit amet lobortis magna viverra sed. Proin dignissim hendrerit est, ut convallis dui suscipit sed. Nulla libero justo, euismod malesuada tempus nec, fringilla vel est. Pellentesque sed tellus a purus porttitor vulputate et at nunc. Donec non ipsum scelerisque, placerat sapien sit amet, fringilla est. Fusce pretium urna sit amet hendrerit ultrices. Morbi eget eleifend mi, non feugiat mauris. Praesent non condimentum ligula. Mauris consequat mi ac magna placerat pellentesque. Vestibulum non interdum enim, ac vehicula diam. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Aenean porta libero quis libero rutrum imperdiet. Curabitur vulputate feugiat odio non suscipit. Interdum et malesuada fames ac ante ipsum primis in faucibus. Vivamus erat magna, pulvinar sed felis non volutpat. ";
// std::string exampleSentence = "we'cre off toc see the cwizard, cthe wonderful wizcard of oz! cbecause beccause becaucse becausce because, acll of the wocnderful things che does. do do cdod do do docd doo! ";
std::string exampleSentence = "selnslslul lblotsancol lest sstructra plantae csui insnltl lsemina. Pars asutem fungslil quae ssporlls gignit corpuls fructsferum appellatur.";



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



struct lifeform
{
	unsigned int identity;
	int energy;
	std::string genes;

	unsigned int cursor_string;
	unsigned int geneSize;
	unsigned int cursor_grid;
	unsigned int prevCursor_grid;
	float accumulatedRotation ;
	float accumulatedRotation_precomputedSin;
	float accumulatedRotation_precomputedCos;

	bool germinated;

	lifeform(std::string a)
	{
		this->energy = 0;
		this->genes = a;

		this->cursor_string = 0;
		this->cursor_grid 		= 0;
		this->prevCursor_grid 	= 0;
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

unsigned int currentlyPaintingMaterial = MATERIAL_LIFE_SOLID;

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

		while (1)
		{
			if ( drawCharacter(creature) < 0)
			{
				break;
			}
		}

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
	}
	case ' ': // break innermost array
	{
		creature->cursor_string++; if (creature->cursor_string > creature->geneSize) {return -1;}

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

	// paint in the points with material
	unsigned int n_points = 0;
	for (std::list<vec_u2>::iterator it = v.begin(); it != v.end(); ++it)
	{
		phase_grid[it->x][it->y] 	= PHASE_POWDER;
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
	phase_grid   [x][y] = PHASE_POWDER;
	identity_grid[x][y] = creature.identity;

	creatures.push_back(
	    creature
	);
}

void initialize ()
{
	// memset( phase_grid, MATERIAL_VACUUM, (sizeof(unsigned int) * sizeX * sizeY) );

	// phase_grid[0] = MATERIAL_GOLD;

	// for (unsigned int x = 0; x < sizeX; ++x)
	// {
	// 	for (unsigned int y = 0; y < 25; ++y)
	// 	{
	// 		phase_grid[x][y] = MATERIAL_STONE;
	// 	}
	// }

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

			// if ((phase_grid[x][y] & (MATERIAL_PHOTON))   == (MATERIAL_PHOTON))     {    colorToUse = lightColor;  }
			// if ((phase_grid[x][y] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))   {    colorToUse = emptyColor; }
			// if ((phase_grid[x][y] & (MATERIAL_LIFE_SOLID))   == (MATERIAL_LIFE_SOLID))     {    colorToUse = lifeColor;  }
			// if ((phase_grid[x][y] & (MATERIAL_LIFE_POWDER))   == (MATERIAL_LIFE_POWDER))     {    colorToUse = lifeColor;  }
			// if ((phase_grid[x][y] & (MATERIAL_LIFE_LIQUID))   == (MATERIAL_LIFE_LIQUID))     {    colorToUse = lifeColor;  }
			// if ((phase_grid[x][y] & (MATERIAL_LIFE_GAS))   == (MATERIAL_LIFE_GAS))     {    colorToUse = lifeColor;  }

			// if ((phase_grid[x][y] & (MATERIAL_STONE)) == (MATERIAL_STONE))    {    colorToUse = stoneColor; }
			// if ((phase_grid[x][y] & (MATERIAL_GOLD)) == (MATERIAL_GOLD))    {    colorToUse = goldColor; }

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
	material_grid[a] = temp_id;

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


	for (unsigned int i = 0; i < totalSize; ++i)
	{


		unsigned int squareBelow = i - sizeX;

		if ((phase_grid[i] & (PHASE_POWDER)) == (PHASE_POWDER))
		{

			unsigned int neighbours[] = {
				squareBelow + 1,
				squareBelow,
				squareBelow - 1
			}

			for (unsigned int j = 0; j < 3; ++j)
			{
				if ((phase_grid[neighbours[j]] & (PHASE_VACUUM)) == (PHASE_VACUUM))
				{
					// phase_grid[neighbours[j]] = PHASE_POWDER;
					// phase_grid[i] = PHASE_VACUUM;

					swap(i, neighbours[j]);
					break;
				}
			}


		}

		// movement instructions for LIQUIDS
		if ((phase_grid[i] & (PHASE_LIQUID)) == (PHASE_LIQUID))
		{
			unsigned int neighbours[] = {
				i - 1,
				i + 1,
				squareBelow + 1,
				squareBelow,
				squareBelow - 1
			}


			for (unsigned int j = 0; j < 5; ++j)
			{
				if ((phase_grid[neighbours[j]] & (PHASE_VACUUM)) == (PHASE_VACUUM))
				{
					// phase_grid[neighbours[j]] = PHASE_LIQUID;
					// phase_grid[i] = PHASE_VACUUM;

					swap(i, neighbours[j]);
					break;
				}
			}
		}

		// movement instructions for GASES
		if ((phase_grid[i] & (PHASE_GAS)) == (PHASE_GAS))
		{
			unsigned int squareAbove = i + sizeX;
			unsigned int neighbours[] = {
				squareAbove + 1,
				squareAbove,
				squareAbove - 1
				i - 1,
				i + 1,
				squareBelow + 1,
				squareBelow,
				squareBelow - 1
			}


			for (unsigned int j = 0; j < 8; ++j)
			{
				if ((phase_grid[neighbours[j]] & (PHASE_VACUUM)) == (PHASE_VACUUM))
				{

					if (RNG() < 0.125)
					{
						// phase_grid[neighbours[j]] = PHASE_GAS;
						// phase_grid[i] = PHASE_VACUUM;

						swap(i, neighbours[j]);
						break;
					}
				}
			}
		}



		// // movement instructions for PHOTONS
		// if ((phase_grid[i] & (PHASE_LIGHT)) == (PHASE_LIGHT))
		// {
		// 	// move down towards the earth
		// 	if ((phase_grid[x][y - 1] & (PHASE_VACUUM)) == (PHASE_VACUUM))
		// 	{
		// 		phase_grid[x][y - 1] = PHASE_LIGHT;
		// 		phase_grid[x][y] = PHASE_VACUUM;
		// 	}
		// 	// disappear if you hit something
		// 	else
		// 	{
		// 		phase_grid[x][y] = PHASE_VACUUM;

		// 		lifeform * fellOnThisThing = getCreatureByID(identity_grid[x][y - 1]);

		// 		if (fellOnThisThing != nullptr)
		// 		{
		// 			fellOnThisThing->energy++;
		// 		}
		// 	}
		// }
		// }
	}
}
