#ifndef KNN_H
#define KNN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define COALESCING_5_512bit

#ifdef COALESCING_5_512bit
#include "ap_int.h"
#include <inttypes.h>
    const int DWIDTH = 512;
#define INTERFACE_WIDTH ap_uint<DWIDTH>
    const int WIDTH_FACTOR = DWIDTH/32;
#endif 



const int VECTOR_LENGTH = 4096;

struct bench_args_t {
    float vector_A[VECTOR_LENGTH];
    float vector_B[VECTOR_LENGTH];
    float vector_C[VECTOR_LENGTH];
};
#endif