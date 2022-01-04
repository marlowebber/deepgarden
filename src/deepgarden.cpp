#include "deepgarden.h"
#include "deepgarden_graphics.h"

#include <ctime>
#include <chrono>
#include <iostream>

#define THREAD_TIMING
#define RENDERING_THREADS 4

const unsigned int totalSize = sizeX * sizeY;
const unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */
float colorGrid[totalSize * numberOfFieldsPerVertex ];

struct Particle {
	unsigned int material;
	unsigned int phase;
	int temperature;
};

Particle grid[totalSize];

const Color color_lightblue = Color( 0.1f, 0.3f, 0.65f, 1.0f );
const Color color_yellow    = Color( 1.0f, 1.0f, 0.0f, 1.0f );
const Color color_lightgrey = Color( 0.75f, 0.75f, 0.75f, 1.0f );
const Color color_grey      = Color( 0.50f, 0.50f, 0.50f, 1.0f );
const Color color_darkgrey  = Color( 0.25f, 0.25f, 0.25f, 1.0f );
const Color color_black     = Color( 0.0f, 0.0f, 0.0f, 1.0f );

int wind = 0;

int defaultTemperature = 300;



// b2Color colorOfHotObject (int temperature)
// {

// // https://en.wikipedia.org/wiki/Black-body_radiation#/media/File:Gluehfarben_no_language_horizontal.svg
// // from a blacksmith's color temp chart


// // temp r g b
// // 550  0.2, 0.13, 0.04
// // 630  32.9 15.7 1.2
// // 680  40.8 6.7 0.0
// // 740  52.5 8.6 0
// // 780  62.7 0 0
// // 810  75.7 10.6 10.6
// // 850  83.1 25.5 8.2


// // 900 91.4 34.5 17.3
// // 950 91.4 49.4 11
// // 1000 100 66.7 5.9
// // 1100 98 75 20
// // 1200 100 81 38
// // 1300 100 90 67
// // 1500 100 95 80
// // 2000 100 100 100

// 	if (temperature > 500)
// 	{

// 		int glowyTemp = (temperature - 500);

// 		float red   = glowyTemp / 1000;
// 		float green = glowyTemp / 2000;
// 		float blue  = glowyTemp / 3000;

// 		return b2Color( red   , green   ,blue, blue );
// 	}


// 	return b2Color(  0.0f , 0.0f   , 0.0f, 1.0f );

// }

void initialize ()
{
	setupExtremelyFastNumberGenerators();
	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	srand((unsigned int)time(NULL));

	memset ( colorGrid, 0x00, sizeof(float ) * numberOfFieldsPerVertex * totalSize );

	memset(grid, 0x00, sizeof(Particle) * ( totalSize));

	// // setup the x and y positions in the color grid. these never change so you can just calculate them once.
	unsigned int x = 0;
	unsigned int y = 0;
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		x = i % sizeX;
		if (!x) { y = i / sizeX; }
		float fx = x;
		float fy = y;

		grid[i].temperature = defaultTemperature;
		grid[i].phase = PHASE_VACUUM;

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
				grid[i].material = MATERIAL_OLIVINE;
				grid[i].phase = PHASE_POWDER;
				memcpy(&colorGrid[i * numberOfFieldsPerVertex], &color_darkgrey, 16  );
				grid[i].temperature = defaultTemperature;
			}
		}

		// sprinkle some material on it to make a default scene.
		if (i > (50 * sizeX) && i < (75 * sizeX))
		{
			if (RNG() < 0.5)
			{
				grid[i].material = MATERIAL_AMPHIBOLE;
				grid[i].phase = PHASE_POWDER;
				memcpy(&colorGrid[i * numberOfFieldsPerVertex], &color_grey, 16  );
				grid[i].temperature = defaultTemperature;
			}
		}

		// sprinkle some material on it to make a default scene.
		if (i > (75 * sizeX) && i < (100 * sizeX))
		{
			if (RNG() < 0.5)
			{
				grid[i].material = MATERIAL_QUARTZ;
				grid[i].phase = PHASE_POWDER;
				memcpy(&colorGrid[i * numberOfFieldsPerVertex], &color_lightgrey, 16  );
				grid[i].temperature = defaultTemperature;
			}
		}

		// sprinkle some material on it to make a default scene.
		if (i > (100 * sizeX) && i < (125 * sizeX))
		{
			if (RNG() < 0.5)
			{
				grid[i].material = MATERIAL_WATER;
				grid[i].phase = PHASE_LIQUID;
				memcpy(&colorGrid[i * numberOfFieldsPerVertex], &color_lightblue, 16  );
				grid[i].temperature = defaultTemperature;
			}
		}
	}
}

void swap (unsigned int a, unsigned int b)
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

void heatEverything ()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		if (grid[i].temperature < 5000)
		{
			grid[i].temperature += 25;
		}
	}
}


