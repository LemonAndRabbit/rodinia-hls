#include "lc_mgvf.h"

#define __kernel
#define __global

float heaviside(float x) {
    // A simpler, faster approximation of the Heaviside function
    float out = 0.0;
    if ((float)x > (float)-0.0001) out = (float)0.5;
    if ((float)x >  (float)0.0001) out = (float)1.0;
    return out; 
}

float lc_mgvf_stencil_core(float c, float ul, float u, float ur, float l, float r, float dl, float d, float dr, float vI)
{
    float UL = (float)ul - (float)c;
    float U  = (float)u  - (float)c;
    float UR = (float)ur - (float)c;
    float L  = (float)l  - (float)c;
    float R  = (float)r  - (float)c;
    float DL = (float)dl - (float)c;
    float D  = (float)d  - (float)c;
    float DR = (float)dr - (float)c;

    float vHe = (float)c + (float)MU_O_LAMBDA * (float)(
                                   (float)heaviside(UL) * (float)UL + 
                                   (float)heaviside(U)  * (float)U  + 
                                   (float)heaviside(UR) * (float)UR + 
                                   (float)heaviside(L)  * (float)L  + 
                                   (float)heaviside(R)  * (float)R  + 
                                   (float)heaviside(DL) * (float)DL + 
                                   (float)heaviside(D)  * (float)D  + 
                                   (float)heaviside(DR) * (float)DR 
                                   );

    float new_val = (float)vHe - ((float)ONE_O_LAMBDA * (float)vI * (float)((float)vHe - (float)vI));

    return new_val;
}

