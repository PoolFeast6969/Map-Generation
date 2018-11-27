///3d Rendering Engine 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

//Manages the data for the biomes 
struct terrain_layer {
    int start_color[4];
    int end_color[4];
    float end_height;
    float start_height;
};

// Screen Properties
int screen_width = 1000;
int screen_height = 1000;
int terrain_size = 500;

//Variable Devlerations
float camera_pos[3] = {0, 0, 0};
float camera_ang[3] = {0, 0, 0};
float light_dir[3] = {0.5, 0, -0.2};
int view_angle = 60; 
int z_max_distance = 10;
int z_min_distance = 1;

//Stores the Colour Data
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

//Loop conditionc
int run = 1; 

//Random Data Shite
typedef Uint32 pixel;

//Function Declerations
int render(float ***height, float distance, int screen_width, int screen_height, int view_angle, float projection_matrix[4][4], float *camera_ang, struct terrain_layer biome[3], SDL_Renderer *renderer, SDL_PixelFormat *pixel_format);
int projection(float *position, float projection_matrix[4][4], int *screen_position, float *camera_ang);
int rotation(float *position, float *camera_ang, float *camera_pos, float *result);
int generate_terrain(int size, float x_layer, float y_layer, float z_layer, float ***z, float *light_dir);
int get_terrain_colour(pixel *pixel_color, struct terrain_layer layer, float height);

//Main Function 
int main() {
    printf("Starting\n");

    //Setting up SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window*window  = SDL_CreateWindow("Hopefully this is 3d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_Renderer*renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_Event window_event;

    pixel pixel_format_id = SDL_PIXELFORMAT_RGBA32; 
    SDL_PixelFormat *pixel_format = SDL_AllocFormat(pixel_format_id); 

    //Non-compiling Set-up (floats) are because calculating with ints but expecting float output
    view_angle = view_angle*(M_PI/180); //Turning Degrees to Radians 
    float aspect_ratio =  (float) screen_height/screen_width;
    float z_normalize =   (float) z_max_distance/(z_max_distance - z_min_distance);
    float z_other =       (float) -z_max_distance*z_normalize;
    float feild_of_view = (float) 1/tan(view_angle/2);

    //The projection Matrix used to transfer 3d heightmap into 2d screen co-ordinates
    float projection_matrix[4][4] = {
        {aspect_ratio*feild_of_view, 0,             0,           0},
        {0,                          feild_of_view, 0,           0},
        {0,                          0,             z_normalize, 1},
        {0,                          0,             -z_other,    0}
    };

    //The Main Loop
    printf("Entering Main Loop \n");

    while(run == 1){
        while (SDL_PollEvent(&window_event)){
            switch( window_event.type ){
                case SDL_QUIT:
                    run = 0;
                    break;
                //Look for a keypress
                case SDL_KEYDOWN:
                    //Check the SDLKey values and move change the coords
                    switch( window_event.key.keysym.sym ){
                        case SDLK_LEFT:
                            break; 
                        case SDLK_RIGHT:                     
                            break; 
                        case SDLK_UP:
                            break; 
                        case SDLK_DOWN:
                            break; 
                        default:    
                            break;
                    }
                    break;
                //Look for key up
                case SDL_KEYUP:
                    //Check the SDLKey values and move change the coords
                    switch( window_event.key.keysym.sym ){
                        case SDLK_LEFT:                        
                            break;
                        case SDLK_RIGHT:                        
                            break;
                        case SDLK_UP:
                            break;
                        case SDLK_DOWN:
                            break;
                        default:    
                            break;
                    }
                    break;
            }

            float ***height = malloc(sizeof(float**)*terrain_size);
            // Allocate each float pointer as an array of actual floats
            for (int i=0; i<terrain_size; i++) {
                height[i] = malloc(sizeof(float*)*terrain_size);
                for (int j=0; j<terrain_size; j++){
                    height[i][j] = malloc(sizeof(float)*3);
                }
            }

            //Generate the Map
            generate_terrain(terrain_size, camera_pos[0], camera_pos[1], camera_pos[2], height, light_dir);

            //Render the Map
            render(height, z_max_distance, screen_width, screen_height, view_angle, projection_matrix, camera_ang, biome, renderer, pixel_format);

            //Render the screen
            SDL_RenderPresent(renderer); 

            // Clear the current renderer
            SDL_RenderClear(renderer);
        }
    }

    //Destroy and Clode SDL after program finishes
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}

