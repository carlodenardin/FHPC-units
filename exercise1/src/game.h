#ifndef GAME
#define GAME

int count_alive_neighbors(int *grid, int i, int j, int cols);

void static_evolution(int *grid, int *grid_ns, int rows, int cols);

void ordered_evolution(int *grid, int rows, int cols);

void black_static_evolution(int *grid, int *grid_ns, int rows, int cols);

void white_static_evolution(int *grid, int *grid_ns, int rows, int cols);

void exchange_ghost_rows(int *local_grid_wg, int local_rows_wg, int local_cols_wg, int upper_rank, int lower_rank);

void compute_ghost_cols(int *local_grid_wg, int local_rows_wg, int local_cols_wg);

void compute_ghost_rows(int *grid, int rows, int cols, int rows_wg, int cols_wg);

void save_image(int * local_grid_wg, int local_rows, int local_cols, int rows, int cols, int rank, int size, int offset, int step);

#endif