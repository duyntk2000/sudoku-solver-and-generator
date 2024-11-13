#ifndef GRID_H
#define GRID_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_GRID_SIZE 64
#define EMPTY_CELL '_'

#define NOT_CONSISTENT 2
#define SOLVED 1
#define CONSISTENT_NOT_SOLVED 0

static const char color_table[] =
 "123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "@" "abcdefghijklmnopqrstuvwxyz" "&*";

/* Sudoku grid */
typedef struct _grid_t grid_t;

/* Sudoku grid choice */
typedef struct choice_t choice_t;

/* Check if character is valid */
bool grid_check_char (const grid_t *grid, const char c);

/* Allocate memory for new grid */
grid_t *grid_alloc (size_t size);

/* Free the memory of grid_t */
void grid_free (grid_t *grid);

/* Write the grid in the file descriptor fd */
void grid_print (const grid_t *grid, FILE *fd);

/* Check if grid's size is among 1, 4, 9, 16, 25, 36, 49, 64 */
bool grid_check_size (const size_t size);

/* Deep copy of a grid */
grid_t *grid_copy (const grid_t *grid);

/* Get the content of a cell */
char *grid_get_cell (const grid_t *grid, const size_t row, const size_t column);

/* Get the size of the grid */
size_t grid_get_size (const grid_t *grid);

/* Set the content of a cell to a color */
void grid_set_cell (grid_t *grid, const size_t row, 
                    const size_t column, const char color);

/* Check if the grid is solved */
bool grid_is_solved (grid_t *grid);

/* Check if grid is consistent */
bool grid_is_consistent (grid_t *grid);

/* Apply heuristics and get consistency */
size_t grid_heuristics(grid_t *grid);

/* Free the memory of choice_t */
void grid_choice_free (choice_t *choice);

/* Check if a choice is empty */
bool grid_choice_is_empty (const choice_t *choice);

/* Apply a choice to the grid */
void grid_choice_apply (grid_t *grid, const choice_t *choice);

/* Set a choice to blank */
void grid_choice_blank (grid_t *grid, const choice_t *choice);

/* Discard a choice from the grid */
void grid_choice_discard (grid_t *grid, const choice_t *choice);

/* Describe the choice in file descriptor */
void grid_choice_print (const choice_t *choice, FILE *fd);

/* Generate a choice */
choice_t *grid_choice (grid_t *grid, bool random);

/* Randomly fill the first row.
   PRNG need to be initialized with srand() before calling this function */
void grid_initialize (grid_t *grid);

#endif /* GRID_H */
