#include "params.h"

void Params::init(int size)
{
    cov = gsl_matrix_calloc (size, size);
    evec = gsl_matrix_calloc (size, size);
    // initialize sumsOfMetrics and sumsOfProducts
    for (int i = 0; i < size; i++) {
	sumsOfMetrics.push_back(0.0);
	stddevs.push_back(0.0);
	std::vector<double> v;
	for (int j = 0; j < size; j++)
	    v.push_back(0.0);
	sumsOfProducts.push_back(v);
    }
}
