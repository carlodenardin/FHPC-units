# OBLAS and MKL matrix multiplication analysis

This folder contains some results derived from the analysis conducted on the 2 math libraries OBLAS and MKL:
- [weak_scalability](): contains the results obtained by performing the gemm function for single and double precision on the epyc[005] node and the results of the gemm functions for single precision on the thin[007] node using 64 cores (--cpus-per-task) and 1 single task. These tests were performed multiple times starting from the Matrices of size 2000 up to 20000.
- [strong_scalability](): contains the results obtained by performing the gemm function for single and double precision on the epyc[005] node. The tests were performed multiple times with a matrix of size 14000 and and these number of cores (1, 2, 4, 8, 12, 16, 24, 32, 48, 64).

In the [script.sh]() a test is provided.