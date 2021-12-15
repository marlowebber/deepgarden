#include "deepgarden.h"
#include "deepgarden_graphics.h"


static unsigned int grid[sizeX][sizeY];


color sandColor = color(0.2f, 0.2f, 0.2f, 1.0f);
color emptyColor = color(0.05f, 0.05f, 0.05f, 1.0f);
color lifeColor = color(0.2f, 0.05f, 0.4f, 1.0f);
color stoneColor = color(0.5f, 0.5f, 0.5f, 1.0f);


unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */




	// unsigned int x = 0;
	// unsigned int y = 0; 		// moving cursor on the grid

	uvec2 cursor = uvec2(0,0);

	// unsigned int prev_x = 0; // moving cursor position from last turn
	// unsigned int prev_y = 0;

	uvec2 prevCursor = uvec2(0,0);

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

std::string exampleSentence = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";



struct uvec2
{
	unsigned int x;
	unsigned int y;

	uvec2();
};

uvec2::uvec2(unsigned int a, unsigned int b) 
{
	this.x = a;
	this.y = b;

}

float magnitude_int( int x,  int y) 
{
	float mag = sqrt(x * x + y * y);
	return mag; 
}


point_uint rotatePointPrecomputed( uvec2 center, float s, float c, uvec2 point) 
{
	// float s = sin(angle);
	// float c = cos(angle);

	// translate point back to origin:
	point.x -= center.x;
	point.y -= center.y;

	// rotate point
	float xnew = point.x * c - point.y * s;
	float ynew = point.x * s + point.y * c;

	// translate point back:
	point.x = xnew + center.x;
	point.y = ynew + center.y;
	return uvec2(point.x, point.y);
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
std::vector<uvec2> EFLA_E(int x, int y, int x2, int y2) 
{

	std::vector<uvec2> v;

   	bool yLonger=false;
	int shortLen=y2-y;
	int longLen=x2-x;
	if (abs(shortLen)>abs(longLen)) {
		int swap=shortLen;
		shortLen=longLen;
		longLen=swap;				
		yLonger=true;
	}
	int decInc;
	if (longLen==0) decInc=0;
	else decInc = (shortLen << 8) / longLen;

	if (yLonger) {
		if (longLen>0) {
			longLen+=y;
			for (int j=0x80+(x<<8);y<=longLen;++y) {
				// myPixel(surface,j >> 8,y);	
				v.push_back( uvec2(j >> 8,y) );
				j+=decInc;
			}
			return;
		}
		longLen+=y;
		for (int j=0x80+(x<<8);y>=longLen;--y) {
			// myPixel(surface,j >> 8,y);
			v.push_back( uvec2(j >> 8,y) );	
			j-=decInc;
		}
		return;	
	}

	if (longLen>0) {
		longLen+=x;
		for (int j=0x80+(y<<8);x<=longLen;++x) {
			// myPixel(surface,x,j >> 8);
			v.push_back( uvec2(x,j >> 8) );
			j+=decInc;
		}
		return;
	}
	longLen+=x;
	for (int j=0x80+(y<<8);x>=longLen;--x) {
		// myPixel(surface,x,j >> 8);
		v.push_back( uvec2(x,j >> 8) );
		j-=decInc;
	}

	return v;

}


void drawCharacter (char c) 
{

		switch (c)
		{
		case 'c': // paint a circle at the cursor
			{
			unsigned int numberModifier = 0;
			while (!numberModifier) {	numberModifier = alphanumeric( string[cursor] ); if (cursor < cursorLimit) { cursor++ }; }

			// get the circle radius (the next number in the string.)
			unsigned int radius = numberModifier;

			// make a list of integer cell addresses
			std::vector<uvec2> v;

			// define the range of pixels you'll draw- so you don't have to navigate the entire, massive grid.
			unsigned int drawingAreaLowerX = x - radius;
			unsigned int drawingAreaLowerY = y - radius;
			unsigned int drawingAreaUpperX = x + radius;
			unsigned int drawingAreaUpperY = y + radius;

			// carve out a circle
			for (unsigned int i = searchAddrStartX; i < searchAddrStartX + (2* radius); ++i)
			{
				for (unsigned int j = searchAddrStartY; j < searchAddrStartY + (2* radius); ++j)
				{

					if (  magnitude_int (  i-x , j-y )  < radius ) 
					{
						v.push_back( point_uint(i, j) );
					}
				}
			}

			for(std::vector<T>::iterator it = v.begin(); it != v.end(); ++it) 
			{
				grid[it.x][it.y] = currentlyPaintingMaterial;
			}

			break;
			}

		case 'l':
		{

			prev_x = x;
			prev_y = y;

			unsigned int numberModifier = 0;
			while (!numberModifier) {	numberModifier = alphanumeric( string[cursor] ); if (cursor < cursorLimit) { cursor++ }; }

			x = numberModifier;


			numberModifier = 0;
			while (!numberModifier) {	numberModifier = alphanumeric( string[cursor] ); if (cursor < cursorLimit) { cursor++ }; }

			y = numberModifier;

			rotatedPoint =  rotatePointPrecomputed( point_uint(x,y), float s, float c, point_uint point) 



			std::vector<uvec2> v = EFLA_E(prev_x, prev_y, x, y);

			for(std::vector<T>::iterator it = v.begin(); it != v.end(); ++it) 
			{
				grid[it.x][it.y] = currentlyPaintingMaterial;
			}
			break;
		}

		}

		if (c == '\n')
		{
			break;
		}

}


void drawFromSentence (std::string sentence, unsigned int startX, unsigned int startY)
{
	
	if (sentence.length() <= 0) 
	{
		return;
	}

	unsigned int cursorLimit = sentence.length -1;

	if (cursorLimit == 1) 
	{
		return;
	}

	x = startX;
	y = startY; 		// moving cursor on the grid

	prev_x = startX; // moving cursor position from last turn
	prev_y = startY;

	// variables keep track of the sequence and rotation states.
	unsigned int remainingSequence = 0;
	float accumulatedRotation = 0.0f;

	float colorCursorR = 0.0f;
	float colorCursorG = 0.0f;
	float colorCursorB = 0.0f;

	unsigned int currentlyPaintingMaterial = MATERIAL_STONE;

	char c = ' ';		// present character

	unsigned int cursor = 0; // keep track of the place in the string

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




	for (;;)
	{
	
	}

}


void initialize ()
{
	memset( grid, 0x00, (sizeof(unsigned int) * sizeX * sizeY) );

	for (unsigned int x = 0; x < sizeX; ++x)
	{
		for (unsigned int y = 0; y < sizeY; ++y)
		{
			if (RNG() > 0.5)
			{
				grid[x][y] = MATERIAL_SAND;
			}
			else
			{
				grid[x][y] = MATERIAL_VACUUM;
			}

			// if (RNG() < 0.005)
			// {
			// 	grid[x][y] = MATERIAL_STONE;
			// }



		}
	}
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
			if ((grid[x][y] & (MATERIAL_VACUUM)) == (MATERIAL_STONE))    {    colorToUse = stoneColor; }

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
