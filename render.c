/* lmao */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "SDL2/SDL.h"

// The compliler needs to know that this function exists before it calls it, or something like that
int generate_terrain (int size, float scaling, float x_layer, float y_layer, float z_layer, float **height);

struct terrain_layer {
    int start_color[4];
    int end_color[4];
    float end_height;
    float start_height;
};

int get_terrain_pixels(Uint32 *pixels, int pixel_amount, struct terrain_layer layer, float **height, SDL_PixelFormat *pixel_format) {
    // Convert height map to pixel color map
    for(int x=0; x < pixel_amount; x++) {
        for(int y=0; y < pixel_amount; y++) {
                if (layer.start_height <= height[x][y] && layer.end_height >= height[x][y]) { // the layer of interest
                    float pixel_color[4]; 
                    for(int color=0; color <= 3; color++) {
                        // linearly interpolate between the two end colors
                        pixel_color[color] = layer.start_color[color] + ((layer.start_color[color] - layer.end_color[color])*(height[x][y]-layer.start_height))/(layer.start_height - layer.end_height);
                    }
                    pixels[x + y*pixel_amount] = SDL_MapRGBA(pixel_format,pixel_color[0],pixel_color[1],pixel_color[2],pixel_color[3]);
            }
        }
    }
    return 0;
}

int set_shadow(SDL_Texture* shadow_texture, float lightness) {
    // Create cloud shadows by blacking out the cloud texture
    SDL_SetTextureColorMod(shadow_texture, lightness, lightness, lightness);
    SDL_SetTextureAlphaMod(shadow_texture, lightness);
    return 0;
}

