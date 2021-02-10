/**
 * A naive implemention of vector add algorithm on FPGA
 * by Ye Zhifan 2021.2.8
 */
#include "vadd.h"

#define TILE_SIZE 256

extern "C"{
    void load(int flag, INTERFACE_WIDTH buffer_A[TILE_SIZE/WIDTH_FACTOR], INTERFACE_WIDTH buffer_B[TILE_SIZE/WIDTH_FACTOR], int offset,
                INTERFACE_WIDTH vector_A[VECTOR_LENGTH/WIDTH_FACTOR], INTERFACE_WIDTH vector_B[VECTOR_LENGTH/WIDTH_FACTOR]){
        if(flag)                        
        LOAD_LOOP: for(int j=0; j<TILE_SIZE/WIDTH_FACTOR; j++){
        #pragma HLS pipeline II=1   
            buffer_A[j] = vector_A[offset+j];
            buffer_B[j] = vector_B[offset+j];
        }            
    }

    void compute(int flag, INTERFACE_WIDTH buffer_A[TILE_SIZE/WIDTH_FACTOR], INTERFACE_WIDTH buffer_B[TILE_SIZE/WIDTH_FACTOR],
                    INTERFACE_WIDTH buffer_C[TILE_SIZE/WIDTH_FACTOR]){
        if(flag)                
        COMPUTE_LOOP: for(int j=0; j<TILE_SIZE/WIDTH_FACTOR; j++){
        #pragma HLS UNROLL
            INNER_COMPUTER_LOOP: for(int k=0; k<WIDTH_FACTOR; k++){
            #pragma HLS UNROLL
                int k_idx = k*32;
                uint32_t element_A = buffer_A[j].range(k_idx+31, k_idx);
                uint32_t element_B = buffer_B[j].range(k_idx+31, k_idx);
                float value_A = *((float*)(&element_A));
                float value_B = *((float*)(&element_B));
                float sum = value_A + value_B;
                buffer_C[j].range(k_idx+31, k_idx) = *((uint32_t*)(&sum));
            } 
        }
    }

    void store(int flag, INTERFACE_WIDTH buffer_C[TILE_SIZE/WIDTH_FACTOR], INTERFACE_WIDTH vector_C[VECTOR_LENGTH/WIDTH_FACTOR], int offset){
        if(flag)
        STORE_LOOP: for(int j=0; j<TILE_SIZE/WIDTH_FACTOR; j++){
        #pragma HLS pipeline II=1    
            vector_C[offset+j] = buffer_C[j];
        }
    }


    void workload(
        INTERFACE_WIDTH vector_A[VECTOR_LENGTH/WIDTH_FACTOR],
        INTERFACE_WIDTH vector_B[VECTOR_LENGTH/WIDTH_FACTOR],
        INTERFACE_WIDTH vector_C[VECTOR_LENGTH/WIDTH_FACTOR]
    ) {
        #pragma HLS INTERFACE m_axi port=vector_A offset=slave bundle=gmem1
        #pragma HLS INTERFACE s_axilite port=vector_A bundle=control
        #pragma HLS INTERFACE m_axi port=vector_B offset=slave bundle=gmem2
        #pragma HLS INTERFACE s_axilite port=vector_B bundle=control
        #pragma HLS INTERFACE m_axi port=vector_C offset=slave bundle=gmem3
        #pragma HLS INTERFACE s_axilite port=vector_C bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control


        INTERFACE_WIDTH buffer_A1[TILE_SIZE/WIDTH_FACTOR];
        #pragma HLS ARRAY_PARTITION variable=buffer_A1 complete
        INTERFACE_WIDTH buffer_B1[TILE_SIZE/WIDTH_FACTOR];
        #pragma HLS ARRAY_PARTITION variable=buffer_B1 complete
        INTERFACE_WIDTH buffer_C1[TILE_SIZE/WIDTH_FACTOR];   
        #pragma HLS ARRAY_PARTITION variable=buffer_C1 complete 

        INTERFACE_WIDTH buffer_A2[TILE_SIZE/WIDTH_FACTOR];
        #pragma HLS ARRAY_PARTITION variable=buffer_A2 complete
        INTERFACE_WIDTH buffer_B2[TILE_SIZE/WIDTH_FACTOR];
        #pragma HLS ARRAY_PARTITION variable=buffer_B2 complete
        INTERFACE_WIDTH buffer_C2[TILE_SIZE/WIDTH_FACTOR];   
        #pragma HLS ARRAY_PARTITION variable=buffer_C2 complete 
        
        for(int i=0; i< VECTOR_LENGTH/TILE_SIZE + 2; i++){
            //burst_read
            int offset = TILE_SIZE*i/WIDTH_FACTOR;
            int flag_load = i < VECTOR_LENGTH/TILE_SIZE;
            int flag_compute = i > 0 && (i < VECTOR_LENGTH/TILE_SIZE + 1);
            int flag_store = i > 1;

            if(i%2 == 0){
                load(flag_load, buffer_A1, buffer_B1, offset, vector_A, vector_B);
                compute(flag_compute, buffer_A2, buffer_B2, buffer_C2);
                store(flag_store, buffer_C1, vector_C, offset - 2*TILE_SIZE/WIDTH_FACTOR);
            }
            else{
                load(flag_load, buffer_A2, buffer_B2, offset, vector_A, vector_B);
                compute(flag_compute, buffer_A1, buffer_B1, buffer_C1);
                store(flag_store, buffer_C2, vector_C, offset - 2*TILE_SIZE/WIDTH_FACTOR);
            }
        }
    }
}