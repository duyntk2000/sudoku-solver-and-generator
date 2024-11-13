#include "sudoku.h"

#include "grid.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <err.h>
#include <getopt.h>
#include <string.h>
#include <time.h>

typedef enum { mode_first, mode_all, mode_unique } mode_t;
static bool verbose = false;
static size_t solutions = 0;

static grid_t *file_parser (char *filename)
{ 
  FILE *stream = fopen(filename, "r");
  if (stream == NULL)
    errx(EXIT_FAILURE, "Can't open input file!");

  char first_row[MAX_GRID_SIZE];
  int ch;
  size_t n = 0;
  size_t line = 1;
  bool comment = false;
  bool exit = false;
  while(!exit) {
    ch = fgetc(stream);
    
    if (ch == EOF) {
      line++;
      break;
    }
    
    if (ch == '\n') {
      line++;
      if (n > 0)
        break;
      comment = false;
      continue;
    }
    
    if (!comment) {
      switch (ch) {
        case ' ':
        case '\t':
          break;

        case '\n':
          line++;
          if (n > 0)
            exit = true;
          break;

        case '#':
          comment = true;
          break;

        default:
          if (n == MAX_GRID_SIZE) {
            warnx("error: line %zu is malformed! (exceed max size)", line);
            fclose(stream);
            return NULL;
          }
          first_row[n++] = ch;
          break;
      }
    }
  }
  
  if (n == 0) {
    warnx("error: Grid is empty");
    fclose(stream);
    return NULL;
  }
  
  grid_t *grid = grid_alloc(n);
  if (!grid) {
    warnx("error: Can't allocate new grid!");
    fclose(stream);
    return NULL;
  }

  for (size_t i = 0; i < n; i++)
    if (grid_check_char(grid, first_row[i]))
      grid_set_cell(grid, 0, i, first_row[i]);
    else {
      warnx("error: wrong character '%c' at line %zu!", first_row[i], line - 1);
      grid_free(grid);
      fclose(stream);
      return NULL;
    }
  size_t row = 1;
  
  n = 0;
  comment = false;
  while(true) {
    ch = fgetc(stream);
    
    if (ch == '\n' || ch == EOF) {
      comment = false;
      if (n == grid_get_size(grid)) {
        row++;
        n = 0;
      }
      else if (n > 0) {
        warnx("error: line %zu is malformed! (wrong number of columns)", line);
        grid_free(grid);
        fclose(stream);
        return NULL;
      }
      if (ch == EOF)
        break;
      line++;
      continue;
    }
    if (!comment) {
      switch (ch) {
        case ' ':
        case '\t':
          break;

        case '#':
          comment = true;
          break;

        default:
          if (n >= grid_get_size(grid)) {
            warnx("error: line %zu is malformed! (wrong number of columns)", line);
            grid_free(grid);
            fclose(stream);
            return NULL;
          }
          if (row >= grid_get_size(grid)) {
            warnx("error: grid has extra lines starting from line %zu!", line);
            grid_free(grid);
            fclose(stream);
            return NULL;
          }
          if (grid_check_char(grid, ch)) {
            grid_set_cell(grid, row, n, ch);
            n++;
          }
          else {
            warnx("error: wrong character '%c' at line %zu!", ch, line);
            grid_free(grid);
            fclose(stream);
            return NULL;
          }
          break;
      }
    }
  }
  
  if (row < grid_get_size(grid)) {
    warnx("error: grid has %zu missing line(s)", grid_get_size(grid) - row);
    grid_free(grid);
    fclose(stream);
    return NULL;
  }
  fclose(stream);
  
  return grid;
}

static grid_t *grid_solver (grid_t *grid, const mode_t mode, FILE* stream, bool random)
{
  if (!grid)
    return NULL;
  
  size_t c = grid_heuristics(grid);
  if (c == NOT_CONSISTENT) {
    grid_free(grid);
    return NULL;
  }
  if (c == SOLVED) {
    solutions++;
    if (stream)
      grid_print(grid, stream);
    return grid;
  }
  
  choice_t *choice = grid_choice(grid, random);
  grid_t *last = NULL;
  while (!grid_choice_is_empty(choice)) {
    grid_t *grid_new = grid_copy(grid);
    grid_choice_apply(grid_new, choice);
    grid_t *result = grid_solver(grid_new, mode, stream, random);
    if (result) {
      if (mode == mode_first) {
        grid_choice_free(choice);
        grid_free(grid);
        return result;
      }
      else {
        if (last)
          grid_free(last);
        last = result;
      }
    }
    grid_choice_discard(grid, choice);
    grid_choice_free(choice);
    if (!grid_is_consistent(grid)) {
      grid_free(grid);
      if (mode == mode_first)
        return NULL;
      return last;
    }
    choice = grid_choice(grid, random);
  }
  grid_choice_free(choice);
  grid_free(grid);
  if (mode == mode_all)
    return last;
  return NULL;
}

