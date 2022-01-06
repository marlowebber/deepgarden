#include "deepgarden.h"
#include "deepgarden_graphics.h"
#include <ctime>
#include <chrono>
#include <iostream>

bool paused = false;
bool flagQuit = false;
int mouseX;
int mouseY;
float panSpeed = 0.5f;
unsigned int pixelSize = 3;

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

			case SDLK_b:
				setNeutralTemp();
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
				break;
			}

			}
			break;
		}
		}
	}
}

int main( int argc, char * argv[] )
{
	setupGraphics();
	initialize();

	for ( ;; )
	{


		// start all the threads and then wait for them to finish.
		// start threads in order of chunkiest to least chunky.

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

		if (flagQuit)
		{
			flagQuit = false;
			return 0;
		}
	}
	return 0;
}
