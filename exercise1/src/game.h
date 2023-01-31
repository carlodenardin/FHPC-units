#ifndef GAME
#define GAME

/**
 * Given a the grid and the position of a cell, returns the number of alive neighbors.
 *
 * @param grid: grid of the game
 * @param i: row of the cell
 * @param j: column of the cell
 * @param cols: number of columns of the grid
 */ 
int count_alive_neighbors(int *grid, int i, int j, int cols);

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
void static_evolution(int *grid, int *grid_ns, int rows, int cols);

/**
 * Given the current grid, the next state grid and the dimension of the grid, 
 * compute the next state of the game with these steps considering only BLACK (ALIVE) cells:
 * - for each BLACK cell, count the number of alive neighbors
 * - if the BLACK cell has less than 2 or more than 3 alive neighbors, the cell dies
 * - otherwise, the cell keeps being alive
 *
 * @param grid: grid of the game
 * @param grid_ns: grid that will contain the next state
 */ 
void black_static_evolution(int *grid, int *grid_ns, int rows, int cols);

/**
 * Given the current grid, the next state grid and the dimension of the grid, 
 * compute the next state of the game with these steps considering only WHITE (DEAD) cells:
 * - for each WHITE cell, count the number of alive neighbors
 * - if the WHITE cell has 3 alive neighbors, the cell becomes alive
 * - otherwise, the cell keeps being dead
 *
 * @param grid: grid of the game
 * @param grid_ns: grid that will contain the next state
 */ 
void white_static_evolution(int *grid, int *grid_ns, int rows, int cols);

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
void exchange_ghost_rows(int *local_grid_wg, int local_rows_wg, int local_cols_wg, int upper_rank, int lower_rank);

/**
 * Copy the last column of the local grid to the first column of the ghost columns
 * and the first column of the local grid to the last column of the ghost columns.
 * 
 * @param local_grid_wg: local grid with ghost rows and columns
 * @param local_rows_wg: number of rows of the local grid with ghost rows
 * @param local_cols_wg: number of columns of the local grid with ghost rows
 */
void compute_ghost_cols(int *local_grid_wg, int local_rows_wg, int local_cols_wg);

/**
 * Copy the last row of the local grid to the first row of the ghost rows and
 * the first row of the local grid to the last row of the ghost rows.
 *
 * This functions is used for the ordered evolution since no message passing is used.
 * 
 * @param grid: local grid with ghost rows and columns
 * @param rows: number of rows of the grid
 * @param cols: number of columns of the frid
 * @param rows_wg: number of rows of the grid with ghost rows
 * @param cols_wg: number of columns of the grid with ghost rows
 */
void compute_ghost_rows(int *grid, int rows, int cols, int rows_wg, int cols_wg);

#endif