//Render Function, Responable for finding what pixels on the screen need to be drawn and drawing them
int render(float ***height, float distance, int screen_width, int screen_height, int view_angle, float projection_matrix[4][4], float *camera_ang, struct terrain_layer biome[3], SDL_Renderer *renderer, SDL_PixelFormat *pixel_format){  
    //Setting up the Y-buffer for each coloumn (Stops Pixels being redrawn)
    float y_buffer[screen_width + 1];

    for(int i = 0; i <= screen_width; i++){
        y_buffer[i] = screen_width; 
    }

    //For each pixel on the screen
    //CHange to Z_MIN_DISTANCE LATER
    for (int i = 1; i < distance; i++){
        //Find what part of the heightmap needs to be drawn (This refers to the part of the heightmap that needs drawing)
        //Rotated manually should remove and use rotate function when up and running (i.e. remove all sins/coss)
        //int view_left[2] = {-tan((view_angle/2))*cos(camera_ang[0])*i -tan(view_angle/2)*sin(camera_ang[0])*i  + camera_pos[0], 
        //                    i*sin(camera_ang[0])                      -i*cos(camera_ang[0])                    + camera_pos[1]};
        //
        //int view_right[2] = {tan(view_angle/2)*cos(camera_ang[0])*i   -tan(view_angle/2)*sin(camera_ang[0])*i + camera_pos[0], 
        //                     -i*sin(camera_ang[0])                    -i*cos(camera_ang[0])                   + camera_pos[1]};

        float view_left[2] = {-i  + camera_pos[0], 
                              -i+ camera_pos[1]};

        float view_right[2] = {i  + camera_pos[0], 
                              -i+ camera_pos[1]};                     

        //The step size 
        float x_step = (view_right[0] - view_left[0])/screen_width; 
        float y_step = (view_right[1] - view_left[1])/screen_width;
        
        for(int j = 0; j < screen_width; j++){
            float position[3] = {view_left[0], view_left[1], height[(int) abs(view_left[0])][(int) abs(view_left[1])][0]};
            
            //Finding the screen co-ordinate
            int *screen_position;
            screen_position = malloc(4*sizeof(float));
            projection(position, projection_matrix, screen_position, camera_ang);

            //Find the colour of that position
            pixel *pixel_color; 
            pixel_color = malloc(sizeof(pixel)*4);

            for(int layer=0; layer < 3; layer++) {
                get_terrain_colour(pixel_color, biome[layer], height[(int) abs(view_left[0])][(int) abs(view_left[1])][0]);
            }

            //Test line
            SDL_SetRenderDrawColor(renderer, pixel_color[0], pixel_color[1], pixel_color[2], 255);
            //SDL_RenderDrawLine(renderer, 0, 10, 10, 20);

            //Set colour then draw base colour line
            //SDL_SetRenderDrawColor(renderer, pixel_color[0], pixel_color[1], pixel_color[2], 255); 
            //SDL_RenderDrawLine(renderer, j, screen_height - screen_position[1], j, screen_height - y_buffer[i]);

            //Set Alpha then draw the shadow 
            //SDL_SetRenderDrawColor(renderer, 255, 255, 255, height[view_left[0]][view_left[1]][1]);
            //SDL_RenderDrawLine(renderer, j, screen_height - screen_position[1], j, screen_height - y_buffer[i]);

            //Update Y_buffer if large (source says smaller but makes no sense, idk try changing if it doesnt work)

            if (screen_position[1] < y_buffer[j]){
                y_buffer[j] = screen_position[1];
            }  

            //Change the line position using step size 
            view_left[0] += x_step;
            view_left[1] += y_step; 
        }
    }

    //End the Function
    return 0;
}

