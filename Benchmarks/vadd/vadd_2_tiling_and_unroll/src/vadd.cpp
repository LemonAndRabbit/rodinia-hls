/**
 * A naive implemention of vector add algorithm on FPGA
 * by Ye Zhifan 2021.2.8
 */
#include "vadd.h"

#define TILE_SIZE 256

extern "C"{
    void workload(
        float vector_A[VECTOR_LENGTH],
        float vector_B[VECTOR_LENGTH],
        float vector_C[VECTOR_LENGTH]
    ) {
        #pragma HLS INTERFACE m_axi port=vector_A offset=slave bundle=gmem1
        #pragma HLS INTERFACE s_axilite port=vector_A bundle=control
        #pragma HLS INTERFACE m_axi port=vector_B offset=slave bundle=gmem2
        #pragma HLS INTERFACE s_axilite port=vector_B bundle=control
        #pragma HLS INTERFACE m_axi port=vector_C offset=slave bundle=gmem3
        #pragma HLS INTERFACE s_axilite port=vector_C bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control


        float buffer_A[TILE_SIZE];
        #pragma HLS ARRAY_PARTITION variable=buffer_A complete
        float buffer_B[TILE_SIZE];
        #pragma HLS ARRAY_PARTITION variable=buffer_B complete
        float buffer_C[TILE_SIZE];   
        #pragma HLS ARRAY_PARTITION variable=buffer_C complete 
        
        for(int i=0; i< VECTOR_LENGTH/TILE_SIZE; i++){
            //burst_read
            int offset = TILE_SIZE*i;
            for(int j=0; j<TILE_SIZE; j++){
            #pragma HLS pipeline II=1   
                buffer_A[j] = vector_A[offset+j];
                buffer_B[j] = vector_B[offset+j];
            }
            
            //compute with fully unrolling
            for(int j=0; j<TILE_SIZE; j++){
            #pragma HLS UNROLL
                buffer_C[j] = buffer_A[j] + buffer_B[j];  
            }

            //write back
            for(int j=0; j<TILE_SIZE; j++){
            #pragma HLS pipeline II=1    
                vector_C[offset+j] = buffer_C[j];
            }
        }
    }
}