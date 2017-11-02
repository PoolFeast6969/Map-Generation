/* Map Generation */

/* Libaries */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define size 300

/* Variables */
/* int size = 300;  The x and y size of the map */
int border = 2; /* The size of the ocean border */
int flooding = 2.5; /* How high the ocean rises */
int generations = 10; /* Number of times the changes will be repeated */
int cleaning = 3; /* The number of cleaning up cycles run */

int higher = 0;
int lower = 0;

int sea = 0;
int ground = 0;
int avg_ground = 0;

int i, int j, int k;

float z[size][size]; /* Making the matrix which will store the altitudes */

srand(time(NULL)); /* Intiallizes the random shite */

float random(); /* Generates random numbers between 0 and 1 */

int main () {
    /* This is assigning a random float between 1 and 0 to each cell */
    for(i = 0; i <= size; i++)
    {
      for(j = 0; j <= size; j++)
      {
        z[i][j] = random();
      }
    }

    /* Running the map generator */
    for(k = 1; k <= generations; k++)
    {
      for(i = 1; i < size; i++)
      {
        for(j = 1; j < size; j++)
        {
          /* This section is checking the neighbors */
          higher = 0;
          lower = 0;

          if (z[i][j] > z[i][j+1]) { higher += 1; }
          else { lower += 1; }

          if (z[i][j] > z[i][j-1]) { higher += 1; }
          else { lower += 1; }

          if (z[i][j] > z[i+1][j+1]) { higher += 1; }
          else { lower += 1; }

          if (z[i][j] > z[i+1][j-1]) { higher += 1; }
          else { lower += 1; }

          if (z[i][j] > z[i+1][j]) { higher += 1; }
          else { lower += 1; }

          if (z[i][j] > z[i-1][j]) { higher += 1; }
          else { lower += 1; }

          if (z[i][j] > z[i-1][j+1]) { higher += 1; }
          else { lower += 1; }

          if (z[i][j] > z[i-1][j-1]) { higher += 1; }
          else { lower += 1; }

          /* This section is aducting the altitude bases on surronding cells */
          /* Note these adjustment values where found via trial and error */
          /* and work best for map size 100 to 300 */
          if (lower > 4 && higher < 8) {
            z[i][j] += 0.6;
          } else if (lower == 8) {
            z[i][j] += 1.1;
          } else if (higher > 5) {
            z[i][j] -= 1.2;
          }
        }
      }
    }

    /* Raising the ocean */
    for(i = 0; i <= size; i++){
      for(j = 0; j <= size; j++){
        z[i][j] -= flooding
      }
    }

    /* Cleaning the map */
    for(k = 0; k <= cleaning; k++)
    {
      for(i = 0; i <= size; i++)
      {
        for(j = 0; j <= size; j++)
        {
           sea = 0;
           ground = 0;
           avg_ground = 0;

           if (z[i][j+1] == 0) {
             sea += 1;
           } else {
             ground += 1;
             avg_ground += z[i][j+1];
           }

           if (z[i][j-1] == 0) {
             sea += 1;
           }  else {
             ground += 1;
             avg_ground += z[i][j-1];
           }

           if (z[i+1][j+1] == 0) {
             sea += 1;
           } else {
             ground += 1;
             avg_ground += z[i+1][j+1];
           }

           if (z[i-1][j+1] == 0) {
             sea += 1;
           } else {
             ground += 1;
             avg_ground += z[i-1][j+1];
           }

           if (z[i][j+1] == 0) {
             sea += 1;
           } else {
             ground += 1;
             avg_ground += z[i+1][j-1];
           }

           if (z[i][j+1] == 0) {
             sea += 1;
           } else {
            ground += 1;
            avg_ground += z[i-1][j-1];
           }

           if (z[i][j+1] == 0) {
              sea += 1;
           } else {
             ground += 1;
             avg_ground += z[i-1][j];
           }

           if (z[i][j+1] == 0) {
             sea += 1;
           } else {
             ground += 1;
             avg_ground += z[i+1][j];
           }
        }
      }
    }
}

float random()
{
  return ((double)rand()/(double)(RAND_MAX));
}
