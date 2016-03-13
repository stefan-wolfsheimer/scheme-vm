#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "util/hash_table.h"

int main(int argc, const char ** argv)
{
  size_t n_max = 100000;
  size_t n_min = 0;
  size_t n_steps = 10000;
  size_t i;
  size_t seed = 1;
  size_t n = 50000;
  double sum = 0;
  for(i = 0; i < n_steps; i++) 
  {
    double r = 0.5 + 0.1*0.5 * sin(2.0*M_PI * i / (double)n_steps);
    double x = (double)rand() / RAND_MAX;
    if(x < r) 
    {
      if(n < n_max-1) 
      {
        n++;
      }
    }
    else 
    {
      if(n > n_min) 
      {
        n--;
      }
    }
    printf("%f %f %d\n",x, r, n);
  }
  return 0;
}
