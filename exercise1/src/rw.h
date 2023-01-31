#ifndef RW_H
#define RW_H

/**
* Given a file_name adds the .pgm extension
*/
char * add_pgm_extension(char *file_name);

/*
* Create a folder that is specified in the constant FOLDER_NAME
*/
void create_folder();


/**
 * Given a PGM file, read the image data and return it as an array of unsigned
 * chars.
 *
 * @param file_name The name of the PGM file.
 * @return The image data as an array of unsigned chars.
 */
unsigned char *read_pgm(char *file_name);

/**
 * Given a PGM file, read the number of columns specified in the header.
 *
 * @param file_name The name of the PGM file.
 */
int read_cols(char *file_name);

/**
 * Given a PGM file, read the number of rows specified in the header.
 *
 * @param file_name The name of the PGM file.
 */
int read_rows(char *file_name);

/**
 * Given a file_name and the dimension generate a PGM file with the specified
 * consisting in a PGM header and data (0 black, 255 white)
 *
 * @param file_name The name of the PGM file.
 * @param rows The number of rows.
 * @param cols The number of columns.
 */
void generate_image_utils(char *file_name, int rows, int cols);

/**
 * Given a file_name and the dimension read a PGM file with the specified
 * and save the data in the grid array.
 *
 * @param file_name The name of the PGM file.
 * @param rows The number of rows.
 * @param cols The number of columns.
 */
void read_image_utils(int *grid, char *file_name, int rows, int cols);


void save_image(int * local_grid_wg, int local_rows, int local_cols, int rows, int cols, int rank, int size, int offset, int step);

/**
 * Given a file_name and the dimension save a PGM file with the specified
 * in the snapshop folder with this format: snapshot_0000<step>.pgm
 *
 * @param grid The grid to save.
 * @param rows The number of rows.
 * @param cols The number of columns.
 * @param step The step of the simulation.
 */
void save_image_utils(int * grid, int rows, int cols, int step);

#endif
