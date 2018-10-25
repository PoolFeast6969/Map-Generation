#include<stdlib.h>
#include<stdio.h>
#include<math.h>

int repeat = -1;

int permutation[] = { 151,160,137,91,90,15,					// Hash lookup table as defined by Ken Perlin.  This is a randomly
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,	// arranged array of all numbers from 0-255 inclusive.
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

int p[512];

double grad(int hash, double x, double y, double z) {
    int h = hash & 15;									// Take the hashed value and take the first 4 bits of it (15 == 0b1111)
    double u = h < 8 /* 0b1000 */ ? x : y;				// If the most significant bit (MSB) of the hash is 0 then set u = x.  Otherwise y.
    double v;											// In Ken Perlin's original implementation this was another conditional operator (?:).  I
                                                        // expanded it for readability.
    if(h < 4 /* 0b0100 */)								// If the first and second significant bits are 0 set v = y
        v = y;
    else if(h == 12 /* 0b1100 */ || h == 14 /* 0b1110*/)// If the first and second significant bits are 1 set v = x
        v = x;
    else 												// If the first and second significant bits are not equal (0/1, 1/0) set v = z
        v = z;
    return ((h&1) == 0 ? u : -u)+((h&2) == 0 ? v : -v); // Use the last 2 bits to decide if u and v are positive or negative.  Then return their addition.
}

double fade(double t) {
                                                        // Fade function as defined by Ken Perlin.  This eases coordinate values
                                                        // so that they will "ease" towards integral values.  This ends up smoothing
                                                        // the final output.
    return t * t * t * (t * (t * 6 - 15) + 10);			// 6t^5 - 15t^4 + 10t^3
}

double lerp(double a, double b, double x) {
    return a + x * (b - a);
}

int inc(int num) {
    num++;
    if (repeat > 0) num %= repeat;
    return num;
}

double perlin(double x, double y, double z) {   
    if(repeat > 0) {									// If we have any repeat on, change the coordinates to their "local" repetitions
        x = fmod(x,repeat);
        y = fmod(y,repeat);
        z = fmod(z,repeat);
    }

    int xi = (int)x & 255;								// Calculate the "unit cube" that the point asked will be located in
    int yi = (int)y & 255;								// The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
    int zi = (int)z & 255;								// plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.
    double xf = x-(int)x;								// We also fade the location to smooth the result.
    double yf = y-(int)y;
    double zf = z-(int)z;
    double u = fade(xf);
    double v = fade(yf);
    double w = fade(zf);
                                                        
    int aaa, aba, aab, abb, baa, bba, bab, bbb;
    aaa = p[p[p[    xi ]+    yi ]+    zi ];
    aba = p[p[p[    xi ]+inc(yi)]+    zi ];
    aab = p[p[p[    xi ]+    yi ]+inc(zi)];
    abb = p[p[p[    xi ]+inc(yi)]+inc(zi)];
    baa = p[p[p[inc(xi)]+    yi ]+    zi ];
    bba = p[p[p[inc(xi)]+inc(yi)]+    zi ];
    bab = p[p[p[inc(xi)]+    yi ]+inc(zi)];
    bbb = p[p[p[inc(xi)]+inc(yi)]+inc(zi)];

    double x1, x2, y1, y2;
    x1 = lerp(	grad (aaa, xf  , yf  , zf),				// The gradient function calculates the dot product between a pseudorandom
                grad (baa, xf-1, yf  , zf),				// gradient vector and the vector from the input coordinate to the 8
                u);										// surrounding points in its unit cube.
    x2 = lerp(	grad (aba, xf  , yf-1, zf),				// This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
                grad (bba, xf-1, yf-1, zf),				// values we made earlier.
                    u);
                    
    y1 = lerp(x1, x2, v);

    x1 = lerp(	grad (aab, xf  , yf  , zf-1),
                grad (bab, xf-1, yf  , zf-1),
                u);
    x2 = lerp(	grad (abb, xf  , yf-1, zf-1),
                grad (bbb, xf-1, yf-1, zf-1),
                u);

    y2 = lerp (x1, x2, v);
    
    return (lerp (y1, y2, w)+1)/2;						// For convenience we bound it to 0 - 1 (theoretical min/max before is -1 - 1)
}

double OctavePerlin(double x, double y, double z, int octaves, double persistence) {
    double total = 0;
    double frequency = 1;
    double amplitude = 1;
    double maxValue = 0;			// Used for normalizing result to 0.0 - 1.0
    for(int i=0;i<octaves;i++) {
        total += perlin(x * frequency, y * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude = amplitude*0.6;//6persistence;
        frequency = frequency*1.9;
    }
    
    return total/maxValue;
}

int generate_terrain (int size, double x_layer, double y_layer, double z_layer, double ***z, double ui, double uj, double uk) {
    // Scaling Factors 
    float scaling[] = {1};
    int octaves = 12;
    float zoom = 5; //Zoom scale, Bigger Zooms in, Smaller Zooms
        
    // A thing that does c things that it needs
    for (int i = 0; i < 256; i++) {
        p[256+i] = p[i] = permutation[i];
    }


    // Fill array
    for(int x = x_layer; x < x_layer + size; x++) {
        double x_noise = x/(double)size*4;
        for(int y = y_layer; y < y_layer + size; y++) {
            double y_noise = y/(double)size*4;
            
            //Adding Altitudes for different frequencies 
            double height = OctavePerlin(x_noise, y_noise, z_layer, octaves, 1.0);   
            z[x][y][0] = height;

            ///
            /// Calculating Normal Vector
            ///

            //Defining The 3 points in space 
            double P1_x = x_noise;
            double P1_y = y_noise;
            double P1_z = z[x][y][0];

            double P2_x = (x - 1)/(double)size*4;
            double P2_y = y_noise;
            double P2_z;

            double P3_x = x_noise;
            double P3_y = (y - 1)/(double)size*4;
            double P3_z;

            if (x == 0){
                P2_z = OctavePerlin((x - 1)/(double)size*4, (y)/(double)size*4, z_layer, octaves, 1.0);
            } else if (y == 0) {    
                P3_z = OctavePerlin((x)/(double)size*4, (y - 1)/(double)size*4, z_layer, octaves, 1.0);
            } else {
                P2_z = z[x - 1][y][0];
                P3_z = z[x][y -1][0];
            }
            
            //Creating the 2 vectors from the 3 points 
            double vector1_i = P2_x - P1_x;
            double vector1_j = P2_y - P1_y;
            double vector1_k = P2_z - P1_z; 

            double vector2_i = P3_x - P1_x;
            double vector2_j = P3_y - P1_y;
            double vector2_k = P3_z - P1_z;

            //The normal vector is found with cross product
            double normal_i = vector1_j*vector2_k - vector1_k*vector2_j;
            double normal_j = vector1_k*vector2_i - vector1_i*vector2_k;
            double normal_k = vector1_i*vector2_j - vector1_j*vector2_i;
            
            //Finding norm which is the vectors magnitude
            double norm_normal = pow(pow(normal_i,2) + pow(normal_j,2) + pow(normal_k,2),0.5);
            double norm_light = pow(pow(ui,2) + pow(uj,2) + pow(uk,2),0.5);

            //Calculating the angle between the normal and light vector 
            double theta = acos((normal_i*ui+normal_j*uj+normal_k*uk)/(norm_normal*norm_light));

            //Calculating Alpha value 
            double alpha = (255/2.87979)*abs(theta) - (255/2.87979)*0.261799;

            printf("%f \n", z[x][y][0]);

            z[x][y][1] = alpha;
        }
    }
    return 0;
} 