void coolEverything ()
{
	for (unsigned int i = 0; i < totalSize; ++i)
	{
		if (grid[i].temperature > 100)
		{
			grid[i].temperature -= 25;
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

void thread_temperature2 ()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	for (unsigned int i = (sizeX + 1); i < (totalSize - (sizeX + 1)); ++i)
	{





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
					swap(i, neighbours[index]);
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

			for (uint8_t k = 0; k < 2; ++k)
			{
				uint8_t index = extremelyFastNumberFromZeroTo(4);
				// chemistry(i, neighbours[index]) ;
				if ((grid[neighbours[index]].phase == PHASE_VACUUM) || (grid[neighbours[index]].phase == PHASE_GAS) ||  (grid[neighbours[index]].phase == PHASE_LIQUID)     )
				{
					swap(i, neighbours[index]);
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
					swap(i, neighbours[index]);


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
	uint16_t windChange = extremelyFastNumberFromZeroTo(100);

	if (!windChange)
	{
		uint16_t windRand =  extremelyFastNumberFromZeroTo( 2);
		wind = windRand - 1;
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



// void thread_chemistry ()
// {


// 	for (unsigned int i = sizeX + 1; i < totalSize - sizeX - 1; ++i)
// 	{

// 		if (true)
// 		{

// 			unsigned int squareBelow = i - sizeX;

// 			if ((grid[i].phase] & (PHASE_POWDER)) == (PHASE_POWDER))
// 			{
// 				unsigned int neighbours[] =
// 				{
// 					squareBelow - 1,
// 					squareBelow,
// 					squareBelow + 1
// 				};

// 				for (uint8_t k = 0; k < 1; ++k) // instead of checking all of them every turn, you can check half or less and it doesn't make a difference for gameplay.
// 				{
// 					uint8_t index = extremelyFastNumberFromZeroTo(2);
// 					chemistry(i, neighbours[index]);
// 					if ((phase_grid[neighbours[index]] & (PHASE_VACUUM)) == (PHASE_VACUUM))
// 					{
// 						swap(i, neighbours[index]);
// 					}
// 				}
// 			}

// 			// movement instructions for LIQUIDS
// 			else if ((grid[i].phase] & (PHASE_LIQUID)) == (PHASE_LIQUID))
// 			{
// 				unsigned int neighbours[] =
// 				{
// 					squareBelow - 1,
// 					squareBelow,
// 					squareBelow + 1,
// 					i - 1,
// 					i + 1
// 				};

// 				for (uint8_t k = 0; k < 2; ++k)
// 				{
// 					uint8_t index = extremelyFastNumberFromZeroTo(4);
// 					chemistry(i, neighbours[index]) ;
// 					if ((phase_grid[neighbours[index]] & (PHASE_VACUUM)) == (PHASE_VACUUM))
// 					{
// 						swap(i, neighbours[index]);
// 					}
// 				}
// 			}

// 			// movement instructions for GASES
// 			else if ((grid[i].phase] & (PHASE_GAS)) == (PHASE_GAS))
// 			{
// 				unsigned int squareAbove = i + sizeX;
// 				unsigned int neighbours[] =
// 				{
// 					squareBelow - 1,
// 					squareBelow,
// 					squareBelow + 1,
// 					i - 1,
// 					i + 1,
// 					squareAbove - 1,
// 					squareAbove,
// 					squareAbove + 1
// 				};

// 				for (uint8_t j = 0; j < 3; ++j)
// 				{
// 					uint8_t index = extremelyFastNumberFromZeroTo(7);
// 					chemistry(i, neighbours[index]);
// 					if ((phase_grid[neighbours[index]] & (PHASE_VACUUM)) == (PHASE_VACUUM))
// 					{
// 						swap(i, neighbours[index]);
// 					}
// 				}
// 			}
// 		}
// 	}
// }

void chooseColor ( Color * colorToUse, unsigned int index )
{
	if ((grid[index].material & (MATERIAL_VACUUM))     == (MATERIAL_VACUUM))       {    *colorToUse = color_black;  return; }
	if ((grid[index].material & (MATERIAL_WATER))      == (MATERIAL_WATER))       {    *colorToUse = color_lightblue;  return; }
	if ((grid[index].material & (MATERIAL_QUARTZ))     == (MATERIAL_QUARTZ))       {    *colorToUse = color_lightgrey;  return; }
	if ((grid[index].material & (MATERIAL_AMPHIBOLE))  == (MATERIAL_AMPHIBOLE))       {    *colorToUse = color_grey;  return; }
	if ((grid[index].material & (MATERIAL_OLIVINE))    == (MATERIAL_OLIVINE))       {    *colorToUse = color_darkgrey;  return; }
	if ((grid[index].material & (MATERIAL_GOLD))       == (MATERIAL_GOLD))       {    *colorToUse = color_yellow;  return; }
}

void setPointSize (unsigned int pointSize)
{

	glPointSize(pointSize);
}

void thread_graphics()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif
	preDraw();

	unsigned int nVertsToRenderThisTurn = 1 * totalSize;
	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

	glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, colorGrid, GL_DYNAMIC_DRAW );
	glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

	postDraw();

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_graphics " << elapsed.count() << " microseconds." << std::endl;
#endif
}


