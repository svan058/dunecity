#include <SDL/SDL.h>
#include "SDLMain.h"

SDL_Surface* screen;

int main(int argc, char * argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
	
    screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen == NULL) {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        exit(1);
    }
	
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
            {
                done = true;
                break;
            }
            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    done = true;
                break;
            }
        }
    }
	SDL_Quit();
	
    return 0;
}