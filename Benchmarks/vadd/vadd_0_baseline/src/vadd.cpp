/**
 * A naive implemention of vector add algorithm on FPGA
 * by Ye Zhifan 2021.2.8
 */
#include "vadd.h"

extern "C"{
    void workload(
        float vector_A[VECTOR_LENGTH],
        float vector_B[VECTOR_LENGTH],
        float vector_C[VECTOR_LENGTH]
    ) {
        #pragma HLS INTERFACE m_axi port=vector_A offset=slave bundle=gmem
        #pragma HLS INTERFACE s_axilite port=vector_A bundle=control
        #pragma HLS INTERFACE m_axi port=vector_B offset=slave bundle=gmem
        #pragma HLS INTERFACE s_axilite port=vector_B bundle=control
        #pragma HLS INTERFACE m_axi port=vector_C offset=slave bundle=gmem
        #pragma HLS INTERFACE s_axilite port=vector_C bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control


        for(int i=0; i < VECTOR_LENGTH; i++){
            vector_C[i] = vector_A[i] + vector_B[i];
        }
    }
}