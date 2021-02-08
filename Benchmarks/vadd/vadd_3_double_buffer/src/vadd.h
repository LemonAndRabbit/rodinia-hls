#ifndef KNN_H
#define KNN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


const int VECTOR_LENGTH = 4096;

struct bench_args_t {
    float vector_A[VECTOR_LENGTH];
    float vector_B[VECTOR_LENGTH];
    float vector_C[VECTOR_LENGTH];
};
#endif