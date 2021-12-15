#include "deepgarden.h"
#include "deepgarden_graphics.h"


static unsigned int grid[sizeX][sizeY];


color sandColor = color(0.2f, 0.2f, 0.2f, 1.0f);
color emptyColor = color(0.05f, 0.05f, 0.05f, 1.0f);
color lifeColor = color(0.2f, 0.05f, 0.4f, 1.0f);
color stoneColor = color(0.5f, 0.5f, 0.5f, 1.0f);

color goldColor = color(1.0f, 1.0f, 0.0f, 1.0f);

unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

unsigned int cursorLimit = 0;

unsigned int cursor_string = 0;

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


vec_u2 cursor_grid 		= vec_u2(0, 0);
vec_u2 prevCursor_grid 	= vec_u2(0, 0);

// variables keep track of the sequence and rotation states.
unsigned int remainingSequence = 0;
float accumulatedRotation = 0.0f;
float accumulatedRotation_precomputedSin = sin(accumulatedRotation);
float accumulatedRotation_precomputedCos = cos(accumulatedRotation);

float colorCursorR = 0.0f;
float colorCursorG = 0.0f;
float colorCursorB = 0.0f;

unsigned int currentlyPaintingMaterial = MATERIAL_STONE;





float RNG()
{
	static std::default_random_engine e;
	static std::uniform_real_distribution<> dis(0, 1);
	return dis(e);
}

std::string exampleSentence = " Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque viverra nulla eget fermentum commodo. Nullam ac ex at nisl finibus ultrices. Mauris aliquet elementum turpis, sit amet lobortis magna viverra sed. Proin dignissim hendrerit est, ut convallis dui suscipit sed. Nulla libero justo, euismod malesuada tempus nec, fringilla vel est. Pellentesque sed tellus a purus porttitor vulputate et at nunc. Donec non ipsum scelerisque, placerat sapien sit amet, fringilla est. Fusce pretium urna sit amet hendrerit ultrices. Morbi eget eleifend mi, non feugiat mauris. Praesent non condimentum ligula. Mauris consequat mi ac magna placerat pellentesque. Vestibulum non interdum enim, ac vehicula diam. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Aenean porta libero quis libero rutrum imperdiet. Curabitur vulputate feugiat odio non suscipit. Interdum et malesuada fames ac ante ipsum primis in faucibus. Vivamus erat magna, pulvinar sed felis non volutpat. ";




float magnitude_int( int x,  int y)
{
	float mag = sqrt(x * x + y * y);
	return mag;
}

// be careful mixing signed and unsigned arithmetic!
vec_i2 rotatePointPrecomputed( vec_u2 ucenter, float s, float c, vec_u2 upoint)
{
	// float s = sin(angle);
	// float c = cos(angle);

	vec_i2 center = vec_i2(ucenter.x, ucenter.y);
	vec_i2 point = vec_i2(upoint.x, upoint.y);

	// translate point back to origin:
	point.x -= center.x;
	point.y -= center.y;

	// rotate point
	float xnew = point.x * c - point.y * s;
	float ynew = point.x * s + point.y * c;

	// translate point back:
	point.x = xnew + center.x;
	point.y = ynew + center.y;
	return vec_i2(point.x, point.y);
};



// lookup table for character alphanumeric values
unsigned int alphanumeric (char c)
{

	unsigned int val = 0;

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
	case '0': { val = 0; break; }

		// case '`': { val = 1; break; }
		// case '~': { val = 2; break; }
		// case '!': { val = 3; break; }
		// case '@': { val = 4; break; }
		// case '#': { val = 5; break; }
		// case '$': { val = 6; break; }
		// case '%': { val = 7; break; }
		// case '^': { val = 8; break; }
		// case '&': { val = 9; break; }
		// case '*': { val = 1; break; }
		// case '(': { val = 2; break; }
		// case ')': { val = 4; break; }
		// case '-': { val = 8; break; }
		// case '+': { val = 16; break; }
		// case '[': { val = 32; break; }
		// case ']': { val = 64; break; }
		// case '{': { val = 128; break; }
		// case '}': { val = 256; break; }
		// case '|': { val = 1024; break; }
		// case ';': { val = 32; break; }
		// case ':': { val = 64; break; }
		// case '<': { val = 128; break; }
		// case '>': { val = 256; break; }
		// case ',': { val = 1; break; }
		// case '.': { val = 2; break; }
		// case '/': { val = 1024; break; }
		// case '?': { val = 4; break; }
	}

	return val;

}


