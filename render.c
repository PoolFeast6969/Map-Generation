/* lmao */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

int render(long double height[], int size){
    //Start SDL
    SDL_Init( SDL_INIT_VIDEO );
    
    SDL_Window *window = SDL_CreateWindow(
        "get ",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1000, 1000,
        SDL_WINDOW_RESIZABLE
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC
    );
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    //SDL_RenderSetLogicalSize(renderer, size, size);
    //SDL_RenderSetScale(renderer, 4,4);
    
    SDL_Texture *land_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB332,
        SDL_TEXTUREACCESS_STATIC,
        size, size
    );
    
    SDL_Texture *cloud_shadow_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB332,
        SDL_TEXTUREACCESS_STREAMING,
        size, size
    );

    
    SDL_Event event;
    int width;
    
    // Convert height map to pixel color map
    Uint32 format;
    SDL_PixelFormat *fmt;
    SDL_QueryTexture(land_texture, &format, NULL, NULL, &width);
    fmt = SDL_AllocFormat(format);
    
    Uint8 land_pixels[size*size];
    for(int columns=0; columns < size*size; columns++) {
        if (height[columns] == 0) {
            // Water
            land_pixels[columns] = SDL_MapRGB(fmt, 60, 60, 255);
        }
        else if (height[columns] > 0) {
            if (height[columns] < 0) {
                // Sand
                land_pixels[columns] = SDL_MapRGB(fmt, 255, 255, 150);
            } else if (height[columns] < 5) {
                // Forest
                int greenness = (height[columns])/7 * 200;
                land_pixels[columns] = SDL_MapRGB(fmt, 0, greenness, 0);
            } else if (height[columns] >= 5) {
                // Snow
                land_pixels[columns] = SDL_MapRGB(fmt, 100, 135, 135);
            }
        }
    }
    
    // Put the background into a texture
    SDL_UpdateTexture(land_texture, NULL, land_pixels, width * sizeof(Uint8));
    
    // Create some clouds
    Uint8 cloud_pixels[size*size];
    for(int columns=0; columns < size*size; columns++) {
        if (height[columns] < 3) {
            // Transparent
            cloud_pixels[columns] = SDL_MapRGB(fmt, 0, 0, 0);
        } else {
            // Greyscale
            int greyness = (3 - height[columns])/10 * 255;
            cloud_pixels[columns] = SDL_MapRGB(fmt, greyness, greyness, greyness);
        }
        //printf("%c ",cloud_pixels[columns]);
    }
    SDL_Surface *cloud_surface = SDL_CreateRGBSurfaceWithFormatFrom(cloud_pixels, size, size, 0,width * sizeof(Uint8), format);
    SDL_SetColorKey(cloud_surface, SDL_TRUE, SDL_MapRGB(fmt, 0, 0, 0));
    SDL_Texture *cloud_texture = SDL_CreateTextureFromSurface(renderer,cloud_surface);
    SDL_FreeSurface( cloud_surface );
    
    // Something
    SDL_FreeFormat(fmt);

    SDL_Rect land_source = {0,0,size,size};
    SDL_Rect land_dest = {0,0,size,size};
    
    SDL_Rect cloud_source = {0,0,size,size};
    SDL_Rect cloud_dest = {0,0,size,size};
    
    // Main loop that updates at vsync in case we ever need animations
    while (true) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }
        // Stop the thing from stretching the background
        //SDL_GetRendererOutputSize(renderer, &land_source.w, &land_source.h);
        
        // Add the land to the frame
        SDL_RenderCopy(renderer,land_texture,&land_source,&land_dest);
        // Add the clouds to the frame
        SDL_RenderCopyEx(renderer, cloud_texture, &cloud_source, &cloud_dest, 90, NULL, SDL_FLIP_VERTICAL);
        // Show the completed frame and wait for vsync
        SDL_RenderPresent(renderer);
        
        // Move clouds really badly
        cloud_dest.x++;
    }
    // Destroy things
    SDL_DestroyTexture(land_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    //Quit SDL
    SDL_Quit();
    return 0;
}

