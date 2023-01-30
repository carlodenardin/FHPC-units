#include <getopt.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <omp.h>
#include <time.h>
#include <unistd.h>

#include "rw.h"
#include "game.h"

#define DEAD 255 // 0
#define ALIVE 0  // 1

#define INIT 0
#define RUN 1

#define ORDERED 0
#define STATIC 1
#define BLACK_WHITE_STATIC 2

#define FILE_FORMAT ".pgm"

// Default values
int action = INIT;
int e = STATIC;
int k = 1000;
int n = 100;
int s = 0;
char *file_name = NULL;

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

    get_arguments_utils(argc, argv);

    // CHECK THAT FILE NAME IS PROVIDED //
    if (rank == 0 && file_name == NULL) {
        printf("\nFile name is not provided. Please provide a file name with -f <filename> option.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // INITIALIZATION //
    if (action == INIT && rank == 0) {
        char * new_file_name = (char *) malloc(strlen(file_name) + strlen(FILE_FORMAT) + 1);
        strcpy(new_file_name, file_name);
        strcat(new_file_name, FILE_FORMAT);
        generate_image_utils(new_file_name, k, k);
        free(new_file_name);
    }

    // RUN //
    if (action == RUN && e != ORDERED) {

        int rows;
        int cols;

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

        int full_size = rows * cols;

        int local_rows = rows / size;
        int local_cols = cols;

        int offset = rows % size;

        if (rank == size - 1) {
            local_rows += offset;
        }

        int local_size = local_rows * local_cols;

        int local_rows_wg = local_rows + 2;
        int local_cols_wg = local_cols + 2;
        int local_size_wg = local_rows_wg * local_cols_wg;

        int upper_rank = (rank == 0) ? size - 1 : rank - 1;
        int lower_rank = (rank == size - 1) ? 0 : rank + 1;

        int *local_grid_temp = (int *) malloc(local_size * sizeof(int));
        int *local_grid_wg = (int *) malloc(local_size_wg * sizeof(int));
        int *local_grid_ns = (int *) malloc(local_size_wg * sizeof(int));

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

        // Receive the image from rank 0
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

        if (e == STATIC) {
            for (int step = 1; step <= n; step++) {
                if (rank == 0) {
                    printf("Step %d/%d\n", step, n);
                }
                
                exchange_ghost_rows(local_grid_wg, local_rows_wg, local_cols_wg, upper_rank, lower_rank);
                compute_ghost_cols(local_grid_wg, local_rows_wg, local_cols_wg);

                static_evolution(local_grid_wg, local_grid_ns, local_rows_wg, local_cols_wg);

                memcpy(local_grid_wg, local_grid_ns, local_size_wg * sizeof(int));

                if ((s!=0 && step % s == 0) || step == n){
                    save_image(local_grid_wg, local_rows, local_cols, rows, cols, rank, size, offset, step); 
                }        
            }
        } else if (e == BLACK_WHITE_STATIC) {
            for (int step = 1; step <= n; step++) {
                exchange_ghost_rows(local_grid_wg, local_rows_wg, local_cols_wg, upper_rank, lower_rank);
                compute_ghost_cols(local_grid_wg, local_rows_wg, local_cols_wg);
                black_static_evolution(local_grid_wg, local_grid_ns, local_rows_wg, local_cols_wg);
                memcpy(local_grid_wg, local_grid_ns, local_size_wg * sizeof(int));
                
                exchange_ghost_rows(local_grid_wg, local_rows_wg, local_cols_wg, upper_rank, lower_rank);
                compute_ghost_cols(local_grid_wg, local_rows_wg, local_cols_wg);
                white_static_evolution(local_grid_wg, local_grid_ns, local_rows_wg, local_cols_wg);
                memcpy(local_grid_wg, local_grid_ns, local_size_wg * sizeof(int));

                if ((s!=0 && step % s == 0) || step == n){
                    save_image(local_grid_wg, local_rows, local_cols, rows, cols, rank, size, offset, step);  
                }
            }   
        }
    
        MPI_Barrier(MPI_COMM_WORLD);

        free(local_grid_temp);
        free(local_grid_wg);
        free(local_grid_ns);
    }

    if (action == RUN && e == ORDERED && rank == 0) {
        printf("Ordered evolution\n");
        int rows = read_rows(file_name);
        int cols = read_cols(file_name);
        int rows_wg = rows + 2;
        int cols_wg = cols + 2;
        int full_size = rows * cols;
        int full_size_wg = rows_wg * cols_wg;

        int *grid = (int*) malloc(full_size * sizeof(int));
        int *grid_wg = (int*) malloc(full_size_wg * sizeof(int));

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

            compute_ghost_rows(grid_wg, rows, cols, rows_wg, cols_wg);
            compute_ghost_cols(grid_wg, rows_wg, cols_wg);

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
        
    }

    MPI_Finalize();
    return 0;
}