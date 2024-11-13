#include "grid.h"

#include "colors.h"

#include <math.h>

struct _grid_t
{
  size_t size;
  colors_t **cells;
};

struct choice_t
{
  size_t row;
  size_t column;
  colors_t color;
};

bool grid_check_char (const grid_t *grid, const char c)
{ 
  if (c == EMPTY_CELL)
    return true;

  for (size_t i = 0; i < grid->size; ++i)
    if (c == color_table[i])
      return true;
  return false;
}

grid_t *grid_alloc (size_t size)
{ 
  if (!grid_check_size(size))
    return NULL;

  colors_t **cells = malloc(size * sizeof(colors_t*));
  if (cells== NULL)
    return NULL;
  
  for (size_t i = 0; i < size; ++i) {
    cells[i] = malloc(size * sizeof(colors_t));
    if (!cells[i])
    {
      for (size_t j = 0; j < size; ++j)
        free(cells[j]);
      free(cells);
      return NULL;
    }

    for (size_t j = 0; j < size; ++j)
      cells[i][j] = colors_full(size);
  }

  grid_t *grid = malloc(sizeof(grid_t));
  if (!grid)
    return NULL;

  grid->size = size;
  grid->cells = cells;
  return grid;
}

void grid_free (grid_t *grid)
{ 
  if (grid == NULL)
    return;

  for (size_t i = 0; i < grid->size; ++i)
      free(grid->cells[i]);
  free(grid->cells);
  free(grid);
}

void grid_print (const grid_t *grid, FILE *fd)
{
  if (grid == NULL)
    return;

  for (size_t i = 0; i < grid->size; ++i) {
    for (size_t j = 0; j < grid->size; ++j) {
      char *content = grid_get_cell(grid, i, j);
      if (content == NULL)
        return;

      fprintf(fd, "%s ", content);
      free(content);
    }
    fputs("\n", fd);
  }
  fputs("\n", fd);
}

bool grid_check_size (const size_t size)
{
  return size == 1 || size == 4 || size == 9 || size == 16 || 
         size == 25 || size == 36 || size == 49 || size == 64;
}

grid_t *grid_copy (const grid_t *grid)
{ 
  if (grid == NULL)
    return NULL;

  grid_t *grid_new = grid_alloc(grid->size);
  if (grid_new == NULL)
    return NULL;

  for (size_t i = 0; i < grid->size; ++i) 
    for (size_t j = 0; j < grid->size; ++j)
      grid_new->cells[i][j] = grid->cells[i][j];
  return grid_new;
}

char *grid_get_cell (const grid_t *grid, const size_t row, const size_t column)
{ 
  if (grid == NULL)
    return NULL;
  if (row >= grid->size || column >= grid->size)
    return NULL;
  colors_t cell = grid->cells[row][column];
  char *content = calloc(grid->size + 1, sizeof(char));
  if (content == NULL)
    return NULL;
  
  content[0] = EMPTY_CELL;
  if (cell == colors_full(grid->size) && grid->size != 1) {
    return content;
  }
  size_t j = 0;
  for (size_t i = 0; i < grid->size; ++i)
    if (colors_is_in(cell, i)) {
      content[j] = color_table[i];
      ++j;
    }
  return content;
}

size_t grid_get_size (const grid_t *grid)
{ 
  if (!grid)
    return 0;
  return grid->size;
}

void grid_set_cell (grid_t *grid, const size_t row, 
                    const size_t column, const char color)
{
  if (!grid)
    return;
  if (row >= grid->size || column >= grid->size)
    return;

  if (color == EMPTY_CELL) {
    grid->cells[row][column] = colors_full(grid->size);
    return;
  }
  for (size_t i = 0; i < grid->size; ++i)
    if (color == color_table[i]) {
      grid->cells[row][column] = colors_set(i);
      return;
    }
}

bool grid_is_solved (grid_t *grid)
{
  if (!grid)
    return false;

  for (size_t i = 0; i < grid->size; ++i)
    for (size_t j = 0; j < grid->size; ++j)
      if (!colors_is_singleton(grid->cells[i][j]))
        return false;
  return true;
}

