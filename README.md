# FHPC-units
A report for this assignment is provided [here]()

An explanations of the tasks achieved for the Assigment of Foundations of High Performance Computing @UniTS is now provided:

## Exercise 1 (Game of Life hybrid MPI and OMP implementation)

For the Game of Life hybrid implementations these tasks have been done:
- Implementation of the hybrid MPI and OMP Game of Life with a `serial`, `ordered` and `black-white serial evolution`;
- `OMP strong scalability analyses` on a image of size `5000x5000` with an increase number of omp threads `(1, 2, 4, 8, 12, 16, 24, 32, 48, 64, 96, 128)` and only `1 MPI task`;
- `Multiple MPI strong scalability` analyses on a differen image size `(5000x5000, 10000x10000, 15000x15000, 20000x20000)` and an example of possible `overhead analysis`;
- `MPI weak scalability` analyses keeping the `workload equal` among MPI tasks and increasing the `MPI tasks from 1 to 6`. On orfeo is possible to request 3 nodes and each node as 2 cpus.

## Exercise 2 (Analyses of OBLAS and MKL library for matrix matrix multiplications)

For these analyses these tasks have been done:
- Analyses on AMD `epyc[005]` node for `single precision` with `64 cores` and different matrices dimensions `(2000, ..., 20000; step = 1000)`;
- Analyses on AMD `epyc[005]` node for `double precision` with `64 cores` and different matrices dimensions `(2000, ..., 20000; step = 1000)`;
- `Strong scalability analyses` on `epyc[005]` node for `single and double precision` with matrices of size `14000 x 14000` and different number of omp cores `(1, 2, 4, 8, 12, 16, 24, 32, 48, 64)`. In this case only the parameters `places = <sockets, cores>` and `bind = close` were tested
- Analyses on INTEL `thin[007]` node for `single precision` with `12 cores` and different matrices dimensions `(2000, ..., 20000; step = 1000)`;
