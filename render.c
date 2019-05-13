/* lmao */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "SDL2/SDL.h"

// The compiler needs to know that this function exists before it calls it, or something like that
int generate_terrain (int x_resolution, int y_resolution, double x_start, double y_start, double x_size, double y_size, double z_layer, float **z);

struct terrain_layer {
    int start_color[4];
    int end_color[4];
    float end_height;
    float start_height;
};

typedef Uint32 pixel;

int get_terrain_pixels(int x_resolution, int y_resolution, pixel *pixels, struct terrain_layer layer, float **height, SDL_PixelFormat *pixel_format) {
    // Convert height map to pixel color map
    for(int x=0; x < x_resolution; x++) {
        for(int y=0; y < y_resolution; y++) {
                if (layer.start_height <= height[x][y] && layer.end_height >= height[x][y]) { // the layer of interest
                    float pixel_color[4]; 
                    for(int color=0; color <= 3; color++) {
                        // linearly interpolate between the two end colors // TODO add something so beaches aren't so discontinious
                        pixel_color[color] = layer.start_color[color] + ((layer.start_color[color] - layer.end_color[color])*(height[x][y]-layer.start_height))/(layer.start_height - layer.end_height);
                    }
                    pixels[x*x_resolution + y] = SDL_MapRGBA(pixel_format,pixel_color[0],pixel_color[1],pixel_color[2],pixel_color[3]);
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

int get_terrain_texture(int size_x, int size_y, double location_x, double location_y, double x_location_size, double y_location_size, double z_layer, struct terrain_layer biome*, SDL_Texture* texture) {
    // Set pixels transparent, could be done faster, but what if transparent isn't all zeros?
    for(int p=0;p<size_x*size_y;p++) pixels[p] = SDL_MapRGBA(pixel_format,0,0,0,0);
    
    // Calculate the perlin map position to render (top left of tile)
    // Then run terrain generation
    generate_terrain(
        size_x, 
        size_y, 
        location_x,
        location_y,
        x_location_size,
        y_location_size,
        z_layer, 
        height
    );

    // Create a colored pixel map from a height map
    for (int layer = 0; layer < biome/sizeof(struct terrain_layer); layer++) {
        // render each biome layer on top of each other
        get_terrain_pixels(
            size_x, 
            size_y,                        
            pixels, 
            biome[layer],
            height, 
            pixel_format
        );
    }

    // Draw pixels into texture (slow?)
    SDL_UpdateTexture(texture,NULL, pixels, size_x * sizeof(pixel));

    return 0;
}

int main() {
    //
    // Init
    //

    printf("Starting\n");

    int window_w = 1000;
    int window_h = 1000;

    // yep all these pixels are the same size
    double pixel_scaling = 5;

    // Effects all background layers
    const int tile_array_height = 3;
    const int tile_array_width = 4;

    SDL_Init( SDL_INIT_VIDEO );
    SDL_Window *window = SDL_CreateWindow("planes but with less detail",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,window_h,window_w,SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    // Set pixel format to make the textures in
    Uint32 pixel_format_id = SDL_PIXELFORMAT_RGBA32;
    //Uint32 pixel_format_id = SDL_PIXELFORMAT_RGB332; // should be able to do whatever you want here, just remember to change typedef pixel
    SDL_PixelFormat *pixel_format = SDL_AllocFormat(pixel_format_id); // Get the actual format object from its ID

    //
    // Background
    //

    int tile_size[2];
    // Calculate the size of a tile
    int tile_size[0] = window_h/tile_array_height;
    int tile_size[1] = window_w/tile_array_width;

    // Make array for the terrain generator to fill (a texture i guess)
    // Allocating memory for the matrix which will store the altitudes
    // Allocate the first dimension as an array of float pointers
    float **height = malloc(sizeof(float*)*tile_size[0]);
    // Allocate each float pointer as an array of actual floats
    for (int i=0; i<tile_size[0]; i++) {
        height[i] = malloc(sizeof(float)*tile_size[1]);
    }

    // Reusable pixel array for raw values
    pixel* pixels = malloc(sizeof(pixel)*tile_size[0]*tile_size[1]);

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

    struct terrain_layer clouds[] = {{
        .start_color = {180,218,255,255}, // Deep water
        .end_color = {255,255,255,255}, // Shallow water
        .start_height = 0.6, // Minimum value
        .end_height = 1, // Maximum value
    }};

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
    struct background_layer background_layers[] = {
        land,
        {
            4.9,
            2,
            .biome=clouds
        },{
            2,
            5,
            .biome=clouds
        }
    };
            
    int background_layer_amount = sizeof(background_layers) / sizeof(struct background_layer);
    printf("There are %i background layer(s)\n",background_layer_amount);

    for (int i = 0; i < background_layer_amount; i++) {
        // initialise the tile arrays
        SDL_Texture** tile_array[tile_array_height][tile_array_width] = {0}; // Set everything to 0?
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
                if (abs(change) > tile_size[a]) {
                    // shift the array in the correct direction

                    background_layers[i].tile_array_position[a] = background_layers[i].position[a];
                }
            }

            // Go through the tile array for this layer
            for (int row = 0; row < tile_array_height; row++) {
                for (int column = 0; column < tile_array_width; column++) {
                    if (background_layers[i].tile_array[row][column] == NULL) {
                        // Generate a new texture for this tile
                        SDL_Texture* layer_texture = SDL_CreateTexture(renderer,pixel_format_id,SDL_TEXTUREACCESS_TARGET,tile_size[0],tile_size[1]);
                        get_terrain_texture(
                            tile_size[0], 
                            tile_size[1],
                            background_layers[i].tile_array_position[0] + tile_size[0]*column,
                            background_layers[i].tile_array_position[1] + tile_size[1]*row,
                            background_layers[i].distance, // These wont work
                            background_layers[i].distance,
                            background_layers[i].z_layer,
                            background_layers[i].biome,
                            layer_texture
                        )

                        //
                        // Shadows
                        //

                        // now we know how big this array is, so preallocate it
                        SDL_Texture* shadow_textures[background_layer_amount - i - 1];
                        // Allow shadows to be rendered into existing texture, remembering to turn off later          
                        SDL_SetRenderTarget(renderer, layer_texture);
                        // Ensure blend thingo is on
                        SDL_SetTextureBlendMode(layer_texture, SDL_BLENDMODE_BLEND); 

                        // Get shadows for every layer above this one, this one being index i
                        // Iterating through all the indexs higher than this one, from closest to furthest
                        for (int caster_index = i + 1; caster_index < background_layer_amount; caster_index++) {
                            shadow_textures[caster_index-i-1] = SDL_CreateTexture(renderer, pixel_format_id, SDL_TEXTUREACCESS_STATIC,tile_size[0],tile_size[1]);
                            get_terrain_texture(
                                tile_size[0], 
                                tile_size[1],
                                background_layers[i].tile_array_position[0] + tile_size[0]*column,
                                background_layers[i].tile_array_position[1] + tile_size[1]*row,
                                background_layers[i].distance, // These wont work
                                background_layers[i].distance,
                                background_layers[caster_index].z_layer,
                                {cloud_shadows},
                                shadow_textures[caster_index-i-1]
                            )
                            
                            // Combine shadow and clouds for each layer using the renderer
                            // Using a custom blend thing to remove shadows where there is no surface for them to fall on
                            SDL_BlendMode blend_mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD);
                            SDL_SetTextureBlendMode(shadow_textures[caster_index-i-1], blend_mode);
                            // Add to texture using above blend mode
                            SDL_RenderCopy(renderer, shadow_textures[caster_index-i-1], NULL, NULL);
                        }
                        // Set target back to the screen
                        SDL_SetRenderTarget(renderer, NULL);


                        // Add texture pointer to tile array
                        background_layers[i].tile_array[row][column] = layer_texture;

                    }
                    // Draw the tile on the screen
                    SDL_Rect new_position = {
                        background_layers[i].tile_array_position[0] + tile_size[0]*column,
                        background_layers[i].tile_array_position[1] + tile_size[1]*row, 
                        background_layers[i].tile_array_position[0] + tile_size[0]*(column+1), 
                        background_layers[i].tile_array_position[1] + tile_size[1]*(row+1), 
                    };
                    // Add to frame
                    SDL_RenderCopy(renderer,background_layers[i].tile_array[row][column],NULL,&new_position);
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
