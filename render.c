///3d Rendering Engine 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

// Screen Properties
int screen_width = 1000;
int screen_height = 500;

//Variable Devlerations
float camera_pos[3] = {0, 0, 0};
float camera_ang[3] = {0, 0, 0};
float view_angle = 60; 
int z_max_distance = 10;
int z_min_distance = 1;

//Function Declerations
int render(x, y, z, phi, z_max, distance, screen_width, screen_height);
int projection(float position, float* prjection_matrix, float* screen_position);

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

    float projection_matrix[4][4] = {
        {aspect_ratio*feild_of_view, 0,             0,                           0},
        {0,                          feild_of_view, 0,                           0},
        {0,                          0,             z_normalize,                 1},
        {0,                          0,             -z_max_distance*z_normalize, 0}
    };

    while(){
        //Draw Sky

        //Draw Land 
        render();
    }

    //Destroy and Clode SDL after program finishes
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}

//Render Function, Responable for finding what pixels on the screen need to be drawn and drawing them
int render(float **height , float phi, float z_max, float distance, int screen_width, int screen_height, float projection_matrix){
    //Clear Current Render 
    SDL_RenderClear(renderer);

    //Setting up the Y-buffer for each coloumn 

    //For each pixel 
    for(int i = 0; i < distance; i++){
        for(int j = 0; i < screen_width; i++){
            float position[3] = {i, j, height[i][j]};
            
            //Finding the screen co-ordinate
            float *screen_position[3];
            projection(position, projection_matrix, screen_position);

            //Draw Line 
            SDL_SetRenderDrawColor(renderer , 0, 0, 0); 
            SDL_RenderDrawLine(renderer, j, screen_position[1], j, y_buffer[i]);

            //Update Y_buffer if larger
            if (screen_position[1] < y_buffer[i]){
                y_buffer[i] = screen_position[i];
            }
        }
    }
}

//Turns coordinates from 3d coordinate system into the Screens 2d cooridinate system
float projection(float position, float* prjection_matrix, float* screen_position){
    //Mulitplying the input co-ordinate by the projection matrix to transform it into 3d (COULD BE A FOR LOOP)
    *screen_position[0] = position[0]*prjection_matrix[0][0] + position[1]*prjection_matrix[1][0] + position[2]*prjection_matrix[2][0] + prjection_matrix[3][0];
    *screen_position[1] = position[0]*prjection_matrix[0][1] + position[1]*prjection_matrix[1][1] + position[2]*prjection_matrix[2][1] + prjection_matrix[3][1];
    *screen_position[2] = position[0]*prjection_matrix[0][2] + position[1]*prjection_matrix[1][2] + position[2]*prjection_matrix[2][2] + prjection_matrix[3][2];
    float scale         = position[0]*prjection_matrix[0][3] + position[1]*prjection_matrix[1][3] + position[2]*prjection_matrix[2][3] + prjection_matrix[3][3]; 

    if (scaler != 0){
        *screen_position[0] /= scale;
        *screen_position[1] /= scale;
        *screen_position[2] /= scale;
    }
}



