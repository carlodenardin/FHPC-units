/* 
Author: Carlo De Nardin
Date: January 30, 2023
Purpose: This file contains an hybrid parallel implementation of the Game of Life (MPI + OpenMP).
*/

#include <getopt.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <omp.h>

#include "rw.h"
#include "game.h"

#define DEAD 255
#define ALIVE 0

#define INIT 0
#define RUN 1

#define ORDERED 0
#define STATIC 1
#define BLACK_WHITE_STATIC 2

#define FILE_FORMAT ".pgm"

/*
* action: action to be performed (INIT or RUN)
* k: number of rows and columns of the image
* n: number of evolutions
* e: evolution type (ORDERED, STATIC, BLACK_WHITE_STATIC)
* s: after how many evolutions save the image
* file_name: name of the file to be read or written (REQUIRED!)
*/
int action = INIT;
int k = 1000;
int n = 100;
int e = STATIC;
int s = 0;
char *file_name = NULL;

/**
 * Given a the argc (number of arguments) and argv (array of arguments) of the main function,
 * this function parses the arguments and sets the global variables accordingly.
 *
 * @param argc number of arguments
 * @param argv array of arguments
 */
void get_arguments_utils(int argc, char **argv) {
    char *optstring = "irk:f:n:e:s:";

    int c;

    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch(c) { 
        case 'i': 
            action = INIT;
            break;
        case 'r': 
            action = RUN;
            break;
        case 'k':
            k = atoi(optarg);
            break;
        case 'e':
            e = atoi(optarg);
            break;
        case 'f': 
            file_name = (char*)malloc(sizeof(optarg) + 1);
            sprintf(file_name, "%s", optarg );
            break;  
        case 'n': 
            n = atoi(optarg);
            break;
        case 's':
            s = atoi(optarg);
            break;
        default: 
            printf("argument -%c not known\n", c ); break;
        }
    }
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Parse run-time arguments
    get_arguments_utils(argc, argv);

    // Check if file name is provided, if not, abort
    if (rank == 0 && file_name == NULL) {
        printf("\nFile name is not provided. Please provide a file name with -f <filename> option.\n\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Initialization (Process 0 generates the image randomly)
    if (action == INIT && rank == 0) {
        char * new_file_name = (char *) malloc(strlen(file_name) + strlen(FILE_FORMAT) + 1);
        strcpy(new_file_name, file_name);
        strcat(new_file_name, FILE_FORMAT);
        generate_image_utils(new_file_name, k, k);
        free(new_file_name);
    }

    // Run (static evolution and black-white static evolution)
    if (action == RUN && e != ORDERED) {
        int rows;
        int cols;

        // Process 0 reads the number of rows and columns of the image and
        // sends them to the other processes
        if (rank == 0) {
            rows = read_rows(file_name);
            cols = read_cols(file_name);
            for (int i = 1; i < size; i++) {
                MPI_Send(&rows, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&cols, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        } else {
            MPI_Recv(&rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&cols, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        // Grid size
        int full_size = rows * cols;

        // Size of the rows and columns for the grid that each process will work on
        int local_rows = rows / size;
        int local_cols = cols;
        
        // If the number of rows is not divisible by the number of processes,
        // the last process will work also on the remaining rows
        int offset = rows % size;
        if (rank == size - 1) {
            local_rows += offset;
        }

        // Size of the grid that each process will work on
        int local_size = local_rows * local_cols;

        // Size of the grid that each process will work on, including the ghost rows
        // and the ghost columns
        int local_rows_wg = local_rows + 2;
        int local_cols_wg = local_cols + 2;
        int local_size_wg = local_rows_wg * local_cols_wg;

        // Computing the ranks of the processes that have the local grid below
        // or above to the current process 
        int upper_rank = (rank == 0) ? size - 1 : rank - 1;
        int lower_rank = (rank == size - 1) ? 0 : rank + 1;

        // Allocating memory for the local grid and the local grid with ghost
        // and the local grid next state (after the evolution)
        int *local_grid_temp = (int *) malloc(local_size * sizeof(int));
        int *local_grid_wg = (int *) malloc(local_size_wg * sizeof(int));
        int *local_grid_ns = (int *) malloc(local_size_wg * sizeof(int));

        // Process 0 reads the image and sends the local grid to the other processes
        // and it keeps the local grid for itself
        if (rank == 0) {
            int *full_grid_temp = (int*) malloc(full_size * sizeof(int));

            read_image_utils(full_grid_temp, file_name, rows, cols);

            for (int i = 0; i < local_size; i++) {
                local_grid_temp[i] = full_grid_temp[i];
            }

            for (int i = 1; i < size; i++) {
                if (i < size - 1) {
                    MPI_Send(&full_grid_temp[i * local_size], local_size, MPI_INT, i, 0, MPI_COMM_WORLD);
                } else {
                    MPI_Send(&full_grid_temp[i * local_size], (local_rows + offset) * local_cols, MPI_INT, i, 0, MPI_COMM_WORLD);
                }
            }

            free(full_grid_temp);
        }

        // Receive the local grid from process 0
        if (rank != 0) {
            MPI_Recv(&local_grid_temp[0], local_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        // Copy local_grid_temp to local_grid_wg
        for(int i = 0; i < local_rows; i++) {
            for(int j = 0; j < local_cols; j++) {
                local_grid_wg[(i + 1) * local_cols_wg + (j + 1)] = local_grid_temp[i * local_cols + j];
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);

        // Static evolution and black-white static evolution
        if (e == STATIC) {
            for (int step = 1; step <= n; step++) {
                if (rank == 0) {
                    printf("Step %d/%d\n", step, n);
                }
                
                // Exchange ghost rows and compute ghost columns
                exchange_ghost_rows(local_grid_wg, local_rows_wg, local_cols_wg, upper_rank, lower_rank);
                compute_ghost_cols(local_grid_wg, local_rows_wg, local_cols_wg);
                
                // Perform the evolution
                static_evolution(local_grid_wg, local_grid_ns, local_rows_wg, local_cols_wg);

                // Copy the new state to the current state
                memcpy(local_grid_wg, local_grid_ns, local_size_wg * sizeof(int));

                // Save the image based on the save frequency (s)
                if ((s!=0 && step % s == 0) || step == n){
                    save_image(local_grid_wg, local_rows, local_cols, rows, cols, rank, size, offset, step); 
                }        
            }
        } else if (e == BLACK_WHITE_STATIC) {
            for (int step = 1; step <= n; step++) {
                if (rank == 0) {
                    printf("Step %d/%d\n", step, n);
                }

                // Exchange ghost rows and compute ghost columns
                exchange_ghost_rows(local_grid_wg, local_rows_wg, local_cols_wg, upper_rank, lower_rank);
                compute_ghost_cols(local_grid_wg, local_rows_wg, local_cols_wg);

                // Perform the evolution
                black_static_evolution(local_grid_wg, local_grid_ns, local_rows_wg, local_cols_wg);

                // Copy the new state to the current state
                memcpy(local_grid_wg, local_grid_ns, local_size_wg * sizeof(int));
                
                // Exchange ghost rows and compute ghost columns
                exchange_ghost_rows(local_grid_wg, local_rows_wg, local_cols_wg, upper_rank, lower_rank);
                compute_ghost_cols(local_grid_wg, local_rows_wg, local_cols_wg);

                // Perform the evolution
                white_static_evolution(local_grid_wg, local_grid_ns, local_rows_wg, local_cols_wg);

                // Copy the new state to the current state
                memcpy(local_grid_wg, local_grid_ns, local_size_wg * sizeof(int));

                // Save the image based on the save frequency (s)
                if ((s!=0 && step % s == 0) || step == n){
                    save_image(local_grid_wg, local_rows, local_cols, rows, cols, rank, size, offset, step);  
                }
            }   
        }
        
        // Free the allocated memory
        free(local_grid_temp);
        free(local_grid_wg);
        free(local_grid_ns);
    }

    // Ordered evolution (serial by definition)
    if (action == RUN && e == ORDERED && rank == 0) {
        // Read the number of rows and columns
        int rows = read_rows(file_name);
        int cols = read_cols(file_name);

        // Compute the number of rows and columns with ghost rows and columns
        int rows_wg = rows + 2;
        int cols_wg = cols + 2;
        int full_size = rows * cols;
        int full_size_wg = rows_wg * cols_wg;

        // Allocate memory for the grid and the grid with ghost rows and columns
        int *grid = (int*) malloc(full_size * sizeof(int));
        int *grid_wg = (int*) malloc(full_size_wg * sizeof(int));

        // Read the grid from the file
        read_image_utils(grid, file_name, rows, cols);

        // Copy local_grid_temp to local_grid_wg
        for(int i = 0; i < rows; i++) {
            for(int j = 0; j < cols; j++) {
                grid_wg[(i + 1) * cols_wg + (j + 1)] = grid[i * cols + j];
            }
        }

        for (int step = 1; step <= n; step++) {
            if (rank == 0) {
                printf("Step %d/%d\n", step, n);
            }

            // Compute ghost rows and columns
            compute_ghost_rows(grid_wg, rows, cols, rows_wg, cols_wg);
            compute_ghost_cols(grid_wg, rows_wg, cols_wg);

            // Perform the evolution
            for(int i = 1; i < rows_wg - 1; i++) {
                for(int j = 1; j < cols_wg - 1; j++) {
                    int count = count_alive_neighbors(grid_wg, i, j, cols_wg);
                    if (count < 2 || count > 3) {
                        grid_wg[i * cols_wg + j] = DEAD;
                        compute_ghost_rows(grid_wg, rows, cols, rows_wg, cols_wg);
                        compute_ghost_cols(grid_wg, rows_wg, cols_wg);
                    } else if (count == 3) {
                        grid_wg[i * cols_wg + j] = ALIVE;
                        compute_ghost_rows(grid_wg, rows, cols, rows_wg, cols_wg);
                        compute_ghost_cols(grid_wg, rows_wg, cols_wg);
                    } else {
                        grid_wg[i * cols_wg + j] = grid_wg[i * cols_wg + j];
                    }
                }
            }

            // Save the image based on the save frequency (s)
            if ((s!=0 && step % s == 0) || step == n){
                int id = 0;
                for(int i = 1; i <= rows; i++) {
                    for(int j = 1; j <= cols; j++) {
                        grid[id] = grid_wg[i * cols_wg + j];
                        id++;
                    }
                }
                save_image_utils(grid, rows, cols, step); 
            }        
        }
        
        // Free the allocated memory
        free(grid);
        free(grid_wg);
    }

    MPI_Finalize();
    return 0;
}