void lc_mgvf(INTERFACE_WIDTH result[TILE_ROWS * GRID_COLS/WIDTH_FACTOR], INTERFACE_WIDTH imgvf[(TILE_ROWS + 2) * GRID_COLS/WIDTH_FACTOR], INTERFACE_WIDTH I[TILE_ROWS * GRID_COLS/WIDTH_FACTOR], int which_boundary)
{
	int cols = GRID_COLS;
	int rows = GRID_ROWS;
	float imgvf_rf[GRID_COLS * (2 * MAX_RADIUS) + MAX_RADIUS * 2 + PARA_FACTOR];
#pragma HLS array_partition variable=imgvf_rf cyclic dim=0 factor=64

	int i;

    int input_bound = GRID_COLS * (2 * MAX_RADIUS) + MAX_RADIUS + PARA_FACTOR;
    for (i = 0; i < input_bound; i+= WIDTH_FACTOR) {
#pragma HLS unroll
        for (int j = 0; j < WIDTH_FACTOR && i + j < input_bound; j++){
#pragma HLS unroll
            unsigned int range_idx = j*32;
            uint32_t temp_imgvf = imgvf[i/WIDTH_FACTOR].range(range_idx+31, range_idx);
            float read_imgvf = *((float*)(&temp_imgvf));
            imgvf_rf[i + j + MAX_RADIUS] = read_imgvf;
        }
    }


	//for (i = -(GRID_COLS * (2 * MAX_RADIUS) + MAX_RADIUS + PARA_FACTOR) / PARA_FACTOR; i < GRID_COLS / PARA_FACTOR * TILE_ROWS; i++) {
    for (i = 0; i < GRID_COLS / PARA_FACTOR * TILE_ROWS; i++) {
		int k;
#pragma HLS pipeline II=1

		for (k = 0; k < PARA_FACTOR; k++) {
#pragma HLS unroll

			float ul[PARA_FACTOR], u[PARA_FACTOR], ur[PARA_FACTOR], l[PARA_FACTOR], c[PARA_FACTOR], r[PARA_FACTOR], dl[PARA_FACTOR], d[PARA_FACTOR], dr[PARA_FACTOR], vI[PARA_FACTOR];

			int is_top = (which_boundary == TOP) && (i < GRID_COLS / PARA_FACTOR);
			int is_right = (i % (GRID_COLS / PARA_FACTOR) == (GRID_COLS / PARA_FACTOR - 1)) && (k == PARA_FACTOR - 1);
			int is_bottom = (which_boundary == BOTTOM) && (i >= GRID_COLS / PARA_FACTOR * (TILE_ROWS - 1));
			int is_left = (i % (GRID_COLS / PARA_FACTOR) == 0) && (k == 0);

			c[k] = imgvf_rf[GRID_COLS * (0) + 0 + k + GRID_COLS + MAX_RADIUS];
			ul[k] = (is_top || is_left) ? c[k] : imgvf_rf[GRID_COLS * (-1) + -1 + k + GRID_COLS + MAX_RADIUS];
			u[k] = (is_top) ? c[k] : imgvf_rf[GRID_COLS * (-1) + 0 + k + GRID_COLS + MAX_RADIUS];
			ur[k] = (is_top || is_right) ? c[k] : imgvf_rf[GRID_COLS * (-1) + 1 + k + GRID_COLS + MAX_RADIUS];
			l[k] = (is_left) ? c[k] : imgvf_rf[GRID_COLS * (0) + -1 + k + GRID_COLS + MAX_RADIUS];
			r[k] = (is_right) ? c[k] : imgvf_rf[GRID_COLS * (0) + 1 + k + GRID_COLS + MAX_RADIUS];
			dl[k] = (is_bottom || is_left) ? c[k] : imgvf_rf[GRID_COLS * (1) + -1 + k + GRID_COLS + MAX_RADIUS];
			d[k] = (is_bottom) ? c[k] : imgvf_rf[GRID_COLS * (1) + 0 + k + GRID_COLS + MAX_RADIUS];
			dr[k] = (is_bottom || is_right) ? c[k] : imgvf_rf[GRID_COLS * (1) + 1 + k + GRID_COLS + MAX_RADIUS];

            unsigned int idx = (i*PARA_FACTOR+k) / WIDTH_FACTOR;
            unsigned int range_idx =  (i*PARA_FACTOR+k) % WIDTH_FACTOR * 32;

            //vI[k] = I[i * PARA_FACTOR + k];
            uint32_t temp_I = I[idx].range(range_idx+31, range_idx);
            float read_I = *((float*)(&temp_I));
            vI[k] = read_I;

            float res = lc_mgvf_stencil_core(c[k], ul[k], u[k], ur[k], l[k], r[k], dl[k], d[k], dr[k], vI[k]);
			result[idx].range(range_idx+31, range_idx) = *((uint32_t *)(&res));

         
/*
            float c[PARA_FACTOR];
            unsigned int idx = (i*PARA_FACTOR+k) / WIDTH_FACTOR;
            unsigned int range_idx =  (i*PARA_FACTOR+k) % WIDTH_FACTOR * 32;

            c[k] = imgvf_rf[GRID_COLS * (0) + 0 + k + GRID_COLS + MAX_RADIUS];
            float res = c[k];
			result[idx].range(range_idx+31, range_idx) = *((uint32_t *)(&res));
*/

		}

		for (k = 0; k < GRID_COLS * (2 * MAX_RADIUS) + MAX_RADIUS * 2; k++) {
#pragma HLS unroll
			imgvf_rf[k] = imgvf_rf[k + PARA_FACTOR];
		}

		for (k = 0; k < PARA_FACTOR; k += WIDTH_FACTOR) {
#pragma HLS unroll
            for(int g = 0; g < WIDTH_FACTOR && g+k < PARA_FACTOR; g++){
#pragma HLS unroll                
                unsigned int idx = (GRID_COLS * (2 * MAX_RADIUS) + MAX_RADIUS + (i + 1) * PARA_FACTOR + k + g)/WIDTH_FACTOR;
                unsigned int range_idx = (GRID_COLS * (2 * MAX_RADIUS) + MAX_RADIUS + (i + 1) * PARA_FACTOR + k + g)%WIDTH_FACTOR*32;
                uint32_t temp_imgvf = imgvf[idx].range(range_idx+31, range_idx);
                float read_imgvf = *((float*)(&temp_imgvf));
                imgvf_rf[GRID_COLS * (2 * MAX_RADIUS) + MAX_RADIUS * 2 + g + k] = read_imgvf;
            }
			//imgvf_rf[GRID_COLS * (2 * MAX_RADIUS) + MAX_RADIUS * 2 + k] = imgvf[GRID_COLS * (2 * MAX_RADIUS) + MAX_RADIUS + (i + 1) * PARA_FACTOR + k];
		}

	}

	return;
}

