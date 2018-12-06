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
int terrain_size = 1000;

//Variable Devlerations
float camera_pos[3] = {0, 0, 0};
float camera_ang[3] = {0, 0, 0};
float light_dir[3] = {0.5, 0, -0.2};
int camera_angle = 90; 
int z_max_distance = 350;
int z_min_distance = 1;

//Loop conditionc
int run = 1; 

//Random Data Shite
typedef Uint32 pixel;

//Function Decleration
int generate_terrain(int size, float x_layer, float y_layer, float z_layer, float ***z, float *light_dir);

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

//Main Function 
int main() {
    //Setting up SDL5
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window*window  = SDL_CreateWindow("Hopefully this is 3d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_Renderer*renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_Event window_event;

    pixel pixel_format_id = SDL_PIXELFORMAT_RGBA32; 
    SDL_PixelFormat *pixel_format = SDL_AllocFormat(pixel_format_id); 

    //Projection Matrix Set-up
    float view_angle =    (float) camera_angle*(M_PI/180); //Turning Degrees to Radians 
    float aspect_ratio =  (float) screen_height/screen_width;
    float z_normalize =   (float) z_max_distance/(z_max_distance - z_min_distance);
    float z_other =       (float) (-z_max_distance*z_min_distance)/(z_max_distance - z_min_distance);
    float feild_of_view = (float) 1/tan(view_angle/2);

    //The transition point from cone to circle 
    float cone_distance = cos(view_angle/2)*z_max_distance;

    float projected[2];

    float view_left[2] = {0,0};
    float view_right[2] = {0,0};

    float x_step = 0;
    float y_step = 0;

    float x_dist;
    float height_on_screen;

    float projection_matrix[4][4] = {
        {aspect_ratio*feild_of_view, 0,             0,           0},
        {0,                          feild_of_view, 0,           0},
        {0,                          0,             z_normalize, 1},
        {0,                          0,             z_other,     0}
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
            .start_color = {0,70,0,140}, // low land
            .end_color = {0,218,0,255}, // high land
            .start_height = 0.45, // Minimum value
            .end_height = 1, // halfway
        }
    };

    float ***height = malloc(sizeof(float**)*terrain_size);
    // Allocate each float pointer as an array of actual floats
    for (int i=0; i<terrain_size; i++) {
        height[i] = malloc(sizeof(float*)*terrain_size);
        for (int j=0; j<terrain_size; j++){
            height[i][j] = malloc(sizeof(float)*3);
        }
    }

    //Generate the Map
    generate_terrain(terrain_size, camera_pos[0], camera_pos[1], 1.5, height, light_dir);

    while(run == 1){
        while (SDL_PollEvent(&window_event)){
            switch( window_event.type ){
                case SDL_QUIT:
                    run = 0;
                    break;

            }
        }

        float y_buffer[screen_width];

        for(int i = 0; i <= screen_width; i++){
            y_buffer[i] = screen_height; 
        }                       

        float i = 0.5;
        float dz = 1;

        while(i < z_max_distance){
            //Cone Shaped Section
            if (cone_distance > i){
                view_left[0] = -tan(view_angle/2)*cos(camera_ang[0])*i -tan(view_angle/2)*sin(camera_ang[0])*i + screen_width/2 + camera_pos[0]; 
                view_right[0] = tan(view_angle/2)*cos(camera_ang[0])*i -tan(view_angle/2)*sin(camera_ang[0])*i + screen_width/2 + camera_pos[0];  

            //Pizza Crust Shaped Section          
            } else {
                x_dist = sqrt(pow(z_max_distance,2) - pow(i,2));
                view_left[0]  = -x_dist*cos(camera_ang[0]) - x_dist*sin(camera_ang[0])+ screen_width/2 + camera_pos[0];
                view_right[0] = x_dist*cos(camera_ang[0])  - x_dist*sin(camera_ang[0])+ screen_width/2 + camera_pos[0];
            }

            view_left[1] =   i*sin(camera_ang[0]) -i*cos(camera_ang[0]) + screen_height/2 + camera_pos[1];
            view_right[1] = -i*sin(camera_ang[0]) -i*cos(camera_ang[0]) + screen_height/2 + camera_pos[1];  

            //The step size 
            x_step = (view_right[0] - view_left[0])/screen_width; 
            y_step = (view_right[1] - view_left[1])/screen_width;  

            for(int j = 0; j < screen_width; j++){

                height_on_screen = (camera_pos[2] + 580 - height[(int) view_left[0]][(int) view_left[1]][0]*1000) / i*200 + screen_height/3;

                //Drawing Each Dot
                if (height_on_screen < y_buffer[j]){
                    //Find the colour of that position
                    pixel *pixel_color; 
                    pixel_color = malloc(sizeof(pixel)*4);

                    for(int layer=0; layer < 3; layer++) {
                        get_terrain_colour(pixel_color, biome[layer], height[(int) view_left[0]][(int) view_left[1]][0]);
                    }

                    //Colour of the Box 
                    SDL_SetRenderDrawColor(renderer, pixel_color[0], pixel_color[1], pixel_color[2], 255);

                    SDL_RenderDrawLine(renderer, j, height_on_screen, j, y_buffer[j]);
                    
                    //SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255 - (height[(int) view_left[0]][(int) view_left[1]][1] - 320)*3);

                    //SDL_RenderDrawLine(renderer, j, height_on_screen, j, y_buffer[j]);


                    y_buffer[j] = height_on_screen;
                }

                view_left[0]  += x_step;
                view_left[1]  += y_step;
            } 

            i += dz;
            dz += 0.01; 
        }

        camera_ang[0] += 0.02;
        //camera_pos[0] += 1;

        //Render the screen
        SDL_RenderPresent(renderer); 

        // Clear the current renderer
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
    }

    //Destroy and Clode SDL after program finishes
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}

