#include "deepgarden.h"
#include "deepgarden_graphics.h"
#include <ctime>
#include <chrono>
#include <iostream>

bool paused = false;
bool flagQuit = false;
int mouseX;
int mouseY;
float panSpeed = 0.25f;
unsigned int pixelSize = 3;

bool crudOps = false;

bool flagSave = false;
bool flagLoad = false;

unsigned int loadCooldown = 0;

void quit ()
{
	flagQuit = true;
}

void togglePause ()
{
	paused = !paused;
}

void thread_interface()
{
	SDL_Event event;
	while ( SDL_PollEvent( &event ) )
	{
		switch ( event.type )
		{

		case SDL_KEYDOWN:
		{
			switch ( event.key.keysym.sym )
			{

			case SDLK_LEFT:
				viewPanSetpointX = viewPanSetpointX - (panSpeed * viewZoomSetpoint  );
				break;
			case SDLK_RIGHT:
				viewPanSetpointX = viewPanSetpointX + (panSpeed * viewZoomSetpoint  );
				break;
			case SDLK_UP:
				viewPanSetpointY = viewPanSetpointY + (panSpeed * viewZoomSetpoint  );
				break;
			case SDLK_DOWN:
				viewPanSetpointY = viewPanSetpointY - (panSpeed * viewZoomSetpoint  );
				break;
			case SDLK_EQUALS:
				viewZoomSetpoint = viewZoomSetpoint * 0.9f;
				break;
			case SDLK_MINUS:
				viewZoomSetpoint = viewZoomSetpoint * 1.1f;
				break;

			case SDLK_h:
				heatEverything();
				break;

			case SDLK_c:
				coolEverything();
				break;

			case SDLK_m:
				setEverythingHot();
				break;

			case SDLK_y:
				clearGrids() ;
				break;

			case SDLK_b:
				setNeutralTemp();
				break;

			case SDLK_r:
				// sendLifeToBackground();
				break;

			case SDLK_n:
				insertRandomSeed();
				break;

			case SDLK_t:
				toggleEnergyGridDisplay();
				break;

			case SDLK_s:
				flagSave = true;
				break;

			case SDLK_l:
				flagLoad = true;
				break;

			case SDLK_LEFTBRACKET:
				increaseLampBrightness();
				break;

			case SDLK_RIGHTBRACKET:
				decreaseLampBrightness();
				break;

			case SDLK_v:
				drawRandomLandscape();
				break;

			case SDLK_e:
				// toggleErodingRain();
				break;

			case SDLK_x:
				manualErode();
				break;

			case SDLK_k:
				// createRandomWorld();
				createWorld( WORLD_GONQUIN);
				break;

			case SDLK_p:
				togglePause();
				break;

			case SDLK_o:
				clearLiquids();
				break;

			case SDLK_i:
				clearGases();
				break;

			case SDLK_g:
				insertRandomAnimal();
				break;

			case SDLK_u:
				eraseFallenSeeds();
				break;

			case SDLK_j:
				dropAllSeeds();
				break;

			case SDLK_q:
				eraseAllLife();
				break;

			case SDLK_f:
				clearAllPressureVelocity();
				break;


			case SDLK_ESCAPE:
				quit();
			}
			break;
		}

		case SDL_MOUSEBUTTONDOWN:
		{
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
			{
				unsigned int mouseusu = mouseX;
				unsigned int mousewewe = mouseY;
				setExtremeTempPoint (mouseusu, mousewewe);
				break;
			}
			}
			break;
		}

		case SDL_MOUSEMOTION:
		{
			mouseX = event.motion.x;
			mouseY = event.motion.y;
		}
		}
	}
}

// plant drawing runs in variable time but is always slower than everything else. this threading arrangement lets it run forever, but at its own pace.
void thread_supervisor_plantDrawing()
{
	while (true)
	{
		boost::thread t101 { thread_plantDrawing};
		t101.join();
	}
}

int main( int argc, char * argv[] )
{
	setupGraphics();
	initialize();
	boost::thread t100 { thread_supervisor_plantDrawing};
	for ( ;; )
	{
		// start all the threads and then wait for them to finish.
		// start threads in order of chunkiest to least chunky.
		if (!crudOps && !paused)
		{
			// boost::thread t99{ thread_temperature2 };

			boost::thread t3{ thread_weather };
			boost::thread t454{ thread_materialPhysics };
			boost::thread t2{ thread_physics };
			boost::thread t10 { thread_animals };
			boost::thread t6{ thread_interface };
			boost::thread t7 { thread_life};
			boost::thread t8 { thread_seeds};

			// graphics only seems to work in this thread, so we can just say that's what this thread is for.
			thread_graphics();

			t8.join();
			t7.join();
			t6.join();
			t10.join();
			t2.join();
			t454.join();
			t3.join();
			// t99.join();
		}
		else
		{
			boost::thread t6{ thread_interface };
			thread_graphics();
			t6.join();
		}

		// these operations can't be done while threading is happening. wait for the turn to finish, then use them.
		for (unsigned int i = 0; i < totalSize; ++i)
		{
			if (isAnimal(i))
			{
				animalCrudOps(i);
			}
		}

		if (flagSave)
		{
			crudOps = true;
			save();
			flagSave = false;
			crudOps = false;
		}

		if (flagLoad && loadCooldown == 0)
		{
			crudOps = true;
			flagLoad = false;
			load();
			crudOps = false;
		}

		if (loadCooldown > 0) {loadCooldown--;}

		if (flagQuit && !flagSave)
		{
			crudOps = true;
			flagQuit = false;

			shutdownGraphics();
			SDL_Quit();

			return 0;
		}
	}
	return 0;
}
