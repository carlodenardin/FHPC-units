#include <stdlib.h>
#include <stdio.h> 
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <mpi.h>

#define ALIVE 0
#define DEAD 255

#define MAGIC "P5"
#define MAXVAL 255

#define FILE_FORMAT ".pgm"
#define FILE_NAME "snapshot"
#define FOLDER_NAME "snapshots"
#define PATTERNS_FOLDER "patterns"


char * add_extension(char *file_name) {
    char *file_name_with_ext = malloc(strlen(file_name) + strlen(FILE_FORMAT) + 1);
    strcpy(file_name_with_ext, file_name);
    strcat(file_name_with_ext, FILE_FORMAT);

    return file_name_with_ext;
}

/**
 * Create a folder if it doesn't exist (snapshots).
 */
void create_folder() {
    struct stat st = {0};

    if (stat(FOLDER_NAME, &st) == -1) {
        mkdir(FOLDER_NAME, 0700);
    }
}

/**
 * Write a PGM image to a file. In this case the file is added to the snapshots folder.
 *
 * @param image The image data.
 * @param file_name The name of the file.
 * @param rows The number of rows.
 * @param cols The number of columns.
 */
void write_pgm_image(void *image, char *file_name, int rows, int cols) {

    // Create folder if it doesn't exist
    create_folder();

    // Concatenate folder and image name
    char *full_path = malloc(strlen(FOLDER_NAME) + strlen(file_name) + 2);
    strcpy(full_path, FOLDER_NAME);
    strcat(full_path, "/");
    strcat(full_path, file_name);

    // File pointer
    FILE* image_file;
    image_file  = fopen(full_path, "w");

    // Write PGM header
    fprintf(image_file, "%2s %d %d\n%d\n", MAGIC, rows, cols, MAXVAL);
    
    // Write image data
    fwrite(image, 1, rows * cols, image_file);  

    fclose(image_file); 
    return;
}

void write_pgm_image_(void *image, char *file_name, int rows, int cols) {

    // File pointer
    FILE* image_file;
    image_file  = fopen(file_name, "w");

    // Write PGM header
    fprintf(image_file, "%2s %d %d\n%d\n", MAGIC, rows, cols, MAXVAL);
    
    // Write image data
    fwrite(image, 1, rows * cols, image_file);  

    fclose(image_file); 
    return;
}

/**
 * Generate a random grid of data composed of 0 (alive / black) and 255 (dead / white)
 *
 * @param rows The number of rows.
 * @param cols The number of columns.
 */
void * generate_gradient_random(int rows, int cols) {
    char *cImage;
    void *ptr;

    cImage = (char*) calloc(rows * cols, sizeof(char));

    int id = 0;
    srand(time(NULL));
    for (int i = 0; i < rows; i++ ) {
        for( int j = 0; j < cols; j++ ) {
            cImage[id++] = rand() % 2 == 0 ? (unsigned char) ALIVE : (unsigned char) DEAD;
        }
    }
    ptr = (void*)cImage;

    return ptr;
}

/**
 * Given a number it returns a string with 5 digits with a zero left padding
 *
 * @param number The number to pad with zeros.
 */
char* pad_with_zeros(int number) {
    static char str[6];
    sprintf(str, "%05d", number);
    return str;
}

/**
 * Given a grid of data it returns a pointer to a char array with the data
 *
 * @param grid The grid of data.
 * @param rows The number of rows.
 * @param cols The number of columns.
 */
void * generate_gradient_with_input(int *grid, int rows, int cols) {
    char *cImage;
    void *ptr;

    cImage = (char*) calloc(rows * cols, sizeof(char));

    int id = 0;

    srand(time(NULL));
    for (int i = 0; i < rows; i++ ) {
        for( int j = 0; j < cols; j++ ) {
            cImage[id++] = (char) grid[i * cols + j];
        }
    }
    ptr = (void*)cImage;

    return ptr;
}

/**
 * Given a PGM file, read the image data and return it as an array of unsigned
 * chars.
 *
 * @param file_name The name of the PGM file.
 */
unsigned char *read_pgm(char *file_name) {
    int row = 0;
    FILE *fp = fopen(file_name, "rb");
    char magic[3];
    int w, h, mv;

    if (!fp) {
        printf("Error: Unable to open file\n");
        exit(1);
    }

    // Read the header
    if (fscanf(fp, "%2s %d %d\n%d\n", magic, &w, &h, &mv) != 4) {
        printf("Error: Invalid header format\n");
        fclose(fp);
        exit(1);
    }

    // Allocate memory for the image data
    unsigned char *data = (unsigned char *)malloc(w * h);

    // Read the image data
    fread(data, 1, w * h, fp);
    
    fclose(fp);
    return data;
}