/*
//Loop conditionc
int run = 1; 

//Cube Points 
float points[8][3] = {
    {0 ,0 ,0},
    {1, 0 ,0},
    {1, 1, 0},
    {0, 1, 0},
    {0, 0, 1},
    {1, 0, 1},
    {1, 1, 1},
    {0, 1, 1}
};

//Where the translated points are stored
float projected[8][4];

//Main Function 
int main() {
    //Setting up SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window*window  = SDL_CreateWindow("Hopefully this is 3d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    SDL_Renderer*renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_Event window_event;

    //Projection Matrix Set-up
    float view_angle =    (float) camera_angle*(M_PI/180); //Turning Degrees to Radians 
    float aspect_ratio =  (float) screen_height/screen_width;
    float z_normalize =   (float) z_max_distance/(z_max_distance - z_min_distance);
    float z_other =       (float) (-z_max_distance*z_min_distance)/(z_max_distance - z_min_distance);
    float feild_of_view = (float) 1/tan(view_angle/2);

    float projection_matrix[4][4] = {
        {aspect_ratio*feild_of_view, 0,             0,           0},
        {0,                          feild_of_view, 0,           0},
        {0,                          0,             z_normalize, 1},
        {0,                          0,             z_other,     0}
    };

    while(run == 1){
        while (SDL_PollEvent(&window_event)){
            switch( window_event.type ){
                case SDL_QUIT:
                    run = 0;
                    break;
            }
        }

            //Colour of the Box 
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

            //Z axis rotation matrix
            float rotation_z[4][4] = {
                {cos(camera_ang[2]), sin(camera_ang[2]),  0, 0},
                {-sin(camera_ang[2]), cos(camera_ang[2]), 0, 0},
                {0,                  0,                   1, 0},
                {0,                  0,                   0, 1},     
            };

            //X axis rotation matrix
            float rotation_x[4][4] = {
                {1, 0,                  0,                   0},
                {0, cos(camera_ang[0]), sin(camera_ang[0]),  0},
                {0, -sin(camera_ang[0]), cos(camera_ang[0]), 0},
                {0, 0,                  0,                   1}
            };

            //FOr each point in the cube 
            for(int i = 0; i < 8; i++){
                
                //Reset projected to be same as the unprojected points 
                for (int j = 0; j < 3; j++){
                    projected[i][j] = points[i][j];
                };

                //Rotated z
                projected[i][0] = projected[i][0]*rotation_z[0][0] + projected[i][1]*rotation_z[1][0] + projected[i][2]*rotation_z[2][0] + rotation_z[3][0];
                projected[i][1] = projected[i][0]*rotation_z[0][1] + projected[i][1]*rotation_z[1][1] + projected[i][2]*rotation_z[2][1] + rotation_z[3][1];
                projected[i][2] = projected[i][0]*rotation_z[0][2] + projected[i][1]*rotation_z[1][2] + projected[i][2]*rotation_z[2][2] + rotation_z[3][2];
                projected[i][3] = projected[i][0]*rotation_z[0][3] + projected[i][1]*rotation_z[1][3] + projected[i][2]*rotation_z[2][3] + rotation_z[3][3];

                if (projected[i][3] != 0){
                    projected[i][0] /= projected[i][3];
                    projected[i][1] /= projected[i][3];
                    projected[i][2] /= projected[i][3];
                }

                //Rotated X
                projected[i][0] = projected[i][0]*rotation_x[0][0] + projected[i][1]*rotation_x[1][0] + projected[i][2]*rotation_x[2][0] + rotation_x[3][0];
                projected[i][1] = projected[i][0]*rotation_x[0][1] + projected[i][1]*rotation_x[1][1] + projected[i][2]*rotation_x[2][1] + rotation_x[3][1];
                projected[i][2] = projected[i][0]*rotation_x[0][2] + projected[i][1]*rotation_x[1][2] + projected[i][2]*rotation_x[2][2] + rotation_x[3][2];
                projected[i][3] = projected[i][0]*rotation_x[0][3] + projected[i][1]*rotation_x[1][3] + projected[i][2]*rotation_x[2][3] + rotation_x[3][3];
                
                if (projected[i][3] != 0){
                    projected[i][0] /= projected[i][3];
                    projected[i][1] /= projected[i][3];
                    projected[i][2] /= projected[i][3];
                }

                //Transating Away from the Camera
                projected[i][2] += 3;

                //Projection
                projected[i][0] = projected[i][0]*projection_matrix[0][0] + projected[i][1]*projection_matrix[1][0] + projected[i][2]*projection_matrix[2][0] + projection_matrix[3][0];
                projected[i][1] = projected[i][0]*projection_matrix[0][1] + projected[i][1]*projection_matrix[1][1] + projected[i][2]*projection_matrix[2][1] + projection_matrix[3][1];
                projected[i][2] = projected[i][0]*projection_matrix[0][2] + projected[i][1]*projection_matrix[1][2] + projected[i][2]*projection_matrix[2][2] + projection_matrix[3][2];
                projected[i][3] = projected[i][0]*projection_matrix[0][3] + projected[i][1]*projection_matrix[1][3] + projected[i][2]*projection_matrix[2][3] + projection_matrix[3][3];

                if (projected[i][3] != 0){
                    projected[i][0] /= projected[i][3];
                    projected[i][1] /= projected[i][3];
                    projected[i][2] /= projected[i][3];
                }

                //Translating X and Y our of negitive
                projected[i][0] += 1;
                projected[i][1] += 1;

                //Scaling up to correct size 
                projected[i][0] = projected[i][0]*screen_width/2;
                projected[i][1] = projected[i][1]*screen_height/2;
                
                //Drawing Each Dot
                SDL_RenderDrawPoint(renderer, projected[i][0], projected[i][1]);
            }

            //Drawing the Sides of the Cubes 
            //Front 
            SDL_RenderDrawLine(renderer, projected[0][0], projected[0][1] ,projected[1][0], projected[1][1]);
            SDL_RenderDrawLine(renderer, projected[1][0], projected[1][1] ,projected[2][0], projected[2][1]);
            SDL_RenderDrawLine(renderer, projected[2][0], projected[2][1] ,projected[3][0], projected[3][1]);
            SDL_RenderDrawLine(renderer, projected[3][0], projected[3][1] ,projected[0][0], projected[0][1]);

            //Back
            SDL_RenderDrawLine(renderer, projected[4][0], projected[4][1] ,projected[5][0], projected[5][1]);
            SDL_RenderDrawLine(renderer, projected[5][0], projected[5][1] ,projected[6][0], projected[6][1]);
            SDL_RenderDrawLine(renderer, projected[6][0], projected[6][1] ,projected[7][0], projected[7][1]);
            SDL_RenderDrawLine(renderer, projected[7][0], projected[7][1] ,projected[4][0], projected[4][1]);

            //Left
            SDL_RenderDrawLine(renderer, projected[1][0], projected[1][1] ,projected[2][0], projected[2][1]);
            SDL_RenderDrawLine(renderer, projected[2][0], projected[2][1] ,projected[6][0], projected[6][1]);
            SDL_RenderDrawLine(renderer, projected[6][0], projected[6][1] ,projected[5][0], projected[5][1]);
            SDL_RenderDrawLine(renderer, projected[5][0], projected[5][1] ,projected[1][0], projected[1][1]);

            //Right
            SDL_RenderDrawLine(renderer, projected[0][0], projected[0][1] ,projected[3][0], projected[3][1]);
            SDL_RenderDrawLine(renderer, projected[3][0], projected[3][1] ,projected[7][0], projected[7][1]);
            SDL_RenderDrawLine(renderer, projected[7][0], projected[6][1] ,projected[4][0], projected[4][1]);
            SDL_RenderDrawLine(renderer, projected[4][0], projected[4][1] ,projected[0][0], projected[0][1]);

            //Move the angle
            camera_ang[0] += 0.00;
            camera_ang[1] += 0.00;
            camera_ang[2] += 0.00;

            //Render the screen
            SDL_RenderPresent(renderer); 

            // Clear the current renderer
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        
    }

    //Destroy and Clode SDL after program finishes
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
*/

                //Projection
                //projected[0] = view_left[0]*projection_matrix[0][0] + view_left[1]*projection_matrix[1][0] + height[(int) view_left[0]][(int) view_left[1]][0]*projection_matrix[2][0] + projection_matrix[3][0];
                //projected[1] = view_left[0]*projection_matrix[0][3] + view_left[1]*projection_matrix[1][3] + height[(int) view_left[0]][(int) view_left[1]][0]*projection_matrix[2][3] + projection_matrix[3][3];

                //if (projected[1] != 0){
                //    projected[0] /= projected[1];
                //}

                //float height_on_screen = projected[0] - 100;
