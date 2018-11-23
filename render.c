///3d Rendering Engine 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

// Screen Properties
int screen_width = 1000;
int screen_height = 500;
int terrain_size = 500;

//Variable Devlerations
float camera_pos[3] = {0, 0, 0};
float camera_ang[3] = {0, 0, 0};
float view_angle = 60; 
int z_max_distance = 10;
int z_min_distance = 1;

//Manages the data for the biomes 
struct terrain_layer {
    int start_color[4];
    int end_color[4];
    float end_height;
    float start_height;
};

//Loop condition
int run = 1; 

//Random Data Shite
typedef Uint32 pixel;

//Function Declerations
int render(double **height, float distance, int screen_width, int screen_height, int view_angle, float projection_matrix, float camera_ang[2], struct terrain_layer layer, SDL_Renderer renderer);
int projection(float *position, float **projection_matrix, float *screen_position, float *camera_ang);
int rotation(float *position, float *camera_ang, float *camera_pos, float *result);
int generate_terrain(int size, double x_layer, double y_layer, double z_layer, float **z);
int get_terrain_pixels(pixel *pixels, int pixel_amount, struct terrain_layer layer, float **height, SDL_PixelFormat *pixel_format);
int matrix_vector_multiplication(float *vector, float **matrix, int size, float *result);

//Main Function 
int main() {
    //Setting up SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window*window  = SDL_CreateWindow("Hopefully this is 3d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1000, 1000, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_Renderer*renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    SDL_Event window_event;

    //Non-compiling Set-up
    float aspect_ratio = screen_height/screen_width;
    float z_normalize = z_max_distance/(z_max_distance - z_min_distance);
    float feild_of_view = 1/tan(view_angle/2);

    //The projection Matrix used to transfer 3d heightmap into 2d screen co-ordinates
    float projection_matrix[4][4] = {
        {aspect_ratio*feild_of_view, 0,             0,                           0},
        {0,                          feild_of_view, 0,                           0},
        {0,                          0,             z_normalize,                 1},
        {0,                          0,             -z_max_distance*z_normalize, 0}
    };

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

    //The Main Loop
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
        }

        // Clear the current renderer
        SDL_RenderPresent(renderer);

        //REserving memory for heights and shadow data
        float **height = malloc(sizeof(float*)*terrain_size);
        // Allocate each float pointer as an array of actual floats
        for (int i=0; i<terrain_size; i++) {
            height[i] = malloc(sizeof(float)*terrain_size);
        }

        //Generate Map
        generate_terrain(terrain_size, camera_pos[0], camera_pos[1], camera_pos[2], height);

        //Draw Land 
        render(height, z_max_distance, screen_width, screen_height, camera_ang, terrain_layer layer), SDL_Renderer renderer;

        //Render the screen
        SDL_RenderPresent(renderer); 
    }

    //Destroy and Clode SDL after program finishes
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}