bool grid_is_consistent (grid_t *grid)
{
  if (grid->size == 1)
    return true;
  colors_t *subgrid = malloc(grid->size * sizeof(colors_t));
  if (!subgrid)
    return NULL;

  for (size_t index = 0; index < grid->size; ++index) {
    for (size_t i = 0; i < grid->size; ++i)
      subgrid[i] = grid->cells[index][i];
    if (!subgrid_consistency(subgrid, grid->size)) {
      free(subgrid);
      return false;
    }
  
    for (size_t i = 0; i < grid->size; ++i)
      subgrid[i] = grid->cells[i][index];
    if (!subgrid_consistency(subgrid, grid->size)) {
      free(subgrid);
      return false;
    }
  
    size_t block_size = sqrt(grid->size);
    size_t start_row = index / block_size * block_size;
    size_t start_column = index % block_size * block_size;
    size_t c = 0;
    for (size_t i = start_row; i < start_row + block_size; ++i) {
      for (size_t j = start_column; j < start_column + block_size; ++j) {
        subgrid[c++] = grid->cells[i][j];
      }
    }
    if (!subgrid_consistency(subgrid, grid->size)) {
      free(subgrid);
      return false;
    }
  }
  free(subgrid);
  return true;
}

size_t grid_heuristics(grid_t *grid)
{ 
  if (!grid)
    return NOT_CONSISTENT;
  if (grid->size == 1)
    return SOLVED;
  if (!grid_is_consistent(grid))
    return NOT_CONSISTENT;
  
  size_t level = 0;
  while (level < 2) {
    bool fix = false;
    colors_t *subgrid[grid->size];
    for (size_t index = 0; index < grid->size; ++index) {
      for (size_t i = 0; i < grid->size; ++i)
        subgrid[i] = &grid->cells[index][i];
      fix |= subgrid_heuristics(subgrid, grid->size, level);
    
      for (size_t i = 0; i < grid->size; ++i)
        subgrid[i] = &grid->cells[i][index];
      fix |= subgrid_heuristics(subgrid, grid->size, level);

      size_t block_size = sqrt(grid->size);
      size_t start_row = index / block_size * block_size;
      size_t start_column = index % block_size * block_size;
      size_t c = 0;
      for (size_t i = start_row; i < start_row + block_size; ++i)
        for (size_t j = start_column; j < start_column + block_size; ++j )
          subgrid[c++] = &grid->cells[i][j];
      fix |= subgrid_heuristics(subgrid, grid->size, level);
    }
    if (fix) {
      if (level == 1)
        --level;
    }
    else
      ++level;
  }
  if (grid_is_solved(grid))
    return SOLVED;
  if (grid_is_consistent(grid))
    return CONSISTENT_NOT_SOLVED;
  return NOT_CONSISTENT;
}

void grid_choice_free (choice_t *choice)
{
  if (!choice)
    return;

  free(choice);
}

bool grid_choice_is_empty (const choice_t *choice)
{
  if (!choice)
    return true;

  return choice->color == 0;
}

void grid_choice_apply (grid_t *grid, const choice_t *choice)
{
  if (!grid || !choice)
    return;

  grid->cells[choice->row][choice->column] = choice->color;
}

void grid_choice_blank (grid_t *grid, const choice_t *choice)
{
  if (!grid || !choice)
    return;

  grid->cells[choice->row][choice->column] = colors_full(grid->size);
}

void grid_choice_discard (grid_t *grid, const choice_t *choice)
{
  if (!grid || !choice)
    return;

  colors_t cell = grid->cells[choice->row][choice->column];
  cell = colors_subtract(cell, choice->color);
  grid->cells[choice->row][choice->column] = cell;
}

void grid_choice_print (const choice_t *choice, FILE *fd)
{
  fprintf(fd, "Choice : row %ld, column %ld, colors %ld \n",
          choice->row, choice->column, choice->color);
}

choice_t *grid_choice (grid_t *grid, bool random)
{
  if (!grid)
    return NULL;
  choice_t *choice = malloc(sizeof(choice_t));
  if (!choice)
    return NULL;
    
  choice->color = 0;
  choice->column = 0;
  choice->row = 0;
  for (size_t i = 0; i < grid->size; ++i)
    for (size_t j = 0; j < grid->size; ++j) {
      if (colors_is_singleton(grid->cells[i][j]))
        continue;
      
      choice->color = grid->cells[i][j];
      choice->row = i;
      choice->column = j;
      i = grid->size;
      j = grid->size;
    }
  if (random)
    choice->color = colors_random(choice->color);
  else
    choice->color = colors_leftmost(choice->color);
  return choice;
}

void grid_initialize (grid_t *grid)
{ 
  /* PRNG need to be initialized with srand() before calling this function */
  for (size_t i = 0; i < grid->size; ++i)
    grid->cells[0][i] = colors_set(i);

  for (size_t i = grid->size - 1; i > 0; --i) {
    size_t j = rand() % i;
    colors_t temp = grid->cells[0][i];
    grid->cells[0][i] = grid->cells[0][j];
    grid->cells[0][j] = temp;
  }
}

