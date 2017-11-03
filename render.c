/* lmao */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_Init( SDL_INIT_VIDEO );
    
    SDL_Window *window = SDL_CreateWindow(
        "get shit on",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1024, 768,
        SDL_WINDOW_RESIZABLE
    );
    
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 1024, 768);
    bool quit = false;
    SDL_Event event;
    while (!quit)
    {
        SDL_WaitEvent(&event);
        
        switch (event.type)
        {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                }
        }
    }
    SDL_DestroyRenderer(renderer);
    //Quit SDL
    SDL_Quit();
    return 0;

}

