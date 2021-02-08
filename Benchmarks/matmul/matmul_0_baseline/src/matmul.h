#ifndef MATMUL_H
#define MATMUL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


const int MARTRIX_SIZE = 32;

struct bench_args_t {
    float MATRIX_A[MARTRIX_SIZE*MARTRIX_SIZE];
    float MATRIX_B[MARTRIX_SIZE*MARTRIX_SIZE];
    float MATRIX_C[MARTRIX_SIZE*MARTRIX_SIZE];
};
#endif