// THE EXTREMELY FAST LINE ALGORITHM Variation E (Addition Fixed Point PreCalc Small Display)
// Small Display (256x256) resolution.
std::vector<vec_u2> EFLA_E(vec_u2 start, vec_u2 end)
{

	int x  = start.x;
	int y  = start.y;
	int x2 = end.x;
	int y2 = end.y;

	std::vector<vec_u2> v;

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


void drawCharacter (std::string s)
{

	char c = s[cursor_string];

	printf("drawing character: %c\n", c);

	printf("cursor location: %u %u\n", cursor_grid.x , cursor_grid.y);

	switch (c)
	{
	case 'c': // paint a circle at the cursor
	{

		printf("c: drawing circle\n");
		cursor_string++;

		unsigned int numberModifier = 0;
		while (!numberModifier) {	numberModifier = alphanumeric( s[cursor_string] ); cursor_string++; if (cursor_string > cursorLimit) {return;} }

		// get the circle radius (the next number in the string.)
		unsigned int radius = numberModifier;

		printf("with radius %u\n", radius);

		// make a list of integer cell addresses
		std::vector<vec_u2> v;

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

		// paint in the points with material
		for (std::vector<vec_u2>::iterator it = v.begin(); it != v.end(); ++it)
		{
			grid[it->x][it->y] = currentlyPaintingMaterial;
		}

		break;
	}

	case 'l':
	{
		// rasters a line, taking into account the accumulated rotation

		printf("l: drawing line\n");

		// set the previous x and y, which will be the start of the line
		prevCursor_grid = cursor_grid;

		// read the endpoints in from file, and apply the rotation
		// cursor_string++;

		bool chosenSignX = false;
		int signX = 1;

		while (!chosenSignX)
		{
			cursor_string++; if (cursor_string > cursorLimit) {return;}
			signX = alphanumeric( s[cursor_string]);
			if (signX > 13) { signX = 1; } else { signX = -1; }
			chosenSignX = true;
		}

		bool chosenSignY = false;
		int signY = 1;

		while (!chosenSignY)
		{
			cursor_string++; if (cursor_string > cursorLimit) {return;}
			signY = alphanumeric( s[cursor_string]);
			if (signY > 13) { signY = 1; } else { signY = -1; }
			chosenSignY = true;
		}



		int numberModifier = 0;

		while (!numberModifier)
		{
			cursor_string++; if (cursor_string > cursorLimit) {return;}
			numberModifier = alphanumeric( s[cursor_string] );
		}

		
		numberModifier = numberModifier * signX;

		if (cursor_grid.x + (numberModifier ) > sizeX ) 
		{
			return;
		}
		if (cursor_grid.x + (numberModifier ) < 0 ) 
		{
			return;
		}

		cursor_grid.x += (numberModifier );

		printf("offset x: %u\n", numberModifier);

		numberModifier = 0;

		while (!numberModifier)
		{
			cursor_string++; if (cursor_string > cursorLimit) {return;}
			numberModifier = alphanumeric( s[cursor_string] );
		}

		numberModifier = numberModifier * signY;

		if (cursor_grid.y + (numberModifier ) > sizeY ) 
		{
			return;
		}
		if (cursor_grid.y + (numberModifier ) < 0 ) 
		{
			return;
		}


		cursor_grid.y += numberModifier;

		printf("offset y: %u\n", numberModifier);

		vec_i2 rotatedPoint = rotatePointPrecomputed( prevCursor_grid,  accumulatedRotation_precomputedSin,  accumulatedRotation_precomputedCos, cursor_grid);
		cursor_grid.x = rotatedPoint.x;
		cursor_grid.y = rotatedPoint.y;

		// raster the line
		std::vector<vec_u2> v = EFLA_E( prevCursor_grid, cursor_grid);

		// fill the grid points with material
		for (std::vector<vec_u2>::iterator it = v.begin(); it != v.end(); ++it)
		{
			if (it->x > 0 && it->x < sizeX && it->y > 0 && it->y < sizeY )
			{
				grid[it->x][it->y] = currentlyPaintingMaterial;
			}
		}
		break;
	}

	default:
	{
		cursor_string++; if (cursor_string > cursorLimit) {return;}
	}

	}

}


void drawFromSentence (std::string sentence, unsigned int startX, unsigned int startY)
{

	if (sentence.length() <= 0)
	{
		return;
	}

	cursorLimit = sentence.length() - 1;

	if (cursorLimit == 1)
	{
		return;
	}

	cursor_grid = vec_u2(startX, startY);

	prevCursor_grid = cursor_grid;

	// prev_x = startX; // moving cursor position from last turn
	// prev_y = startY;

	// variables keep track of the sequence and rotation states.
	unsigned int remainingSequence = 0;
	float accumulatedRotation = 0.0f;

	float colorCursorR = 0.0f;
	float colorCursorG = 0.0f;
	float colorCursorB = 0.0f;

	unsigned int currentlyPaintingMaterial = MATERIAL_STONE;


	cursor_string = 0; // keep track of the place in the string

	/**
	 *
	 *
	 * RULES LIST
	 *
	 *
	 *
	 * // modifier commands
	* s : sequence n times
	* h : mirror horizontally
	* v : mirror vertically
	* ,  : break out of innermost array, sequence, or mirror structure.
	* (space) or . : break out of all array, sequence, or mirror structures
	* +  : plus separates numbers
	* - : minus separates and inverts numbers
	* o : rotate
	*
	* // drawing commands
	* l : draw line (the next number is the x offset, the number after that is the y offset, the one after that is the thickness. A line is interpolated between them.)
	* q : paint square (next number is radius)
	* d : paint diamond
	* c : paint circle (next number is radius)
	* e : extrude, next number is direction (four cardinals + expand in all directions), number afterwards is amount of pixels
	*
	* // color commands
	* r: next number is red component
	* g: next number is green component
	* b: next number is blue component
	*
	* // body commands
	* f : fruit (a new seed is produced here)


	 *
	 *
	 *
	 *
	 * */




	for (unsigned int i = 0; i < cursorLimit ; i ++)
	{

		//
		// void drawCharacter (std::string s, unsigned int * cursor)
		drawCharacter (sentence );


	}

}


void initialize ()
{
	memset( grid, 0x00, (sizeof(unsigned int) * sizeX * sizeY) );

	for (unsigned int x = 0; x < sizeX; ++x)
	{
		for (unsigned int y = 0; y < sizeY; ++y)
		{
			// if (RNG() > 0.5)
			// {
			// 	grid[x][y] = MATERIAL_SAND;
			// }
			// else
			// {
			// 	grid[x][y] = MATERIAL_VACUUM;
			// }

			// if (RNG() < 0.005)
			// {
			// 	grid[x][y] = MATERIAL_STONE;
			// }



		}
	}


	grid[0][0] = MATERIAL_GOLD;


	drawFromSentence (exampleSentence, 128, 128 );

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

	// prepareForWorldDraw();

	for (unsigned int x = 0; x < sizeX; ++x)
	{
		for (unsigned int y = 0; y < sizeY; ++y)
		{
			color colorToUse = emptyColor;

			if ((grid[x][y] & (MATERIAL_SAND))   == (MATERIAL_SAND))     {    colorToUse = sandColor;  }
			if ((grid[x][y] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))   {    colorToUse = emptyColor; }
			if ((grid[x][y] & (MATERIAL_LIFE))   == (MATERIAL_LIFE))     {    colorToUse = lifeColor;  }
			if ((grid[x][y] & (MATERIAL_STONE)) == (MATERIAL_STONE))    {    colorToUse = stoneColor; }

			if ((grid[x][y] & (MATERIAL_GOLD)) == (MATERIAL_GOLD))    {    colorToUse = goldColor; }

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

void deepgardenLoop()
{
	for (unsigned int x = 1; x < sizeX - 1; ++x)
	{
		for (unsigned int y = 1; y < sizeY - 1; ++y)
		{
			if ((grid[x][y] & (MATERIAL_SAND)) == (MATERIAL_SAND))
			{

				if ((grid[x][y - 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x][y - 1] = MATERIAL_SAND;
					grid[x][y] = MATERIAL_VACUUM;
				}

				else if ((grid[x - 1][y - 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x - 1][y - 1] = MATERIAL_SAND;
					grid[x][y] = MATERIAL_VACUUM;
				}

				else if ((grid[x + 1][y - 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x + 1][y - 1] = MATERIAL_SAND;
					grid[x][y] = MATERIAL_VACUUM;
				}
			}

			if ((grid[x][y] & (MATERIAL_LIFE)) == (MATERIAL_LIFE))
			{


				if ((grid[x][y - 1] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x][y - 1]  = MATERIAL_LIFE;
				}

				else if ((grid[x + 1][y ] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x + 1][y ]  = MATERIAL_LIFE;
				}
				else if ((grid[x - 1][y] & (MATERIAL_VACUUM)) == (MATERIAL_VACUUM))
				{
					grid[x - 1][y ]  = MATERIAL_LIFE;
				}


			}

		}
	}
}
