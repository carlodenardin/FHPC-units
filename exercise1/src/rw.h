#ifndef RW_H
#define RW_H

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
 * @param file_name The name of the PGM file.
 * @param rows The number of rows.
 * @param cols The number of columns.
 */
void read_image_utils(int *grid, char *file_name, int rows, int cols);

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
