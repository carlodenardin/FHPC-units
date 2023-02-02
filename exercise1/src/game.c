#include <omp.h>
#include <stdlib.h>
#include <stdio.h> 
#include <mpi.h>
#include <string.h>

#define ALIVE 0
#define DEAD 255

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

void black_static_evolution(int *grid, int *grid_ns, int rows, int cols) {
    #pragma omp parallel for schedule(static)
    for(int i = 1; i < rows - 1; i++) {
        for(int j = 1; j < cols - 1; j++) {
            if (grid[i * cols + j] == ALIVE) {
                int count = count_alive_neighbors(grid, i, j, cols);
                if (count < 2 || count > 3) {
                    grid_ns[i * cols + j] = DEAD;
                } else {
                    grid_ns[i * cols + j] = grid[i * cols + j];
                }
            } else {
                grid_ns[i * cols + j] = grid[i * cols + j];
            }
        }
    }
}

void white_static_evolution(int *grid, int *grid_ns, int rows, int cols) {
    #pragma omp parallel for schedule(static)
    for(int i = 1; i < rows - 1; i++) {
        for(int j = 1; j < cols - 1; j++) {
            if (grid[i * cols + j] == DEAD) {
                int count = count_alive_neighbors(grid, i, j, cols);
                if (count == 3) {
                    grid_ns[i * cols + j] = ALIVE;
                } else {
                    grid_ns[i * cols + j] = grid[i * cols + j];
                }
            } else {
                grid_ns[i * cols + j] = grid[i * cols + j];
            }
        }
    }
}

void exchange_ghost_rows(int *local_grid_wg, int local_rows_wg, int local_cols_wg, int upper_rank, int lower_rank) {
    MPI_Request request;
    MPI_Isend(&local_grid_wg[local_cols_wg], local_cols_wg, MPI_INT, upper_rank, 0, MPI_COMM_WORLD, &request);
    MPI_Irecv(&local_grid_wg[(local_rows_wg - 1) * local_cols_wg], local_cols_wg, MPI_INT, lower_rank, 0, MPI_COMM_WORLD, &request);
    
    MPI_Isend(&local_grid_wg[(local_rows_wg - 2) * local_cols_wg], local_cols_wg, MPI_INT, lower_rank, 1, MPI_COMM_WORLD, &request);
    MPI_Irecv(&local_grid_wg[0], local_cols_wg, MPI_INT, upper_rank, 1, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE);
}

void compute_ghost_cols(int *local_grid_wg, int local_rows_wg, int local_cols_wg) {
    for(int i = 0; i < local_rows_wg; i++) {
        local_grid_wg[i * local_cols_wg] = local_grid_wg[(i + 1) * local_cols_wg - 2];
        local_grid_wg[(i + 1) * local_cols_wg - 1] = local_grid_wg[i * local_cols_wg + 1];
    }
}

void compute_ghost_rows(int *grid, int rows, int cols, int rows_wg, int cols_wg) {
    for(int i = 1; i <= cols; i++) {
        grid[i] = grid[cols_wg * rows + i];
        grid[cols_wg * (rows_wg - 1) + i] = grid[cols_wg + i];
    }
}

