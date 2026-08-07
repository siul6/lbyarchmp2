#ifndef DAXPY_H
#define DAXPY_H

void daxpy_c(int n, double a, const double *x, const double *y, double *z);
void daxpy_asm(int n, double a, const double *x, const double *y, double *z);

#endif
