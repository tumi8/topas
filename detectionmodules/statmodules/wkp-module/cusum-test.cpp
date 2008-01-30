#include "cusum-test.h"

// update the cumulative sum g
double cusum(int64_t X, double beta, double g) {

  g = g + (X - beta);

  // clamp g to zero if negative
  if ( g < 0.0)
    g = 0.0;

  return g;
}
