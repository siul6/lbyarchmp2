#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include "daxpy.h"

#define RUNS 30
#define CHECK_SIZE 1024

typedef void (*kernel_func)(int, double, const double *, const double *, double *);

unsigned int seed = 123456789;

double random_num(void)
{
    seed = seed * 1664525u + 1013904223u;
    return ((int)(seed % 20001u) - 10000) / 1000.0;
}

void fill_vectors(int n, double *x, double *y)
{
    int i;

    for (i = 0; i < n; i++) {
        x[i] = random_num();
        y[i] = random_num();
    }
}

double time_kernel(kernel_func kernel, int n, double a, const double *x, const double *y, double *z)
{
    LARGE_INTEGER freq, start, end;
    double total = 0.0;
    int i;

    QueryPerformanceFrequency(&freq);
    kernel(n, a, x, y, z); /* warm-up run */

    for (i = 0; i < RUNS; i++) {
        QueryPerformanceCounter(&start);
        kernel(n, a, x, y, z);
        QueryPerformanceCounter(&end);
        total += (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    }

    return total / RUNS;
}

int check_c_output(int n, double a, const double* x, const double* y, const double* z)
{
    int i;

    for (i = 0; i < n; i++) {
        double answer = a * x[i] + y[i];
        double difference = fabs(answer - z[i]);
        double limit = 1e-12 * (fabs(answer) + 1.0);

        if (difference > limit)
            return i;
    }

    return -1;
}

int check_output(int n, double a, const double *x, const double *y, const double *z)
{
    double answer[CHECK_SIZE];
    int start, count, i;

    for (start = 0; start < n; start += CHECK_SIZE) {
        count = CHECK_SIZE;

        if (n - start < CHECK_SIZE)
            count = n - start;

        daxpy_c(count, a, x + start, y + start, answer);

        for (i = 0; i < count; i++) {
            double difference = fabs(answer[i] - z[start + i]);
            double limit = 1e-12 * (fabs(answer[i]) + 1.0);

            if (difference > limit)
                return start + i;
        }
    }

    return -1;
}

void print_first_ten(const char *name, const double *z)
{
    int i;

    printf("%s\n", name);

    for (i = 0; i < 10; i++)
        printf("z[%d] = %.6f\n", i, z[i]);
}

int main(void)
{
    int powers[] = {20, 24, 28};
    int test;

    printf("daxpy: z[i] = a * x[i] + y[i]\n");
    printf("runs per kernel: %d\n", RUNS);

    for (test = 0; test < 3; test++) {
        int n = 1 << powers[test];
        size_t bytes = (size_t)n * sizeof(double);
        double *x = malloc(bytes);
        double *y = malloc(bytes);
        double *z = malloc(bytes);
        double a, c_time, asm_time;
        int wrong_index;

        printf("\n************************************\n\n");
        printf("n = 2^%d = %d\n", powers[test], n);

        if (x == NULL || y == NULL || z == NULL) {
            printf("memory allocation failed. lower the last power.\n");
            free(x);
            free(y);
            free(z);
            continue;
        }

        a = random_num();

        if (a == 0.0)
            a = 2.0;

        fill_vectors(n, x, y);
        printf("a = %.6f\n\n", a);

        daxpy_c(n, a, x, y, z);
        print_first_ten("c output:", z);

        wrong_index = check_c_output(n, a, x, y, z);

        if (wrong_index == -1)
            printf("\nc correctness check: passed\n");
        else {
            printf("\nc correctness check: failed at index %d\n", wrong_index);
            free(x);
            free(y);
            free(z);
            return 1;
        }

        c_time = time_kernel(daxpy_c, n, a, x, y, z);
        printf("c average: %.6f seconds\n", c_time);

        printf("\n");

        daxpy_asm(n, a, x, y, z);
        print_first_ten("x86-64 output:", z);

        wrong_index = check_output(n, a, x, y, z);

        if (wrong_index == -1)
            printf("\nx86-64 correctness check: passed\n");
        else {
            printf("\nx86-64 correctness check: failed at index %d\n", wrong_index);
            free(x);
            free(y);
            free(z);
            return 1;
        }

        asm_time = time_kernel(daxpy_asm, n, a, x, y, z);
        printf("x86-64 average: %.6f seconds\n", asm_time);

        // printf("speedup: %.3fx\n", c_time / asm_time);

        free(x);
        free(y);
        free(z);
    }

    return 0;
}