/**
 * Given a PGM file, read the number of columns specified in the header.
 *
 * @param file_name The name of the PGM file.
 */
int read_rows(char *file_name) {
    FILE *fp = fopen(add_extension(file_name), "rb");
    char magic[3];
    int rows, cols, max_value;

    if (!fp) {
        printf("Error: Unable to open file\n");
        return 1;
    }

    // Read the header
    if (fscanf(fp, "%2s %d %d\n%d\n", magic, &rows, &cols, &max_value) != 4) {
        printf("Error: Invalid header format\n");
        fclose(fp);
        return 1;
    }

    return rows;
}

/**
 * Given a PGM file, read the number of columns specified in the header.
 *
 * @param file_name The name of the PGM file.
 */
int read_cols(char *file_name) {
    FILE *fp = fopen(add_extension(file_name), "rb");
    char magic[3];
    int rows, cols, max_value;

    if (!fp) {
        printf("Error: Unable to open file\n");
        return 1;
    }

    // Read the header
    if (fscanf(fp, "%2s %d %d\n%d\n", magic, &rows, &cols, &max_value) != 4) {
        printf("Error: Invalid header format\n");
        fclose(fp);
        return 1;
    }

    return cols;
}

/**
 * Given a file_name and the dimension generate a PGM file with the specified
 * consisting in a PGM header and data (0 black, 255 white)
 *
 * @param file_name The name of the PGM file.
 * @param rows The number of rows.
 * @param cols The number of columns.
 */
void generate_image_utils(char *file_name, int width, int height) {
    
    printf("Initializing condition\n");
    
    printf("0. Writing initial condition to file %s\n", file_name);

    void *image = generate_gradient_random(width, height);

    printf("1. The image has been generated\n");

    write_pgm_image_(image, file_name, width, height);

    printf("2. The image has been written as %s\n", file_name);
}

/**
 * Given a file_name and the dimension read a PGM file with the specified
 * and save the data in the grid array.
 * @param file_name The name of the PGM file.
 * @param rows The number of rows.
 * @param cols The number of columns.
 */
void read_image_utils(int *grid, char *file_name, int rows, int cols) {
    unsigned char *image_data = read_pgm(add_extension(file_name));

    for (int i = 0; i < rows * cols; i++) {
        grid[i] = (int) image_data[i];
    }
}

/**
 * Given a file_name and the dimension save a PGM file with the specified
 * in the snapshop folder with this format: snapshot_0000<step>.pgm
 *
 * @param grid The grid to save.
 * @param rows The number of rows.
 * @param cols The number of columns.
 * @param step The step of the simulation.
 */
void save_image_utils(int * grid, int rows, int cols, int step) {
    void *image = generate_gradient_with_input(grid, rows, cols);

    char *padded_step = pad_with_zeros(step);

    char *full_file_name = (char *) malloc(strlen(FILE_NAME) + strlen(padded_step) + strlen(FILE_FORMAT) + 1);

    strcpy(full_file_name, FILE_NAME);
    strcat(full_file_name, padded_step);
    strcat(full_file_name, FILE_FORMAT);

    write_pgm_image(image, full_file_name, rows, cols);
}


void save_image(int * local_grid_wg, int local_rows, int local_cols, int rows, int cols, int rank, int size, int offset, int step) {
    int * local_grid = (int*) malloc(local_rows * local_cols * sizeof(int));

    int id = 0;
    for(int i = 1; i <= local_rows; i++) {
        for(int j = 1; j <= local_cols; j++) {
            local_grid[id] = local_grid_wg[i * (local_cols + 2) + j];
            id++;
        }
    }

    if (rank != 0) {
        MPI_Send(&local_grid[0], local_rows * local_cols, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else {
        int *full_grid_temp = (int*) malloc(rows * cols * sizeof(int));

        for (int i = 0; i < local_rows * local_cols; i++) {
            full_grid_temp[i] = local_grid[i];
        }

        for (int i = 1; i < size; i++) {
            if (i < size - 1) {
                MPI_Recv(&full_grid_temp[i * local_rows * local_cols], local_rows * local_cols, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                MPI_Recv(&full_grid_temp[i * local_rows * local_cols], (local_rows + offset) * local_cols, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        save_image_utils(full_grid_temp, rows, cols, step);

        free(full_grid_temp);
    }  
}