/* lmao */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "SDL2/SDL.h"


int render(long double height[], int size){
    //Start SDL
    SDL_Init( SDL_INIT_VIDEO );
    SDL_Window *window = SDL_CreateWindow(
        "get ",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1000, 1000,
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_OPENGL
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC |
        SDL_RENDERER_TARGETTEXTURE
    );
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    SDL_Event event;
    int width;

    Uint32 format;
    SDL_PixelFormat *fmt;
    SDL_Texture *land_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STATIC, size, size);
    SDL_QueryTexture(land_texture, &format, NULL, NULL, &width);
    fmt = SDL_AllocFormat(format);

    // Convert height map to pixel color map
    Uint8 land_pixels[size*size];
    for(int columns=0; columns < size*size; columns++) {
        if (height[columns] == 0) {
            // Water
            land_pixels[columns] = SDL_MapRGB(fmt, 50, 120, 200);
        }
        else if (height[columns] > 0) {
            if (height[columns] < 0) {
                // Sand
                land_pixels[columns] = SDL_MapRGB(fmt, 255, 255, 150);
            } else if (height[columns] < 7) {
                // Forest
                int greenness = height[columns]/7 * 130 + 90;

                land_pixels[columns] = SDL_MapRGB(fmt, 0, greenness, 0);
            } else if (height[columns] >= 7) {
                // Snow
                land_pixels[columns] = SDL_MapRGB(fmt, 0, 100, 0);
            }
        }
    }
    // Put the background into a texture
    SDL_UpdateTexture(land_texture, NULL, land_pixels, width * sizeof(Uint8));
    SDL_Rect land_source = {0,0,size,size};
    SDL_Rect land_dest = {0,0,size,size};


    // Create some clouds
    Uint8 cloud_pixels[size*size];
    for(int columns=0; columns < size*size; columns++) {
        if (height[columns] < 3) {
            // Transparent
            cloud_pixels[columns] = SDL_MapRGB(fmt, 0, 0, 0);
        } else if (3 <= height[columns]){
            // Greyscale
            int greyness = (1-(height[columns]-3)/16) * 225 + 30;
            cloud_pixels[columns] = SDL_MapRGB(fmt, greyness, greyness, greyness);
        }
    }
    SDL_Texture *cloud_complete_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_TARGET, size, size);
    SDL_SetRenderTarget(renderer, cloud_complete_texture);
    SDL_Surface *cloud_surface = SDL_CreateRGBSurfaceWithFormatFrom(cloud_pixels, size, size, 0,width * sizeof(Uint8), format);
    SDL_SetColorKey(cloud_surface, SDL_TRUE, SDL_MapRGB(fmt, 0, 0, 0));
    SDL_Texture *cloud_texture = SDL_CreateTextureFromSurface(renderer,cloud_surface);
    SDL_Rect cloud_source = {0,0,size,size};
    SDL_Rect cloud_dest = {0,0,size,size};
    // Create cloud shadows by blacking out the cloud texture
    SDL_Texture *cloud_shadow_texture = SDL_CreateTextureFromSurface(renderer,cloud_surface);
    SDL_SetTextureColorMod(cloud_shadow_texture, 30, 30, 30);
    SDL_SetTextureAlphaMod(cloud_shadow_texture, 140);
    SDL_Rect cloud_shadow_source = {0,0,size,size};
    SDL_Rect cloud_shadow_dest = {-15,25,size,size};
    // Add the cloud shadows to the frame
    SDL_SetTextureBlendMode(cloud_complete_texture, SDL_BLENDMODE_BLEND);
    SDL_RenderCopyEx(renderer, cloud_shadow_texture, &cloud_shadow_source, &cloud_shadow_dest, 90, NULL, SDL_FLIP_NONE);
    // Add the clouds to the frame
    SDL_RenderCopyEx(renderer, cloud_texture, &cloud_source, &cloud_dest, 90, NULL, SDL_FLIP_NONE);
    // Something
    SDL_SetRenderTarget(renderer, NULL);
    
    const char *sprite_names[] = {"spitfire"};
    
    int sprite_amount = sizeof(sprite_names) / sizeof(const char *);
    
    struct sprite {
        SDL_Surface* surface;
        SDL_Texture* texture;
        SDL_Rect dest;
        SDL_Rect source;
        char filename[];
    };

    struct sprite sprites[sprite_amount];
    
    // Load sprites
    for (int i = 0; i < sprite_amount; i++) {
        // Organise the filename extension
        strcpy(sprites[i].filename, sprite_names[i]);
        strcat(sprites[i].filename, ".bmp");
        // Load the sprite from the image
        sprites[i].surface = SDL_LoadBMP(sprites[i].filename);
        // Make white areas transparent
        SDL_SetColorKey(sprites[i].surface, SDL_TRUE, SDL_MapRGB(sprites[i].surface->format, 255, 255, 255));
        // Convert to 8 bit
        SDL_ConvertSurface(sprites[i].surface, fmt, 0);
        // Create texture
        sprites[i].texture = SDL_CreateTextureFromSurface(renderer, sprites[i].surface);
        // Delete surface or something
        SDL_FreeSurface(sprites[i].surface);
    }
    
    int window_h;
    int window_w;
    SDL_GetRendererOutputSize(renderer, &window_w, &window_h);
    
    cloud_dest.x = -window_w*2;
    land_dest.w = window_w*3;
    land_dest.h = window_w*3;
    
    cloud_dest.w = window_w*3;
    cloud_dest.h = window_w*3;
    
    land_source.w = window_w*3;
    
    sprites[0].dest.x = window_w/2-window_w/4/2;
    sprites[0].dest.y = 2*window_h/3;
    
    sprites[0].dest.w =window_w/4;
    sprites[0].dest.h =window_w/4;
    
    float cloud_speed = 0.005; // In pixels per millisecond

    // Main loop that updates at vsync in case we ever need animations
    Uint32 last_update_time = SDL_GetTicks();
    while (true) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
            window_w = event.window.data1;
            window_h = event.window.data2;
            
            cloud_dest.x = -window_w*2;
            land_dest.w = window_w*3;
            land_dest.h = window_w*3;
            
            cloud_dest.w = window_w*3;
            cloud_dest.h = window_w*3;
            
            land_source.w = window_w*3;
            // 0 = player/spitfire
            sprites[0].dest.x = window_w/2-window_w/4/2;
            sprites[0].dest.y = 2*window_h/3;
            
            sprites[0].dest.w =window_w/4;
            sprites[0].dest.h =window_w/4;
        }

        Uint32 time_since_update = SDL_GetTicks() - last_update_time;
        if ((int)(time_since_update * cloud_speed) != 0) {
            cloud_dest.x = cloud_dest.x + (int)(time_since_update * cloud_speed);
            last_update_time = SDL_GetTicks();
        }
        //SDL_RenderClear(renderer);
        // Add the land to the frame
        SDL_RenderCopy(renderer,land_texture,&land_source,&land_dest);
        // Add the clouds
        SDL_RenderCopy(renderer,cloud_complete_texture, &cloud_source, &cloud_dest);
        // Add plane
        SDL_RenderCopyEx(renderer, sprites[0].texture, NULL, &sprites[0].dest, 0, NULL, SDL_FLIP_NONE);
        // Show the completed frame and wait for vsync
        SDL_RenderPresent(renderer);
    }
    // Destroy things
    SDL_DestroyTexture(land_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    //Quit SDL
    SDL_Quit();
    return 0;
}
