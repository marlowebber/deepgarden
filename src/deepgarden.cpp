#include "deepgarden.h"
#include "deepgarden_graphics.h"


static unsigned int grid[sizeX][sizeY];


color sandColor = color(0.2f, 0.2f, 0.2f, 1.0f);
color emptyColor = color(0.05f, 0.05f, 0.05f, 1.0f);
color lifeColor = color(0.2f, 0.05f, 0.4f, 1.0f);
color stoneColor = color(0.5f, 0.5f, 0.5f, 1.0f);


unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

float RNG()
{
	static std::default_random_engine e;
	static std::uniform_real_distribution<> dis(0, 1);
	return dis(e);
}


void drawFromSentence (std::string sentence, int startX, int startY)
{

	int x;
	int y; // moving cursor

	int prev_x; // moving cursor position from last turn
	int prev_y;

	char c;

	float colorCursorR;
	float colorCursorG;
	float colorCursorB;

	for (;;)
	{
		switch (c)
		{
		case 'a':
			break;
		}

		if (c == '\n')
		{
			break;
		}
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