extern "C"{
void buffer_load(int flag, int k, INTERFACE_WIDTH imgvf_dest[GRID_COLS * (TILE_ROWS + 2) / WIDTH_FACTOR], INTERFACE_WIDTH *imgvf_src, INTERFACE_WIDTH I_dest[GRID_COLS * TILE_ROWS / WIDTH_FACTOR], INTERFACE_WIDTH *I_src)
{
#pragma HLS inline off
/*
    if (flag) {
		memcpy_wide_bus_read_float(imgvf_dest, (class ap_uint<LARGE_BUS> *)(imgvf_src + (k * TILE_ROWS * GRID_COLS - GRID_COLS) / (LARGE_BUS / 32)),0 * sizeof(float) , sizeof(float) *((unsigned long) ((TILE_ROWS + 2) * GRID_COLS)) );
		memcpy_wide_bus_read_float(I_dest, (class ap_uint<LARGE_BUS> *)(I_src + (k * TILE_ROWS * GRID_COLS) / (LARGE_BUS / 32)),0 * sizeof(float) , sizeof(float) *((unsigned long) (TILE_ROWS * GRID_COLS)) );
		}
    return;
*/

#pragma HLS INLINE OFF
    if (flag){
        int start_idx = (k * TILE_ROWS * GRID_COLS -GRID_COLS) / WIDTH_FACTOR;
        LOAD_TILE: for (int i(0); i < (TILE_ROWS + 2)*GRID_COLS/WIDTH_FACTOR; ++i){
        #pragma HLS PIPELINE II=1
            imgvf_dest[i] = imgvf_src[start_idx+i];
        }
    }
    if(flag){
        int start_idx = (k * TILE_ROWS * GRID_COLS) / WIDTH_FACTOR;
        LOAD_TILE2: for (int i(0); i < (TILE_ROWS)*GRID_COLS/WIDTH_FACTOR; ++i){
        #pragma HLS PIPELINE II=1
            I_dest[i] = I_src[start_idx+i];
        }
    }
    return;

}
}

extern "C"{
void buffer_compute(int flag, INTERFACE_WIDTH result_inner[GRID_COLS * TILE_ROWS/WIDTH_FACTOR], INTERFACE_WIDTH imgvf_inner[GRID_COLS * (TILE_ROWS + 2)/WIDTH_FACTOR], INTERFACE_WIDTH I_inner[GRID_COLS * TILE_ROWS/WIDTH_FACTOR], int k)
{
#pragma HLS inline off
    if (flag) lc_mgvf(result_inner, imgvf_inner, I_inner, k);
    return;
}
}

extern "C"{
void buffer_store(int flag, int k, INTERFACE_WIDTH *result_dest, INTERFACE_WIDTH result_src[GRID_COLS * TILE_ROWS/WIDTH_FACTOR])
{
#pragma HLS inline off
    if (flag){
        int start_idx = (k * TILE_ROWS * GRID_COLS) / WIDTH_FACTOR;
        STORE_TILE: for (int i(0); i < TILE_ROWS*GRID_COLS/WIDTH_FACTOR; ++i){
        #pragma HLS PIPELINE II=1
            result_dest[start_idx+i] = result_src[i];
        }
    }
}
}

