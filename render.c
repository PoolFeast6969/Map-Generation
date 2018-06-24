/* lmao */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "SDL2/SDL.h"

// The compliler needs to know that this function exists before it calls it, or something like that
int generate_terrain (int size, float scaling, float z_layer, float **height);

int main(){
    //
    // Init
    //

    SDL_Init( SDL_INIT_VIDEO );
    SDL_Window *window = SDL_CreateWindow("planes but with less detail",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,1000,1000,SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    
    //
    // Terrain Heights Generation
    //

    int terrain_size = 100;

    // Make array for the terrain generator to fill (a texture i guess)
    // Allocating memory for the matrix which will store the altitudes
    // Allocate the first dimension as an array of float pointers
    float **height = malloc(sizeof(float*)*terrain_size);
    // Allocate each float pointer as an array of actual floats
    for (int i=0; i<terrain_size; i++) {
        height[i] = malloc(sizeof(float)*terrain_size);
    }

    // Run terrain generation
    generate_terrain(terrain_size, 4.0, 1.0, height);

    //
    // Background
    //
    
    struct background_layer {
        int distance; // Distance from view
        double density; // Cloud thickness
        int shadow_offset[2];
        double position[2];
        double last_update_time[2]; // When it was last redrawn
        SDL_Texture* texture; // Texture in video memory
        Uint32 *pixels; // Pixel data in normal memory
    };
    
    // Set pixel format to make the textures in
    Uint32 pixel_format_id = SDL_PIXELFORMAT_RGBA32; // A 32 bit format that lets the OS decide specifics
    SDL_PixelFormat *pixel_format = SDL_AllocFormat(pixel_format_id); // Get the actual format object from its ID
    
    
    // Instantiate the land layer struct
    struct background_layer land = {
        .distance = 500,
        .texture = 0,
        .pixels = 0
    };
    
    // Creates an array of cloud layers with their height and density already set, and the land already in the background
    struct background_layer background_layers[] = {land,{400,8,{-15,25}},{340,8,{-15,25}},{100, 12,{-25,35}}};
    
    int background_layer_amount = sizeof(background_layers) / sizeof(struct background_layer);
    
    // Convert height map to clouds
    for (int i = 1; i < background_layer_amount; i++) {
        // Run terrain generation
        generate_terrain(terrain_size, 1.5 ,1, height);        // Create a cloud pixel map from the height map provided
        Uint32 pixels[terrain_size][terrain_size];
        for(int columns=0; columns < terrain_size; columns++) {
            for(int rows=0; rows < terrain_size; rows++) {
                if ((height)[columns][rows] < background_layers[i].density) {
                    // Draw Transparent
                    pixels[columns][rows] = SDL_MapRGBA(pixel_format, 0, 0, 0, 0);
                } else if (background_layers[i].density <= height[columns][rows]){
                    // Draw Greyscale
                    int greyness = (1-(height[columns][rows]-background_layers[i].density)/16) * 225 + 30;
                    pixels[columns][rows] = SDL_MapRGBA(pixel_format, greyness, greyness, greyness, 255);
                }
            }
        }
        background_layers[i].pixels = *pixels; // Add the pixels to the struct for this layer
        SDL_Texture *cloud_complete_texture = SDL_CreateTexture(renderer, pixel_format_id, SDL_TEXTUREACCESS_TARGET, terrain_size, terrain_size);
        SDL_SetRenderTarget(renderer, cloud_complete_texture);
        SDL_Surface *cloud_surface = SDL_CreateRGBSurfaceWithFormatFrom(background_layers[i].pixels, terrain_size, terrain_size, 0,terrain_size * sizeof(Uint32), pixel_format_id);
        SDL_Texture *cloud_texture = SDL_CreateTextureFromSurface(renderer,cloud_surface);
        SDL_Rect cloud_dest = {0,0,terrain_size,terrain_size};
        // Create cloud shadows by blacking out the cloud texture
        SDL_Texture *cloud_shadow_texture = SDL_CreateTextureFromSurface(renderer,cloud_surface);
        SDL_SetTextureColorMod(cloud_shadow_texture, 30, 30, 30);
        SDL_SetTextureAlphaMod(cloud_shadow_texture, 140);
        SDL_Rect cloud_shadow_dest = {background_layers[i].shadow_offset[0],background_layers[i].shadow_offset[1],terrain_size,terrain_size};
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
    
    // Avaliable: {"spitfire","me109","avro_lancaster","mosquito","dehavalland_vampire","horten_229","p38_lightening"}
    
    const char *sprite_names[] = {"spitfire"};
    
    int sprite_amount = sizeof(sprite_names) / sizeof(char *);
    
    struct sprite {
        SDL_Surface* surface;
        SDL_Texture* texture;
        double position[2];
        double velocity[2];
        double last_update_time[2];
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
        // Convert to the pixel format we're using
        SDL_ConvertSurface(sprites[i].surface, pixel_format, 0);
        // Create texture
        sprites[i].texture = SDL_CreateTextureFromSurface(renderer, sprites[i].surface);
        // Delete surface or something
        sprites[i].position[1] = 50;
        sprites[i].position[0] = 50;
        sprites[i].velocity[0] = 0;
        sprites[i].velocity[1] = 0;
    }

    //
    // Loop
    //
    
    // yep all these pixels ae the same size
    double pixel_scaling = 10;

    double view_velocity[] = {0,0};
    SDL_Event window_event;
    int window_h;
    int window_w;

    bool run = true;
    int velocity = 1;
    int left_speed = 0; 
    int right_speed = 0; 
    int up_speed = 0; 
    int down_speed = 0; 

    double z_layer = 10;
    // Main loop that updates at vsync in case we ever need animations
    while (run) {
        while (SDL_PollEvent(&window_event)){
            switch( window_event.type ){
                case SDL_QUIT:
                    run = false;
                    break;
                    // Look for a keypress
                case SDL_KEYDOWN:
                    // Check the SDLKey values and move change the coords
                    switch( window_event.key.keysym.sym ){
                        case SDLK_LEFT:
                            left_speed = velocity;
                            break; 
                        case SDLK_RIGHT:
                            right_speed = velocity;
                            break; 
                        case SDLK_UP:
                            up_speed = velocity;
                            break; 
                        case SDLK_DOWN:
                            down_speed = velocity;
                            break; 
                        default:    
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    // Check the SDLKey values and move change the coords
                    switch( window_event.key.keysym.sym ){
                        case SDLK_LEFT:
                            left_speed = 0;
                            break;
                        case SDLK_RIGHT:
                            right_speed = 0;
                            break;
                        case SDLK_UP:
                            up_speed = 0;
                            break;
                        case SDLK_DOWN:
                            down_speed = 0;
                            break;
                        default:    
                            break;
                    }
                    break;
            }
        }
        
        sprites[0].velocity[0] = right_speed - left_speed;
        sprites[0].velocity[1] = down_speed - up_speed; 

        //Stop you from getting speeding tickets on diagnols 
        if (sprites[0].velocity[0] != 0 && sprites[0].velocity[1] != 0){
            if ((sprites[0].velocity[0] == velocity || sprites[0].velocity[0] == -velocity ) && (sprites[0].velocity[1] == velocity || sprites[0].velocity[1] == -velocity)){
                sprites[0].velocity[0] = sprites[0].velocity[0]*0.5; 
                sprites[0].velocity[1] = sprites[0].velocity[1]*0.5;
            }   
        }

        //Stop you leaving the map 
        //if (sprites[0].position[0] <= 0){
        //    sprites[0].position[0] = 0; 
        //    if (sprites[0].velocity[0] < 0){
        //        sprites[0].velocity[0] = 0;
        //    }                  
        //}        

        generate_terrain(terrain_size, 4.0, z_layer, height);
        z_layer = z_layer + .01;
        // Convert height map to pixel color map
        Uint32 land_pixels[terrain_size][terrain_size]; // Create the array to store the pixels
        for(int columns=0; columns < terrain_size; columns++) {
            for(int rows=0; rows < terrain_size; rows++) {
                if (height[columns][rows] <= 0.5) {
                    // Draw Water
                    int blueness = height[columns][rows] * 120+ 50;
                    land_pixels[columns][rows] = SDL_MapRGBA(pixel_format, 50, 120*(height[columns][rows]/7), blueness, 255);
                } else if (height[columns][rows] > 0.5) {
                    // Draw Forest
                    int greenness = (height[columns][rows]-0.5) * 130+ 95;
                    land_pixels[columns][rows] = SDL_MapRGBA(pixel_format, 0, greenness, 0, 255);
                }
            }
        }
            
        // Put the land image into a texture
        SDL_Surface *land_surface = SDL_CreateRGBSurfaceWithFormatFrom(land_pixels, terrain_size, terrain_size, 0,terrain_size * sizeof(Uint32), pixel_format_id); // Through a surface
        SDL_Texture *land_texture = SDL_CreateTextureFromSurface(renderer,land_surface);

        background_layers[0].texture = SDL_CreateTextureFromSurface(renderer,land_surface);

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
            SDL_Rect new_position = {background_layers[i].position[0],background_layers[i].position[1], terrain_size*pixel_scaling, terrain_size*pixel_scaling};
            // Add to frame
            SDL_RenderCopy(renderer,background_layers[i].texture,NULL,&new_position);
        }
        
        // Draw each of the sprites with the correct position
        for (int i = 0; i < sprite_amount; i++) {
            for (int a = 0; a < 2 ; a++) {
                double time_since_update = SDL_GetTicks() - sprites[i].last_update_time[a];
                sprites[i].position[a] = sprites[i].position[a] + sprites[i].velocity[a] * time_since_update;
                sprites[i].last_update_time[a] = SDL_GetTicks();
            }
            SDL_Rect dest = {sprites[i].position[0],sprites[i].position[1],sprites[i].surface->w*pixel_scaling,sprites[i].surface->h*pixel_scaling};
            SDL_RenderCopyEx(renderer, sprites[i].texture, NULL, &dest, 0, NULL, SDL_FLIP_NONE);
        }
        
        // Show the completed frame and wait for vsync
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(background_layers[0].texture);
        SDL_RenderClear(renderer);

    }
    
    //
    // Clean Up
    //
    free(height);    
    // Destroy things
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    //Quit SDL
    SDL_Quit();
    return 0;
}
