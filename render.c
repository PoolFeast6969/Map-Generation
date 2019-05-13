/* lmao */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "SDL2/SDL.h"

// The compiler needs to know that this function exists before it calls it, or something like that
int generate_terrain (int , double , double , double , double, float **);

struct terrain_layer {
    int start_color[4];
    int end_color[4];
    float end_height;
    float start_height;
};

typedef Uint32 pixel;

int get_terrain_pixels(pixel *pixels, int pixel_amount, struct terrain_layer layer, float **height, SDL_PixelFormat *pixel_format) {
    // Convert height map to pixel color map
    for(int x=0; x < pixel_amount; x++) {
        for(int y=0; y < pixel_amount; y++) {
                if (layer.start_height <= height[x][y] && layer.end_height >= height[x][y]) { // the layer of interest
                    float pixel_color[4]; 
                    for(int color=0; color <= 3; color++) {
                        // linearly interpolate between the two end colors // TODO add something so beaches aren't so discontinious
                        pixel_color[color] = layer.start_color[color] + ((layer.start_color[color] - layer.end_color[color])*(height[x][y]-layer.start_height))/(layer.start_height - layer.end_height);
                    }
                    pixels[x*pixel_amount + y] = SDL_MapRGBA(pixel_format,pixel_color[0],pixel_color[1],pixel_color[2],pixel_color[3]);
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
    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    // Set pixel format to make the textures in
    Uint32 pixel_format_id = SDL_PIXELFORMAT_RGBA32;
    //Uint32 pixel_format_id = SDL_PIXELFORMAT_RGB332; // should be able to do whatever you want here, just remember to change typedef pixel
    SDL_PixelFormat *pixel_format = SDL_AllocFormat(pixel_format_id); // Get the actual format object from its ID
    
    //
    // Terrain Heights Array
    //

    // yep all these pixels are the same size
    double pixel_scaling = 5;

    // based off window size
    int terrain_size = 1000/pixel_scaling;

    // Make array for the terrain generator to fill (a texture i guess)
    // Allocating memory for the matrix which will store the altitudes
    // Allocate the first dimension as an array of float pointers
    float **height = malloc(sizeof(float*)*terrain_size);
    // Allocate each float pointer as an array of actual floats
    for (int i=0; i<terrain_size; i++) {
        height[i] = malloc(sizeof(float)*terrain_size);
    }

    //
    // Background
    //

    struct terrain_layer biome[] = {
        {
            .start_color = {36,36,85,255}, // Deep water
            .end_color = {36,109,170,255}, // Shallow water
            .start_height = 0, // Minimum value
            .end_height = 0.4, // Minimum value
        },{
            .start_color = {148,148,123,255}, // low sand
            .end_color = {170,180,150,255}, // high sand
            .start_height = 0.4, // Minimum value
            .end_height = 0.45, // halfway
        },{
            .start_color = {0,125,0,255}, // low land
            .end_color = {0,218,0,255}, // high land
            .start_height = 0.45, // Minimum value
            .end_height = 1, // halfway
        }
    };

    struct terrain_layer clouds = {
        .start_color = {180,218,255,255}, // Deep water
        .end_color = {255,255,255,255}, // Shallow water
        .start_height = 0.6, // Minimum value
        .end_height = 1, // Maximum value
    };

    struct terrain_layer cloud_shadows = {
        .start_color = {0,0,0,100}, // Deep water
        .end_color = {0,0,0,200}, // Shallow water
        .start_height = 0.6, // Minimum value
        .end_height = 1, // Maximum value
    };

    struct background_layer { // Defines one background layer, ie the land or one cloud layer
        double distance; // Distance from view
        double z_layer; // Equivilant to the seed
        double position[2];
        double tile_array_position[2];
        double last_update_time[2]; // When each position was last updated
        SDL_Texture** tile_array; // for managing the loading of textures currently in view 
        struct terrain_layer *biome; // Used for generating the textures
    };

    // Instantiate the land layer struct
    struct background_layer land = {
        .distance = 5,
        .z_layer = 696969,
        .biome = biome,
    };

    // Creates an array of cloud layers with their height and density already set, and the land already in the background
    struct background_layer background_layers[] = {land,{4.9,2},{2,5}};
            
    int background_layer_amount = sizeof(background_layers) / sizeof(struct background_layer);
    printf("There are %i background layer(s)\n",background_layer_amount);

    // Effects all background layers
    const int tile_array_height = 3;
    const int tile_array_width = 4;
    int tile_size[2];

    for (int i = 0; i < background_layer_amount; i++) {
        // initialise the tile arrays
        SDL_Texture** tile_array[tile_array_height][tile_array_width] = {0};
        background_layers[i].tile_array = tile_array;
    }

    //
    // Sprites
    //

    const char *dir_name[] = {"Sprites"}; 
    const char *sprite_names[] = {"Spitfire"};
    int character_amount = 1;
    int sprite_amount = 11;
    int sprite_middle = (sprite_amount - 1)/2; 
    
    struct sprite {
        SDL_Surface* surface;
        SDL_Texture* texture;
        int size;
        int render; // 1 to render and 0 to not. 
        int number; 
        char filename[50]; // hopefully you don't need more than 50 characters ey?
    };

    struct character {
        double position[2];
        double velocity[2];
        double last_update_time[2];
    };
    
    struct sprite sprites[sprite_amount];
    struct character character[character_amount];

    // Loaintd sprites
    for (int j = 0; j < character_amount; j++){   
        character[j].position[1] = 50;
        character[j].position[0] = 50;
        character[j].velocity[0] = 0;
        character[j].velocity[1] = 0;    
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
            // Delete surface or something?
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

    double view_velocity[] = {-0.05,-0.06};
    SDL_Event window_event;
    int window_w, window_h;

    bool run = true;
    int velocity = 1;
    int left_speed = 0; 
    int right_speed = 0; 
    int up_speed = 0; 
    int down_speed = 0;
    int animation_speed = 50;
    float animation_time = SDL_GetTicks();   

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

        // Draw each of the background layers with the correct position
        for (int i = 0; i < background_layer_amount; i++) {
            // Go through each axis
            for (int a = 0; a < 2 ; a++) {
                // Determine if it needs to be have its position changed
                double percieved_velocity = view_velocity[a]/background_layers[i].distance;
                double time_since_update = SDL_GetTicks() - background_layers[i].last_update_time[a];
                background_layers[i].position[a] = background_layers[i].position[a] + percieved_velocity * time_since_update;
                background_layers[i].last_update_time[a] = SDL_GetTicks();

                // Move the tile if the position has shifted enough
                double change = background_layers[i].position[a] - background_layers[i].tile_array_position[a];
                if (change > tile_size[a]) {
                    // shift the array in the correct direction

                }
            }

            // Update current window dimensions
            SDL_GetRendererOutputSize(renderer, &window_w, &window_h);

            // Calculate the size of a tile
            int tile_size[0] = window_h/tile_array_height;
            int tile_size[1] = window_w/tile_array_width;

            

            // Go through the tile array for this layer
            for (int row = 0; row < tile_array_height; row++) {
                for (int column = 0; column < tile_array_width; column++) {
                    if (background_layers[i].tile_array[row][column] == NULL) {
                        // Generate a new texture for this tile
                    } 
                    // Draw the tile on the screen


                    if (i > 0) { // don't touch the land
                        // Set pixels transparent, could be done faster, but what if transparent isn't all zeros?
                        for(int p=0;p<terrain_size*terrain_size;p++) pixels[p] = SDL_MapRGBA(pixel_format,0,0,0,0);
                        // Run terrain generation
                        generate_terrain(terrain_size, terrain_size, terrain_size, background_layers[i].z_layer, background_layers[i].distance, height);
                        // Create a cloud pixel map from a height map
                        get_terrain_pixels(pixels, terrain_size, clouds ,height, pixel_format);
                        // send to gpu
                        background_layers[i].texture = SDL_CreateTexture(renderer,pixel_format_id,SDL_TEXTUREACCESS_TARGET,terrain_size,terrain_size);
                        // Draw pixels into texture (slow?)
                        SDL_UpdateTexture(background_layers[i].texture,NULL,pixels, terrain_size * sizeof(pixel));
                    } else {
                        // For land
                        generate_terrain(terrain_size, terrain_size, terrain_size, 3.0, land.distance, height); // Get a terrain height map

                        // Reusable pixel array for raw values
                        pixel* pixels = malloc(sizeof(pixel)*terrain_size*terrain_size);

                        // Add each terrain layer into the pixel array to make land
                        for(int layer=0; layer < sizeof(biome) / sizeof(struct terrain_layer); layer++) {
                            get_terrain_pixels(pixels, terrain_size, biome[layer], height, pixel_format);
                        }

                        // Actually put the pixel data into the texture, feels weird to do this after its in the struct
                        SDL_UpdateTexture(land.texture, NULL, pixels, terrain_size * sizeof(pixel));
                    }
                    // now we know how big this array is, so preallocate it
                    SDL_Texture* shadow_textures[background_layer_amount - i - 1];
                    // Allow shadows to be rendered into existing texture, remembering to turn off later          
                    SDL_SetRenderTarget(renderer, background_layers[i].texture);
                    // Ensure blend thingo is on
                    SDL_SetTextureBlendMode(background_layers[i].texture, SDL_BLENDMODE_BLEND); 

                    // Get shadows for every layer above this one, this one being index i
                    // Iterating through all the indexs higher than this one, from closest to furthest
                    for (int caster_index = i + 1; caster_index < background_layer_amount; caster_index++) {
                        // Set pixels transparent, could be done faster, but what if transparent isn't all zeros?
                        for(int p=0;p<terrain_size*terrain_size;p++) pixels[p] = SDL_MapRGBA(pixel_format,0,0,0,0);
                        // Run terrain generation
                        generate_terrain(terrain_size, terrain_size, terrain_size, background_layers[caster_index].z_layer, background_layers[i].distance, height);
                        // Create a cloud pixel map from a height map
                        get_terrain_pixels(pixels, terrain_size, cloud_shadows ,height, pixel_format);
                        // send to gpu
                        shadow_textures[caster_index-i-1] = SDL_CreateTexture(renderer, pixel_format_id, SDL_TEXTUREACCESS_STATIC,terrain_size,terrain_size);
                        // Draw pixels into texture (slow?)
                        SDL_UpdateTexture(shadow_textures[caster_index-i-1],NULL, pixels, terrain_size * sizeof(pixel));
                        // Combine shadow and clouds for each layer using the renderer
                        // Using a custom blend thing to remove shadows where there is no surface for them to fall on
                        SDL_BlendMode blend_mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD);
                        SDL_SetTextureBlendMode(shadow_textures[caster_index-i-1], blend_mode);
                        // Add to texture using above blend mode
                        SDL_RenderCopy(renderer, shadow_textures[caster_index-i-1], NULL, NULL);
                    }
                    // Set target back to the screen
                    SDL_SetRenderTarget(renderer, NULL);


                    SDL_Rect new_position = {background_layers[i].position[0], background_layers[i].position[1], terrain_size*pixel_scaling, terrain_size*pixel_scaling};
                    // Add to frame
                    SDL_RenderCopy(renderer,background_layers[i].texture,NULL,&new_position);
                }
            }
        }

        // This records the velocity and postion for the character         
        for (int j = 0; j < character_amount; j++) {
            //Setting the Sprites Velocity 
            character[j].velocity[0] = right_speed - left_speed;
            character[j].velocity[1] = down_speed - up_speed; 

            //Stop you from getting speeding tickets on diagnols 
            if (character[j].velocity[0] != 0 && character[j].velocity[1] != 0){
                if ((character[j].velocity[0] == velocity || character[0].velocity[0] == -velocity ) && (character[j].velocity[1] == velocity || character[j].velocity[1] == -velocity)){
                    character[j].velocity[0] = character[j].velocity[0]*0.707; 
                    character[j].velocity[1] = character[j].velocity[1]*0.707;
                }   
            } 

            // Update the Position     
            for (int a = 0; a < 2 ; a++) {    
                double time_since_update = SDL_GetTicks() - character[j].last_update_time[a];
                character[j].position[a] = character[j].position[a] + character[j].velocity[a] * time_since_update;
                character[j].last_update_time[a] = SDL_GetTicks();
            }  

            //This manages the sprites for animations 
            for (int i = 0; i < sprite_amount; i++) {     
                //Roll animation command
                if (sprites[i].render == 1 && (SDL_GetTicks() - animation_time > animation_speed)){
                    //Rolling when moving left/right 
                    if ((character[j].velocity[0] < 0) && (sprites[i].number > 1)){
                        sprites[i - 1].render = 1; 
                        sprites[i].render = 0; 
                        animation_time = SDL_GetTicks(); 
                    } else if ((character[j].velocity[0] > 0) && (sprites[i].number < 11)){
                        sprites[i + 1].render = 1; 
                        sprites[i].render = 0;    
                        animation_time = SDL_GetTicks(); 

                    //Returning to middle when not moving                                       
                    } else if ((character[j].velocity[0] == 0) && (sprites[i].number > sprite_middle + 1)){
                        sprites[i - 1].render = 1; 
                        sprites[i].render = 0; 
                        animation_time = SDL_GetTicks(); 
                    } else if ((character[j].velocity[0] == 0) && (sprites[i].number < sprite_middle + 1)){
                        sprites[i + 1].render = 1; 
                        sprites[i].render = 0; 
                        animation_time = SDL_GetTicks();     
                    }   
                }      

                // Render the sprites
                if (sprites[i].render == 1){
                    SDL_Rect dest = {character[j].position[0],character[j].position[1],sprites[i].surface->w*pixel_scaling,sprites[i].surface->h*pixel_scaling};
                    SDL_RenderCopyEx(renderer, sprites[i].texture, NULL, &dest, 0, NULL, SDL_FLIP_NONE);
                }
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