int main() {
    //
    // Init
    //

    printf("Starting\n");

    SDL_Init( SDL_INIT_VIDEO );
    SDL_Window *window = SDL_CreateWindow("planes but with less detail",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,1000,1000,SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    // Set pixel format to make the textures in
    Uint32 pixel_format_id = SDL_PIXELFORMAT_RGBA32; // A 32 bit format that lets the OS decide specifics
    SDL_PixelFormat *pixel_format = SDL_AllocFormat(pixel_format_id); // Get the actual format object from its ID
    
    //
    // Terrain Heights Array
    //

    const int terrain_size = 100;

    // Make array for the terrain generator to fill (a texture i guess)
    // Allocating memory for the matrix which will store the altitudes
    // Allocate the first dimension as an array of float pointers
    float **height = malloc(sizeof(float*)*terrain_size);
    // Allocate each float pointer as an array of actual floats
    for (int i=0; i<terrain_size; i++) {
        height[i] = malloc(sizeof(float)*terrain_size);
    }

    generate_terrain(terrain_size, 0, 0, 4.0, .02, height); // Get a terrain height map
        //}

    //
    // Background
    //

    struct terrain_layer biome[] = {
        {
            .start_color = {36,36,85,255}, // Deep water
            .end_color = {36,109,170,255}, // Shallow water
            .start_height = -3, // Minimum value
            .end_height = 0, // Minimum value
        },{
            .start_color = {148,148,123,255}, // low sand
            .end_color = {148,148,123,255}, // high sand
            .start_height = 0, // Minimum value
            .end_height = 0.11, // halfway
        },{
            .start_color = {0,109,0,255}, // low land
            .end_color = {0,218,0,255}, // high land
            .start_height = 0.11, // Minimum value
            .end_height = 2, // halfway
        }
    };

    Uint32 *land_pixels = malloc(sizeof(Uint32)*terrain_size*terrain_size);
    for(int layer=0; layer < sizeof(biome) / sizeof(struct terrain_layer); layer++) {
        get_terrain_pixels(land_pixels, terrain_size, biome[layer], height, pixel_format);
    }

    // Put the land image into a texture
    SDL_Surface *land_surface = SDL_CreateRGBSurfaceWithFormatFrom(land_pixels, terrain_size, terrain_size, 32,terrain_size * sizeof(Uint32), pixel_format_id); // Through a surface 
    
    struct background_layer {
        int distance; // Distance from view
        double density; // Cloud thickness
        int shadow_offset[2];
        double position[2];
        double last_update_time[2]; // When each position was last updated
        SDL_Texture* texture; // Texture in video memory
        Uint32 *pixels; // Pixel data in normal memory
    };

    // Instantiate the land layer struct
    struct background_layer land = {
        .distance = 500,
        .texture = SDL_CreateTextureFromSurface(renderer,land_surface),
        .pixels = land_pixels
    };

    SDL_FreeSurface(land_surface);

    // Creates an array of cloud layers with their height and density already set, and the land already in the background
    struct background_layer background_layers[] = {land,{400,}};
    
    int background_layer_amount = sizeof(background_layers) / sizeof(struct background_layer);
    printf("There are %i background layer(s)\n",background_layer_amount);

    // Convert height map to clouds
    struct terrain_layer clouds = {
        .start_color = {180,218,241,50}, // Deep water
        .end_color = {255,255,255,255}, // Shallow water
        .start_height = 0, // Minimum value
        .end_height = 1.5, // Minimum value
    };

    for (int i = 1; i < background_layer_amount; i++) {      
        Uint32 *pixels = malloc(sizeof(Uint32)*terrain_size*terrain_size);
        // Set pixels transparent
        memset(pixels, SDL_MapRGBA(pixel_format,0,0,0,255), sizeof(Uint32)*terrain_size*terrain_size);
        // Run terrain generation
        generate_terrain(terrain_size, 0, 0, 1.5 ,1, height);  
        // Create a cloud pixel map from a height map
        get_terrain_pixels(pixels, terrain_size, clouds ,height, pixel_format);
        background_layers[i].pixels = pixels; // Add the pixels to the struct for this layer
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(background_layers[i].pixels, terrain_size, terrain_size, 0,terrain_size * sizeof(Uint32), pixel_format_id);
        SDL_Texture *cloud_texture = SDL_CreateTextureFromSurface(renderer,surface);
        background_layers[i].texture = cloud_texture;
    }
    
    //
    // Sprites
    //
    const char *dir_name[] = {"Sprites"}; 
    const char *sprite_names[] = {"Spitfire"};
    int dir_amount = 1;
    int sprite_amount = 11;
    int sprite_middle = (sprite_amount - 1)/2; 
    
    struct sprite {
        SDL_Surface* surface;
        SDL_Texture* texture;
        double position[2];
        double velocity[2];
        double last_update_time[2];
        int size;
        int render; // 1 to render and 0 to not. 
        int number; 
        char filename[];
    };
    
    struct sprite sprites[sprite_amount];

    // Loaintd sprites
    for (int j = 0; j < dir_amount; j++){   
        for (int i = 0; i < sprite_amount; i++) {
            char spt[80];
            sprintf(spt, "%i", i+1);
            // Organise the filename extension
            strcpy(sprites[i].filename, dir_name[j]);
            strcat(sprites[i].filename, "/");
            strcat(sprites[i].filename, sprite_names[j]);
            strcat(sprites[i].filename, spt);
            strcat(sprites[i].filename, ".bmp");
            puts(sprites[i].filename);
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
            sprites[i].number = i + 1; 

            if (i == sprite_middle){
                sprites[i].render = 1;
            } else {
                sprites[i].render = 0;
            }
        }
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
    int animation_speed = 0.05;
    int animation_time = SDL_GetTicks();   

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
                            for (int i = 0; i < sprite_amount; i++) {
                                if ((SDL_GetTicks() - animation_time > animation_speed) && (sprites[i].render == 1) && (sprites[i].number != 1)){
                                    sprites[i - 1].render = 1; 
                                    sprites[i].render = 0; 
                                    animation_time = SDL_GetTicks(); 
                                }
                            }
                            break; 
                        case SDLK_RIGHT:
                            right_speed = velocity;
                            for (int i = 0; i < sprite_amount; i++) {
                                if ((SDL_GetTicks() - animation_time > animation_speed) && (sprites[i].render == 1) && (sprites[i].number != 11)){
                                    sprites[i + 1].render = 1; 
                                    sprites[i].render = 0;    
                                    animation_time = SDL_GetTicks();                                   
                                }
                            }                           
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

        //Return animation if no inputs 
        if (SDL_GetTicks() - animation_time > animation_speed) { 
            for (int i = 1; i < sprite_amount +1; i++) {
                if ((sprites[i].render == 1) && (sprites[i].number > sprite_middle)){
                    sprites[i - 1].render = 1; 
                    sprites[i].render = 0; 
                    animation_time = SDL_GetTicks(); 
                } else if ((sprites[i].render == 1) && (sprites[i].number < sprite_middle)){
                    sprites[i + 1].render = 1; 
                    sprites[i].render = 0; 
                    animation_time = SDL_GetTicks();     
                }                
            }     
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
                
                //Setting the Sprites Velocity 
                sprites[i].velocity[0] = right_speed - left_speed;
                sprites[i].velocity[1] = down_speed - up_speed; 

                //Stop you from getting speeding tickets on diagnols 
                if (sprites[i].velocity[0] != 0 && sprites[i].velocity[1] != 0){
                    if ((sprites[i].velocity[0] == velocity || sprites[0].velocity[0] == -velocity ) && (sprites[i].velocity[1] == velocity || sprites[i].velocity[1] == -velocity)){
                        sprites[i].velocity[0] = sprites[i].velocity[0]*0.7; 
                        sprites[i].velocity[1] = sprites[i].velocity[1]*0.7;
                    }   
                }
            }
            if (sprites[i].render == 1){
                SDL_Rect dest = {sprites[i].position[0],sprites[i].position[1],sprites[i].surface->w*pixel_scaling,sprites[i].surface->h*pixel_scaling};
                SDL_RenderCopyEx(renderer, sprites[i].texture, NULL, &dest, 0, NULL, SDL_FLIP_NONE);
            }
        }
        
        SDL_RenderPresent(renderer); // Show the completed frame and wait for vsync
        SDL_RenderClear(renderer); // Erase the screen (first action of the new frame)
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
