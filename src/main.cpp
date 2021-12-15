#include "deepgarden.h"
#include "deepgarden_graphics.h"
bool paused = false;
bool flagQuit = false;


int mouseX;
int mouseY;

float panSpeed = 0.5f;







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



int main( int argc, char * argv[] )
{

	setupGraphics();


	initialize();

	for ( ;; )
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

		if (!paused)
		{
			// the gameplay loop
			// deepgardenLoop();
		}

		// graphics loop
		deepgardenGraphics();



		 if (flagQuit) 
		 {
            flagQuit = false;
            return 0;
        }



	}

	return 0;
}
