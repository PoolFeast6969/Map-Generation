/* lmao */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "SDL2/SDL.h"

int render(float height[], int size){
    //
    // Init
    //
    
    SDL_Init( SDL_INIT_VIDEO );
    SDL_Window *window = SDL_CreateWindow("planes but with less detail",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,1000,1000,SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    //
    // Background
    //
    
    struct background_layer {
        int distance;
        double density;
        double position[2];
        double last_update_time[2];
        SDL_Texture* texture;
        Uint8 *pixels;
    };
    
    // Get format to make the textures in
    Uint32 pixel_format_id = SDL_PIXELFORMAT_RGB332;
    SDL_PixelFormat *pixel_format = SDL_AllocFormat(pixel_format_id);
    
    // Convert height map to pixel color map
    Uint8 land_pixels[size*size];
    for(int columns=0; columns < size*size; columns++) {
        if (height[columns] == 0) {
            // Water
            land_pixels[columns] = SDL_MapRGB(pixel_format, 50, 120, 200);
        }
        else if (height[columns] > 0) {
            if (height[columns] < 0) {
                // Sand
                land_pixels[columns] = SDL_MapRGB(pixel_format, 255, 255, 150);
            } else if (height[columns] < 7) {
                // Forest
                int greenness = height[columns]/7 * 130 + 90;
                
                land_pixels[columns] = SDL_MapRGB(pixel_format, 0, greenness, 0);
            } else if (height[columns] >= 7) {
                // Snow
                land_pixels[columns] = SDL_MapRGB(pixel_format, 0, 100, 0);
            }
        }
    }
    
    // Put the background into a surface
    SDL_Surface *land_surface = SDL_CreateRGBSurfaceWithFormatFrom(land_pixels, size, size, 0,size * sizeof(Uint8), pixel_format_id);
    SDL_Texture *land_texture = SDL_CreateTextureFromSurface(renderer,land_surface);
    
    struct background_layer land = {
        .distance = 500,
        .texture = SDL_CreateTextureFromSurface(renderer,land_surface),
        .pixels = land_pixels
    };
    
    // Creates an array of cloud layers with their height and density already set
    struct background_layer background_layers[] = {land,{400,3},{340, 5},{100, 4}};
    
    int background_layer_amount = sizeof(background_layers) / sizeof(struct background_layer);
    
    // Convert height map to clouds
    for (int i = 1; i < background_layer_amount; i++) {
        // Create a cloud pixel map from the height map provided
        Uint8 pixels[size*size];
        background_layers[i].pixels = pixels;
        for(int columns=0; columns < size*size; columns++) {
            if (height[columns] < background_layers[i].density) {
                // Transparent
                background_layers[i].pixels[columns] = SDL_MapRGB(pixel_format, 0, 0, 0);
            } else if (background_layers[i].density <= height[columns]){
                // Greyscale
                int greyness = (1-(height[columns]-background_layers[i].density)/16) * 225 + 30;
                background_layers[i].pixels[columns] = SDL_MapRGB(pixel_format, greyness, greyness, greyness);
            }
        }
        SDL_Texture *cloud_complete_texture = SDL_CreateTexture(renderer, pixel_format_id, SDL_TEXTUREACCESS_TARGET, size, size);
        SDL_SetRenderTarget(renderer, cloud_complete_texture);
        SDL_Surface *cloud_surface = SDL_CreateRGBSurfaceWithFormatFrom(background_layers[i].pixels, size, size, 0,size * sizeof(Uint8), pixel_format_id);
        SDL_SetColorKey(cloud_surface, SDL_TRUE, SDL_MapRGB(pixel_format, 0, 0, 0));
        SDL_Texture *cloud_texture = SDL_CreateTextureFromSurface(renderer,cloud_surface);
        SDL_Rect cloud_dest = {0,0,size,size};
        // Create cloud shadows by blacking out the cloud texture
        SDL_Texture *cloud_shadow_texture = SDL_CreateTextureFromSurface(renderer,cloud_surface);
        SDL_SetTextureColorMod(cloud_shadow_texture, 30, 30, 30);
        SDL_SetTextureAlphaMod(cloud_shadow_texture, 140);
        SDL_Rect cloud_shadow_dest = {-15,25,size,size};
        // Add the cloud shadows to the frame
        SDL_SetTextureBlendMode(cloud_complete_texture, SDL_BLENDMODE_BLEND);
        SDL_RenderCopyEx(renderer, cloud_shadow_texture, NULL, &cloud_shadow_dest, 90, NULL, SDL_FLIP_NONE);
        // Add the clouds to the frame
        SDL_RenderCopyEx(renderer, cloud_texture, NULL, &cloud_dest, 90, NULL, SDL_FLIP_NONE);
        // Something
        SDL_SetRenderTarget(renderer, NULL);
        background_layers[i].texture = cloud_complete_texture;
        SDL_FreeSurface(cloud_surface);
    }
    
    //
    // Sprites
    //
    
    // Avaliable: {"spitfire","me109","avro_lancaster","mosquito"}
    
    const char *sprite_names[] = {"spitfire"};
    
    int sprite_amount = sizeof(sprite_names) / sizeof(char *);
    
    struct sprite {
        SDL_Surface* surface;
        SDL_Texture* texture;
        double *position;
        int size;
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
        // Convert to 8 bit
        SDL_ConvertSurface(sprites[i].surface, pixel_format, 0);
        // Create texture
        sprites[i].texture = SDL_CreateTextureFromSurface(renderer, sprites[i].surface);
        // Delete surface or something
        double position[] = {0,0};
        sprites[i].position = position;
    }

    //
    // Loop
    //
    
    // yep all these pixels are the same size
    double pixel_scaling = 5;
    
    double view_velocity[] = {-12,-12};
    SDL_Event window_event;
    int window_h;
    int window_w;
    
    // Main loop that updates at vsync in case we ever need animations
    while (true) {
        SDL_PollEvent(&window_event);
        if (window_event.type == SDL_QUIT) {
            break;
        }
        // Draw each of the background layers with the correct position
        for (int i = 0; i < background_layer_amount; i++) {
            // Determine if it needs to be have its position changed
            for (int a = 0; a < 2 ; a++) {
                double velocity = view_velocity[a]/background_layers[i].distance;
                double time_since_update = SDL_GetTicks() - background_layers[i].last_update_time[a];
                background_layers[i].position[a] = background_layers[i].position[a] + velocity * time_since_update;
                background_layers[i].last_update_time[a] = SDL_GetTicks();
            }
            SDL_GetRendererOutputSize(renderer, &window_w, &window_h);
            SDL_Rect new_position = {background_layers[i].position[0],background_layers[i].position[1], size*pixel_scaling, size*pixel_scaling};
            // Add to frame
            SDL_RenderCopy(renderer,background_layers[i].texture,NULL,&new_position);
        }
        
        // Draw each of the sprites with the correct position
        for (int i = 0; i < sprite_amount; i++) {
            SDL_Rect dest = {sprites[i].position[0],sprites[i].position[1],sprites[i].surface->w*pixel_scaling,sprites[i].surface->h*pixel_scaling};
            SDL_RenderCopyEx(renderer, sprites[i].texture, NULL, &dest, 135, NULL, SDL_FLIP_NONE);
        }
        
        // Show the completed frame and wait for vsync
        SDL_RenderPresent(renderer);
    }
    
    //
    // Clean Up
    //
    
    // Destroy things
    SDL_DestroyTexture(land_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    //Quit SDL
    SDL_Quit();
    return 0;
}
