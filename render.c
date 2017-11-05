/* lmao */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

int render(long double height[],int size){
    //Start SDL
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_Init( SDL_INIT_VIDEO );
    
    SDL_Window *window = SDL_CreateWindow(
        "get shit on",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        700, 700,
        SDL_WINDOW_RESIZABLE
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC
    );
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB332,
        SDL_TEXTUREACCESS_STREAMING,
        size, size
    );
    SDL_Event event;
    void *pixels;
    int pitch;
    
    // Convert height map to pixel color map
    Uint32 format;
    SDL_PixelFormat *fmt;
    SDL_QueryTexture(texture, &format, NULL, NULL, NULL);
    fmt = SDL_AllocFormat(format);
    
    Uint8 npixels[size*size];
    for(int columns=0; columns < size*size; columns++) {
        if (height[columns] == 0) {
            // Water
            npixels[columns] = SDL_MapRGB(fmt, 60, 60, 255);
        }
        else if (height[columns] > 0) {
            if (height[columns] < 0.7) {
                // Sand
                npixels[columns] = SDL_MapRGB(fmt, 255, 255, 150);
            } else if (height[columns] < 3.5) {
                // Forest
                int greenness = (height[columns] - 0.5)/6 * 255;
                npixels[columns] = SDL_MapRGB(fmt, 50, greenness, 50);
            } else if (height[columns] >= 3.7) {
                // Snow
                npixels[columns] = SDL_MapRGB(fmt, 230, 230, 230);
            }
        }
    }
    
    SDL_FreeFormat(fmt);
    
    // Main loop that updates at vsync in case we ever need animations
    while (true) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }
        // Modify the texture
        // Get a pixel representation that is writable
        SDL_LockTexture(texture, NULL, &pixels, &pitch);
        // Turn the void pointer into a 8 bit integer array because that should be what it is
        memcpy(pixels, npixels, size*size);
        SDL_UnlockTexture(texture);
        // This renders the texture?
        SDL_RenderCopy(renderer,texture,NULL,NULL);
        // This present thing waits until vsync and tells the renderer to show the frame
        SDL_RenderPresent(renderer);
    }
    // Destroy things
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    //Quit SDL
    SDL_Quit();
    return 0;
}

