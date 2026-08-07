#include "daxpy.h"

__declspec(noinline) void daxpy_c(int n, double a, const double *x, const double *y, double *z)
{
    int i;

    for (i = 0; i < n; i++)
        z[i] = a * x[i] + y[i];
}
