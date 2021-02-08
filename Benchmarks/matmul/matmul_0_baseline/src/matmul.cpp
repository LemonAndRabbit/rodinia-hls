/**
 * A naive implemention of matrix multiply algorithm on FPGA
 * by Ye Zhifan 2021.2.8
 */
#include "matmul.h"


extern "C" {
    void workload(
        float MARTRIX_A[MARTRIX_SIZE*MARTRIX_SIZE],
        float MARTRIX_B[MARTRIX_SIZE*MARTRIX_SIZE],
        float MARTRIX_C[MARTRIX_SIZE*MARTRIX_SIZE]
    ) {
        #pragma HLS INTERFACE m_axi port=MARTRIX_A offset=slave bundle=gmem
        #pragma HLS INTERFACE s_axilite port=MARTRIX_A bundle=control
        #pragma HLS INTERFACE m_axi port=MARTRIX_B offset=slave bundle=gmem
        #pragma HLS INTERFACE s_axilite port=MARTRIX_B bundle=control
        #pragma HLS INTERFACE m_axi port=MARTRIX_C offset=slave bundle=gmem
        #pragma HLS INTERFACE s_axilite port=MARTRIX_C bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control

        float sum = 0.0;
        for(int i = 0; i < MARTRIX_SIZE; i++ ) {
            for (int j = 0; j < MARTRIX_SIZE; ++j){
                sum = 0.0;
                for(int k = 0; k < MARTRIX_SIZE; k++){
                    sum += MARTRIX_A[i * MARTRIX_SIZE + k] * MARTRIX_B[k * MARTRIX_SIZE + j];
                }
                MARTRIX_C[i * MARTRIX_SIZE + j] = sum;
            }
        }
    }
}