//Render Function, Responable for finding what pixels on the screen need to be drawn and drawing them
int render(double **height, float distance, int screen_width, int screen_height, int view_angle, float **projection_matrix, float *camera_ang, struct terrain_layer layer, SDL_Renderer renderer){
    //Setting up the Y-buffer for each coloumn (Stops Pixels being redrawn)
    float y_buffer[screen_width];

    for(int i = 0; i < screen_width; i++){
        y_buffer[i] = screen_width; 
    }

    //For each pixel on the screen
    for(int i = 0; i < distance; i++){
        //Find what part of the heightmap needs to be drawn (This refers to the part of the heightmap that needs drawing)
        //Rotated manually should remove and use rotate function when up and running (i.e. remove all sins/coss)
        int view_left[2] = {-tan(view_angle/2)*cos(camera_ang[2])*i + -tan(view_angle/2)*sin(camera_ang[2])*i  + camera_pos[0], 
                              i*sin(camera_ang[2])                    + -i*cos(camera_ang[3])                    + camera_pos[1]};

        int view_right[2] = {tan(view_angle/2)*cos(camera_ang[2])*i  + -tan(view_angle/2)*sin(camera_ang[2])*i + camera_pos[0], 
                              -i*sin(camera_ang[2])                   + -i*cos(camera_ang[3])                    + camera_pos[1]};

        //The step size 
        float x_step = (view_right[0] - view_left[0])/screen_width; 
        float y_step = (view_right[1] - view_left[1])/screen_width;
 
        for(int j = 0; i < screen_width; i++){
            float position[3] = {i, j, height[view_left[0]][view_left[1]]};
            
            //Finding the screen co-ordinate
            float *screen_position;
            screen_position = malloc(4*sizeof(float));
            projection(position, projection_matrix, screen_position, camera_ang);

            //Find the colour of that position
            pixel* land_pixels = malloc(sizeof(pixel));
            for(int layer=0; layer < sizeof(biome) / sizeof(struct terrain_layer); layer++) {
                get_terrain_pixels(land_pixels, terrain_size, biome[layer], height[view_left[0]][view_left[1]][0], SDL_PixelFormat *pixel_format);
            }

            //Set colour then draw base colour line
            SDL_SetRenderDrawColor(renderer, land_pixels[0], land_pixels[1], land_pixels[2], SDL_ALPHA_OPAQUE); 
            SDL_RenderDrawLine(renderer, j, screen_position[1], j, y_buffer[i]);

            //Set Alpha then draw the shadow 
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, height[view_left[0]][view_left[1]][1]);
            SDL_RenderDrawLine(renderer, j, screen_position[1], j, y_buffer[i]);

            //Update Y_buffer if large (source says smaller but makes no sense, idk try changing if it doesnt work)
            if (screen_position[1] > y_buffer[i]){
                y_buffer[i] = screen_position[i];
            }

            //Change the line position using step size 
            view_left[0] += x_step;
            view_left[1] += y_step; 
        }
    }
    return 0;
}

//Turns coordinates from 3d coordinate system into the Screens 2d cooridinate system
int projection(float *position, float **projection_matrix, float *screen_position, float *camera_ang){

    //Mulitplying the input co-ordinate by the projection matrix to transform it into 3d
    matrix_vector_multiplication(position, projection_matrix, 3, screen_position);

    //Dividing by the last term in the matrix (same as dividing by z)
    if (screen_position[3] != 0){
        screen_position[0] /= screen_position[3];
        screen_position[1] /= screen_position[3];
        screen_position[2] /= screen_position[3];
    }
    return 0;
}

//This accounts for the rotation of the camera 
int rotation(float *position, float *camera_ang, float *camera_pos, *result){
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

    //Performing rotation
    matrix_vector_multiplication(translated_position, rotation_x, 2, x_result);
    matrix_vector_multiplication(translated_position, rotation_y, 2, y_result);
    matrix_vector_multiplication(translated_position, rotation_z, 2, z_result);

    //Adding and returning it back to original position  
    result[0] = x_result[0] + y_result[0] + z_result[0] + position[0];
    result[1] = x_result[1] + y_result[1] + z_result[1] + position[1];
    result[2] = x_result[2] + y_result[2] + z_result[2] + position[2];

    return 0;  
}

//Multiplying a Matrix by a vector or position (used for projection and rotation)
int matrix_vector_multiplication(float *vector, float **matrix, int size, float *result){
    for (i =0; i < size; i++){
        result[i] = vector[0]*matrix[0][i] + vector[1]*matrix[1][i] + vector[2]*matrix[2][i];
    }
    return result;
}

//Define the Colours of each pixel
int get_terrain_pixels(pixel *pixels, struct terrain_layer layer, float height) {
    if (layer.start_height <= height && layer.end_height >= height) {
        float pixel_color[4];
        for(int color=0; color <= 3; color++) {
            // linearly interpolate between the two end colors // TODO add something so beaches aren't so discontinious
            pixel_color[color] = layer.start_color[color] + ((layer.start_color[color] - layer.end_color[color])*(height-layer.start_height))/(layer.start_height - layer.end_height);
        }
    }
    return 0;
}