//Turns coordinates from 3d coordinate system into the Screens 2d cooridinate system
int projection(float *position, float projection_matrix[4][4], int *screen_position, float *camera_ang){
    //Matrix Vector Multiplication
    for (int i =0; i < 3; i++){
        screen_position[i] = position[0]*projection_matrix[0][i] + position[1]*projection_matrix[1][i] + position[2]*projection_matrix[2][i];
    }

    //Dividing by the last term in the matrix (same as dividing by z)
    if (screen_position[3] != 0){
        screen_position[0] /= screen_position[3];
        screen_position[1] /= screen_position[3];
        screen_position[2] /= screen_position[3];
    }

    //End the Function
    return 0;
}

//This accounts for the rotation of the camera 
int rotation(float *position, float *camera_ang, float *camera_pos, float *result){
    //X axis rotation matrix
    float rotation_x[3][3] = {
        {1, 0,                  0,                 },
        {0, cos(camera_ang[0]), -sin(camera_ang[0])},
        {0, sin(camera_ang[0]), cos(camera_ang[0]),},
    };

    //Y axis rotation matrix
    float rotation_y[3][3] = {
        {cos(camera_ang[1]),  0, sin(camera_ang[1])},
        {0,                   1, 0,                },
        {-sin(camera_ang[1]), 0, cos(camera_ang[1])},
    };

    //Z axis rotation matrix
    float rotation_z[3][3] = {
        {cos(camera_ang[2]), -sin(camera_ang[2]), 0},
        {sin(camera_ang[2]), cos(camera_ang[2]),  0},
        {0,                  0,                   1},
    };    

    //Addjusting for camera position (i.e. translating postion back to origin)
    float translated_position[3] = {
        position[0] - camera_pos[0],
        position[1] - camera_pos[1],
        position[2] - camera_pos[2]
    };

    //Setting up the memory to receive the output
    float *x_result;
    float *y_result;
    float *z_result;

    x_result = malloc(3*sizeof(float));
    y_result = malloc(3*sizeof(float));
    z_result = malloc(3*sizeof(float));

    //Matrix Vector Multiplication
    for (int i =0; i < 3; i++){
        x_result[i] = translated_position[0]*rotation_x[0][i] + translated_position[1]*rotation_x[1][i] + translated_position[2]*rotation_x[2][i];
    }

    for (int i =0; i < 3; i++){
        y_result[i] = translated_position[0]*rotation_y[0][i] + translated_position[1]*rotation_y[1][i] + translated_position[2]*rotation_y[2][i];
    }

    for (int i =0; i < 3; i++){
        z_result[i] = translated_position[0]*rotation_z[0][i] + translated_position[1]*rotation_z[1][i] + translated_position[2]*rotation_z[2][i];
    }    

    //Adding and returning it back to original position  
    result[0] = x_result[0] + y_result[0] + z_result[0] + position[0];
    result[1] = x_result[1] + y_result[1] + z_result[1] + position[1];
    result[2] = x_result[2] + y_result[2] + z_result[2] + position[2];

    //End the Function
    return 0;  
}

//Define the Colours of each pixel
int get_terrain_colour(pixel *pixel_color, struct terrain_layer layer, float height) {
    if (layer.start_height <= height && layer.end_height >= height) {
        for(int color = 0; color <= 3; color++) {
            // linearly interpolate between the two end colors // TODO add something so beaches aren't so discontinious
            pixel_color[color] = layer.start_color[color] + ((layer.start_color[color] - layer.end_color[color])*(height-layer.start_height))/(layer.start_height - layer.end_height);
        }
    }

    //End the function
    return 0;
}