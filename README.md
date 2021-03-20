# rodinia-hls

## Notes 

This repository is forked from https://github.com/SFU-HiAccel/rodinia-hls.git, See [README.old.md](README.old.md) for more information.

Extended support for vadd algorithm, under directory [Benchmarks/vadd](Benchmarks/vadd). Only for studying the project, performance is not the primary concern.

### Update 2021.3.20

Added newly implemented MGVF algorithm in [Benchmarks/Benchmarks/leukocyte/my_mgvf](Benchmarks/Benchmarks/leukocyte/my_mgvf)

Changes to the original MGVF implementation:

+ Use 1024\*128 input instead of 1024\*1024 input due to resource limitation in Alveo_U50 FPGA
+ Modified [double_buffer](Benchmarks/Benchmarks/leukocyte/my_mgvf/lc_mgvf_4_doublebuffer_3buffer) code: use 3 stages instead of 4
+ Modified [coalescing](Benchmarks/Benchmarks/leukocyte/my_mgvf/lc_mgvf_5_coalescing_bitwise) code: use `range` API instead of original Falcon `memcpy` API for coalescing

## Verification Environment

+ **Host OS**
  + Ubuntu 20.04.1

+ **TARGET FPGA** 
  + Xilinx Alveo U50: xilinx_u50_gen3x16_xdma_201920_3

+ **Software Tools**
  + Vitis 2020.2
  + Xilinx Runtime(XRT) 2020.2
