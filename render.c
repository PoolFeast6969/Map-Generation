/* lmao */
#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>

int main(int argc, char** argv){
    const int size = 1000;
    
    double height[size][size];
    /* Seed the random number generator */
    srand(0);
    
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            height[i][j] = rand();
    
    //Start SDL
    SDL_Init( SDL_INIT_EVERYTHING );
    //The window we'll be rendering to
    SDL_Window* gWindow = NULL;
    //The window renderer
    SDL_Renderer* gRenderer = NULL;
    
    SDL_Window *win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
    
    //Quit SDL
    SDL_Quit();
    return 0;

}