static grid_t *grid_generator (size_t size, const mode_t mode)
{
  grid_t *grid = grid_alloc(size);
  if (!grid)
    return NULL;
    
  /* PRNG need to be initialized with srand() before calling this function */
  size_t total = size * size;
  grid_initialize(grid);
  grid = grid_solver(grid, mode_first, NULL, true);
  size_t *pos = malloc(total*sizeof(size_t));
  for (size_t i = 0; i < total; i ++)
    pos[i] = i;
  for (size_t i = total - 1; i > 0; --i) {
    size_t j = rand() % i;
    size_t temp = pos[i];
    pos[i] = pos[j];
    pos[j] = temp;
  }
  size_t count = total * EMPTY_RATE;
  if (mode == mode_first)
    for (size_t i = 0; i < count; ++i) 
      grid_set_cell(grid, pos[i] / size, pos[i] % size, EMPTY_CELL);
  else {
    for (size_t i = 0; i < total; ++i) {
      solutions = 0;
      grid_t *copy = grid_copy(grid);
      grid_set_cell(copy, pos[i] / size, pos[i] % size, EMPTY_CELL);
      copy = grid_solver(copy, mode_all, NULL, false);
      if (solutions == 1) {
        grid_set_cell(grid, pos[i] / size, pos[i] % size, EMPTY_CELL);
        --count;
      }
      grid_free(copy);
      if (count == 0)
        break;
    }
  }
  free(pos);
  return grid;
}

int main(int argc, char* argv[]) 
{
  bool solver = true;
  bool has_output_file = false;
  int optc;
  int args = 1;
  const struct option longopts[] = {
    { "all", no_argument, NULL, 'a' },
    { "unique", no_argument, NULL, 'u' },
    { "help" , no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
    { "verbose", no_argument, NULL, 'v' },
    { "output", required_argument, NULL, 'o' },
    { "generate", optional_argument, NULL, 'g' },
    { NULL, 0, NULL, 0}
  };
  const char* opt = "g::o:abhVvu";
  char* buffer;
  FILE* stream = stdout;
  bool all = false;
  bool unique = false;
  size_t size = 9;
  while ((optc = getopt_long (argc, argv, opt, longopts, NULL)) != -1) {
    args = optind;
    switch (optc)
    {
      case 'a':
        all = true;
        break;

      case 'u':
        unique = true;
        break;

      case 'h':
        buffer = 
          "Usage: sudoku [-a | -o FILE | -v | -V | -h] FILE...\n"
          "       sudoku -g[SIZE] [-u | -o FILE | -v | -V | -h]\n"
          "Solve or generate Sudoku grids of various sizes (1,4,9,16,25,36,49,64)\n"
          "\n"
          " -a, --all              search for all possible solutions\n"
          " -g[N], --generate[=N]  generate a grid of size NxN (default:9)\n"
          " -u, --unique           generate a grid with unique solution\n"
          " -o FILE, --output FILE write solution to FILE\n"
          " -v, --verbose          verbose output\n"
          " -V, --version          display version and exit\n"
          " -h, --help             display this help and exit\n";
        fputs(buffer, stream);
        return EXIT_SUCCESS;

      case 'V':
        buffer = 
          "sudoku %d.%d.%d\n"
          "Solve/generate sudoku grids (possible sizes: 1,4,9,16,25,36,49,64)\n";
        fprintf(stream, buffer, VERSION, SUBVERSION, REVISION);
        return EXIT_SUCCESS;

      case 'v':
        verbose = true;
        break;

      case 'o':
        if (!has_output_file) {
          has_output_file = true;
          stream = fopen(optarg, "w");
          if (!stream)
            errx(EXIT_FAILURE, "error: invalid output file %s", optarg);
        }
        else
          warnx("warning: 2 output files detected, use only the first one!");
        break;

      case 'g':
        solver = false;
        if (optarg) {
          size = atoi(optarg);
          if (!grid_check_size(size))
            errx(EXIT_FAILURE, "error: invalid grid size, only (1,4,9,16,25,36,49,64)!");
        }
        break;

      default:
        errx(EXIT_FAILURE, "error: invalid option - please check syntax with './sudoku -h or --help'");
    }
  }
  if (solver && unique) {
    warnx("warning: option 'unique' conflict with the solver mode, disabled");
    unique = false;
  }
  else if (!solver && all) {
    warnx("warning: option 'all' conflict with the generator mode, disabled");
    all = false;
  }

  FILE *file;
  bool all_good = true;
  if (solver) {
    if (args == argc)
      errx(EXIT_FAILURE, "error: no input grid given!");
    srand(time(NULL) - getpid());
    for (int i = args; i < argc; i++) {
      if ((file = fopen(argv[i], "r")) == NULL)
        errx(EXIT_FAILURE, "error: file %s can not be read!", argv[i]);
      fprintf(stream, "Solving : %s\n", argv[i]);
      grid_t *grid = file_parser(argv[i]);
      if (grid == NULL) {
        fclose(file);
        all_good = false;
        continue;
      }
      mode_t mode = (all) ? mode_all : mode_first;
      bool random = (all) ? false : true;
      solutions = 0;
      grid = grid_solver(grid, mode, stream, random);
      if (!grid) {
        warnx("error: the initial grid is inconsistent!");
        all_good = false;
      }
      fprintf(stream, "Number of solutions: %ld \n", solutions);
      grid_free(grid);
      fclose(file);
    }
  }
  else {
    srand(time(NULL) - getpid());
    mode_t mode = (unique) ? mode_unique : mode_first;
    grid_t *grid = grid_generator(size, mode);
    grid_print(grid, stream);
    grid_free(grid);
  }
  fclose(stream);
  if (!all_good)
    return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
