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
	shutdownGraphics();
	SDL_Quit();
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



			case SDLK_b:
				setNeutralTemp();
				break;

			case SDLK_r:
				sendLifeToBackground();
				break;

			case SDLK_n:
				insertRandomSeed();
				break;

			case SDLK_t:
				toggleEnergyGridDisplay();
				break;



			case SDLK_s:
				// save();
				flagSave = true;
				break;



			case SDLK_l:
				// load();
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
				toggleErodingRain();
				break;





			case SDLK_p:
				togglePause();
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
				// printf("x %i, y %i", mouseX, mouseY);

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


		if (!crudOps)
		{


			boost::thread t99{ thread_temperature2 };

			boost::thread t2{ thread_physics };

			boost::thread t6{ thread_interface };

			boost::thread t7 { thread_life};

			boost::thread t8 { thread_seeds};

			// graphics only seems to work in this thread, so we can just say that's what this thread is for.

			thread_graphics();

			t8.join();

			t7.join();

			t6.join();

			t2.join();

			t99.join();

		}

		// t100.join();


		// these operations can't be done while threading is happening. wait for the turn to finish, then use them.
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

		if (flagQuit)
		{
			crudOps = true;
			flagQuit = false;
			return 0;
		}
	}
	return 0;
}
