# Game of Life - Hybrid MPI and OMP implementation

## How to compile
To compile the executable
```
srun -n 1 make all
```

To cancel the executable:
```
srun -n 1 make clean
```


## How to inizialize the game

| Arguments | Description | Default |
| ------- | --- | --- |
| -i | initialize a random pmg image | required|
| -k (number) | dimension of the random square image | 1000 |
| -f (name)| name of the random pmg image  |  required|

```
mpirun -np 1 gol.x -i -k 100 -f pattern_random
```

This code will generate the `pattern_random.pmg` image which as a dimension `100x100` in the main folder. The number of processes is indifferent since the generation will be executed in a serial way.

## How to run the game

| Arguments | Description | Default |
| ------- | --- | --- |
| -r | run the game | required |
| -f (name) | name of the random pmg image  |  required |
| -n (number) | number of evolution to perform  | 100 | 
| -e (0, 1, 2) | types of evolution (0: ordered, 1: static, 2: BW static) | 1: static |
| -s (number) | how many evolutions save the image | 0: only at the end |

### Run 1:
```
mpirun -np 4 gol.x -r -f pattern_random -n 100 -e 1 -s 0
```

This code will perform the game evolutions (`static evolution`), for the input `pattern_random.pgm`, for `100 steps` and it will save only the last evolution state.

### Run 2:
```
mpirun -np 4 gol.x -r -f pattern_random -n 1000 -e 1 -s 1
```

This code will perform the game evolutions (`static evolution`), for the input `pattern_random.pgm`, for `1000 steps` and it will save an image of the state for each step.


## Examples of common patterns

| Input | Result |
| ------- | --- |
| pattern_blinker | [Blinker GIF](https://imgur.com/ndWD6tX) |
| pattern_pulsare | [Pulsar GIF](https://imgur.com/9OQdXyP) |
| pattern_ship | name of the random pmg image  |

## Code details
The source code is divided among 3 different files:
- [gol.c](https://github.com/carlodenardin/FHPC-units/blob/main/exercise1/src/gol.c): main file that manages everything
- [game.c](https://github.com/carlodenardin/FHPC-units/blob/main/exercise1/src/game.c): functions that are used to provide the evolutions of the game. The header file [game.h](https://github.com/carlodenardin/FHPC-units/blob/main/exercise1/src/game.h) contains the documentations of the functions
- [rw.c](https://github.com/carlodenardin/FHPC-units/blob/main/exercise1/src/rw.c): functions that are used to read and write pgm files. The header file [rw.h](https://github.com/carlodenardin/FHPC-units/blob/main/exercise1/src/rw.h) contains the documentations of the functions

## Functions

### Count alive neighbors
```c
/**
 * Given a the grid and the position of a cell, returns the number of alive neighbors.
 *
 * @param grid: grid of the game
 * @param i: row of the cell
 * @param j: column of the cell
 * @param cols: number of columns of the grid
 */ 
int count_alive_neighbors(int *grid, int i, int j, int cols) {
    int alive_neighbors = 0;
    for(int k = -1; k <= 1; k++) {
        for(int l = -1; l <= 1; l++) {
            if (k == 0 && l == 0) continue;
            if (grid[(i + k) * cols + (j + l)] == ALIVE) alive_neighbors++;
        }
            
    }
    return alive_neighbors;
}
```

### Static Evolution
```c
/**
 * Given the current grid, the next state grid and the dimension of the grid, 
 * compute the next state of the game with these steps:
 * - for each cell, count the number of alive neighbors
 * - if the cell has less than 2 or more than 3 alive neighbors, the cell dies
 * - if the cell has 3 alive neighbors, the cell survives or becomes alive
 * - if the cell has 2 alive neighbors, the cell state remains the same
 *
 * @param grid: grid of the game
 * @param grid_ns: grid that will contain the next state
 */ 
void static_evolution(int *grid, int *grid_ns, int rows, int cols) {
    #pragma omp parallel for schedule(static)
    for(int i = 1; i < rows - 1; i++) {
        for(int j = 1; j < cols - 1; j++) {
            int count = count_alive_neighbors(grid, i, j, cols);
            if (count < 2 || count > 3) {
                grid_ns[i * cols + j] = DEAD;
            } else if (count == 3) {
                grid_ns[i * cols + j] = ALIVE;
            } else {
                grid_ns[i * cols + j] = grid[i * cols + j];
            }
        }
    }
}
```

### Send Receive Ghost Rows
```c
/**
 * Exchange the ghost rows of the local grid with the upper and lower ranks using
 * the MPI_Send and MPI_Recv functions.
 *
 * @param local_grid_wg: local grid with ghost rows and columns
 * @param local_rows_wg: number of rows of the local grid with ghost rows
 * @param local_cols_wg: number of columns of the local grid with ghost rows
 * @param upper_rank: rank of the upper process
 * @param lower_rank: rank of the lower process
 */
void exchange_ghost_rows(int *local_grid_wg, int local_rows_wg, int local_cols_wg, int upper_rank, int lower_rank) {
    MPI_Send(&local_grid_wg[local_cols_wg], local_cols_wg, MPI_INT, upper_rank, 0, MPI_COMM_WORLD);
    MPI_Recv(&local_grid_wg[(local_rows_wg - 1) * local_cols_wg], local_cols_wg, MPI_INT, lower_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    MPI_Send(&local_grid_wg[(local_rows_wg - 2) * local_cols_wg], local_cols_wg, MPI_INT, lower_rank, 1, MPI_COMM_WORLD);
    MPI_Recv(&local_grid_wg[0], local_cols_wg, MPI_INT, upper_rank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
```

### Compute Ghost Columns
```c
/**
 * Copy the last column of the local grid to the first column of the ghost columns
 * and the first column of the local grid to the last column of the ghost columns.
 * 
 * @param local_grid_wg: local grid with ghost rows and columns
 * @param local_rows_wg: number of rows of the local grid with ghost rows
 * @param local_cols_wg: number of columns of the local grid with ghost rows
 */
void compute_ghost_cols(int *local_grid_wg, int local_rows_wg, int local_cols_wg) {
    for(int i = 0; i < local_rows_wg; i++) {
        local_grid_wg[i * local_cols_wg] = local_grid_wg[(i + 1) * local_cols_wg - 2];
        local_grid_wg[(i + 1) * local_cols_wg - 1] = local_grid_wg[i * local_cols_wg + 1];
    }
}
```