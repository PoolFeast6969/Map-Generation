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

//Random Data Shite
typedef Uint32 pixel;

//Function Declerations
int render(float z, float z_max_distance, float screen_width, float screen_height, float camera_ang[3], struct biome);
float projection(float position[3], float* prjection_matrix[4][4], float screen_position[4], float camera_ang[3]);
int generate_terrain(int size, double x_layer, double y_layer, double z_layer_float **z);
int get_terrain_pixels(pixel *pixels, int pixel_amount, struct terrain_layer layer, float **height, SDL_PixelFormat *pixel_format);

//Main Function 
int main(){
    //Setting up SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window*window  = SDL_CreateWindow("Hopefully this is 3d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1000, 1000, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_Renderer*renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

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
    while(){
        // Clear the current renderer
        SDL_RenderPresent(renderer);

        //REserving memory for heights and shadow data
        float **height = malloc(sizeof(float*)*terrain_size);
        // Allocate each float pointer as an array of actual floats
        for (int i=0; i<terrain_size; i++) {
            height[i] = malloc(sizeof(float)*terrain_size);
        }

        //Generate Map
        generate_terrain(terrain_size, camera_pos[0], camera_pos[1], camera_pos[3], height);

        //Draw Land 
        render(height, z_max_distance, screen_width, screen_height, camera_ang, biome);

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
int render(float **height, float distance, int screen_width, int screen_height, float projection_matrix, float camera_ang, struct biome){
    //Setting up the Y-buffer for each coloumn 
    float y_buffer[screen_width];

    for(int i = 0; i < screen_width; i++){
        y_buffer[i] = screen_width; 
    }

    //For each pixel 
    for(int i = 0; i < distance; i++){
        for(int j = 0; i < screen_width; i++){
            float position[3] = {i, j, height[i][j]};
            
            //Finding the screen co-ordinate
            float screen_position[3];
            projection(position, projection_matrix, screen_position, camera_ang);

            //Find the colour of that position
            pixel* land_pixels = malloc(sizeof(pixel));
            for(int layer=0; layer < sizeof(biome) / sizeof(struct terrain_layer); layer++) {
                get_terrain_pixels(land_pixels, terrain_size, biome[layer], height, pixel_format);
            }

            //Set colour then raw Line 
            SDL_SetRenderDrawColor(renderer, land_pixel[0], land_pixel[1], land_pixel[2]); 
            SDL_RenderDrawLine(renderer, j, screen_position[1], j, y_buffer[i]);

            //Update Y_buffer if larger
            if (screen_position[1] < y_buffer[i]){
                y_buffer[i] = screen_position[i];
            }
        }
    }
}

//Turns coordinates from 3d coordinate system into the Screens 2d cooridinate system
float projection(float position, float projection_matrix, float screen_position, float camera_ang){
    //X axis rotation matrix
    float rotation_x[4][4] = {
        {1, 0,                  0,                   0},
        {0, cos(camera_ang[0]), -sin(camera_ang[0]), 0},
        {0, sin(camera_ang[0]), cos(camera_ang[0]),  0},
        {0, 0,                  0,                   1}
    };

    //Y axis rotation matrix
    float rotation_y[4][4] = {
        {cos(camera_ang[1]),  0, sin(camera_ang[1]), 0},
        {0,                   1, 0,                  0},
        {-sin(camera_ang[1]), 0, cos(camera_ang[1]), 0},
        {0,                   0, 0,                  1}
    };

    //Z axis rotation matrix
    float rotation_z[4][4] = {
        {cos(camera_ang[2]), -sin(camera_ang[2]), 0, 0},
        {sin(camera_ang[2]), cos(camera_ang[2]),  0, 0},
        {0,                  0,                   1, 0},
        {0,                  0,                   0, 1}
    };

    //Mulitplying the input co-ordinate by the projection matrix to transform it into 3d
    for (i =0; i < 3; i++){
        screen_position[i] = position[0]*projection_matrix[0][i] + position[1]*projection_matrix[1][i] + position[2]*projection_matrix[2][i] + projection_matrix[3][i];
    }

    //Dividing by the last term in the matrix (same as dividing by z)
    if (scale != 0){
        screen_position[0] /= screen_position[3];
        screen_position[1] /= screen_position[3];
        screen_position[2] /= screen_position[3];
    }
}

//Define the Colours of each pixel
int get_terrain_pixels(pixel *pixels, struct terrain_layer layer, float **height) {
    if (layer.start_height <= height[x][y] && layer.end_height >= height[x][y]) {
        float pixel_color[4];
        for(int color=0; color <= 3; color++) {
            // linearly interpolate between the two end colors // TODO add something so beaches aren't so discontinious
            pixel_color[color] = layer.start_color[color] + ((layer.start_color[color] - layer.end_color[color])*(height[x][y]-layer.start_height))/(layer.start_height - layer.end_height);
        }    
    }
    return 0;
}