extern "C" {
__kernel void workload(INTERFACE_WIDTH *result, INTERFACE_WIDTH *imgvf, INTERFACE_WIDTH *I)
{
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=result1
    #pragma HLS INTERFACE m_axi port=imgvf offset=slave bundle=imgvf1
    #pragma HLS INTERFACE m_axi port=I offset=slave bundle=I1
    
    #pragma HLS INTERFACE s_axilite port=result bundle=control
    #pragma HLS INTERFACE s_axilite port=imgvf bundle=control
    #pragma HLS INTERFACE s_axilite port=I bundle=control
    
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    INTERFACE_WIDTH result_inner_0 [TILE_ROWS * GRID_COLS/WIDTH_FACTOR];
#pragma HLS array_partition variable=result_inner_0 cyclic complete
    INTERFACE_WIDTH imgvf_inner_0   [(TILE_ROWS + 2) * GRID_COLS/WIDTH_FACTOR];
#pragma HLS array_partition variable=imgvf_inner_0   cyclic complete
    INTERFACE_WIDTH I_inner_0  [TILE_ROWS * GRID_COLS/WIDTH_FACTOR];
#pragma HLS array_partition variable=I_inner_0  cyclic complete

    INTERFACE_WIDTH result_inner_1 [TILE_ROWS * GRID_COLS/WIDTH_FACTOR];
#pragma HLS array_partition variable=result_inner_1 cyclic complete
    INTERFACE_WIDTH imgvf_inner_1   [(TILE_ROWS + 2) * GRID_COLS/WIDTH_FACTOR];
#pragma HLS array_partition variable=imgvf_inner_1   cyclic complete
    INTERFACE_WIDTH I_inner_1  [TILE_ROWS * GRID_COLS/WIDTH_FACTOR];
#pragma HLS array_partition variable=I_inner_1  cyclic complete

    INTERFACE_WIDTH result_inner_2 [TILE_ROWS * GRID_COLS/WIDTH_FACTOR];
#pragma HLS array_partition variable=result_inner_2 cyclic complete
    INTERFACE_WIDTH imgvf_inner_2   [(TILE_ROWS + 2) * GRID_COLS/WIDTH_FACTOR];
#pragma HLS array_partition variable=imgvf_inner_2   cyclic complete
    INTERFACE_WIDTH I_inner_2  [TILE_ROWS * GRID_COLS/WIDTH_FACTOR];
#pragma HLS array_partition variable=I_inner_2  cyclic complete

    int i , r , c;
    int k;

    for (i = 0; i < ITERATION/2; i++) {

        for (k = 0; k < GRID_ROWS / TILE_ROWS + 2; k++) {
            int load_flag = k >= 0 && k < GRID_ROWS / TILE_ROWS;
            int compute_flag = k >= 1 && k < GRID_ROWS / TILE_ROWS + 1;
            int store_flag = k >= 2 && k < GRID_ROWS / TILE_ROWS + 2;
            
            if (k % 3 == 0) {
                buffer_load(load_flag, k, imgvf_inner_0, imgvf, I_inner_0, I);
                buffer_compute(compute_flag, result_inner_2, imgvf_inner_2, I_inner_2, k - 1);
                buffer_store(store_flag, k - 2, result, result_inner_1);
            }

            else if (k % 3 == 1) {
                buffer_load(load_flag, k, imgvf_inner_1, imgvf, I_inner_1, I);
                buffer_compute(compute_flag, result_inner_0, imgvf_inner_0, I_inner_0, k - 1);
                buffer_store(store_flag, k - 2, result, result_inner_2);
            }
            
            else{
                buffer_load(load_flag, k, imgvf_inner_2, imgvf, I_inner_2, I);
                buffer_compute(compute_flag, result_inner_1, imgvf_inner_1, I_inner_1, k - 1);
                buffer_store(store_flag, k - 2, result, result_inner_0);
            }
        }

        for (k = 0; k < GRID_ROWS / TILE_ROWS + 2; k++) {
            int load_flag = k >= 0 && k < GRID_ROWS / TILE_ROWS;
            int compute_flag = k >= 1 && k < GRID_ROWS / TILE_ROWS + 1;
            int store_flag = k >= 2 && k < GRID_ROWS / TILE_ROWS + 2;
            
            if (k % 3 == 0) {
                buffer_load(load_flag, k, imgvf_inner_0, result, I_inner_0, I);
                buffer_compute(compute_flag, result_inner_2, imgvf_inner_2, I_inner_2, k - 1);
                buffer_store(store_flag, k - 2, imgvf, result_inner_1);
            }

            else if (k % 3 == 1) {
                buffer_load(load_flag, k, imgvf_inner_1, result, I_inner_1, I);
                buffer_compute(compute_flag, result_inner_0, imgvf_inner_0, I_inner_0, k - 1);
                buffer_store(store_flag, k - 2, imgvf, result_inner_2);
            }
            
            else{
                buffer_load(load_flag, k, imgvf_inner_2, result, I_inner_2, I);
                buffer_compute(compute_flag, result_inner_1, imgvf_inner_1, I_inner_1, k - 1);
                buffer_store(store_flag, k - 2, imgvf, result_inner_0);
            }
        }
    }

    return;
}
}
