#ifndef COLORS_H
#define COLORS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_COLORS 64

typedef uint64_t colors_t;

/* Initialize all colors within size */
colors_t colors_full(const size_t size);

/* Return empty color */
colors_t colors_empty ();

/* Return a color */
colors_t colors_set (const size_t color_id);

/* Add a color to an existing colors_t */
colors_t colors_add (const colors_t colors, const size_t color_id);

/* Discard a color to an existing colors_t */
colors_t colors_discard (const colors_t colors, const size_t color_id);

/* Check if color is in the color set */
bool colors_is_in (const colors_t colors, const size_t color_id);

/* Return complement of a colors_t */
colors_t colors_negate (const colors_t colors);

/* Intersection of two colors_t */
colors_t colors_and (const colors_t colors1, const colors_t colors2);

/* Union of two colors_t */
colors_t colors_or (const colors_t colors1, const colors_t colors2);

/* Exclusive union of two colors_t */
colors_t colors_xor (const colors_t colors1, const colors_t colors2);

/* Substraction between two colors_t */
colors_t colors_subtract (const colors_t colors1, const colors_t colors2);

/* Equality of two colors_t */
bool colors_is_equal (const colors_t colors1, const colors_t colors2);

/* Inclusion test between two colors_t */
bool colors_is_subset (const colors_t colors1, const colors_t colors2);

/* Check if colors_t is a singleton */
bool colors_is_singleton (const colors_t colors);

/* Return cardinality of colors_t */
size_t colors_count (const colors_t colors);

/* Rightmost color of a colors_t */
colors_t colors_rightmost (const colors_t colors);

/* Leftmost color of a colors_t */
colors_t colors_leftmost (const colors_t colors);

/* Return random color of a colors_t, PRNG need to be initialized with srand()
   before calling this function. */
colors_t colors_random(const colors_t colors);

/* Return cardinality and all colors in colors_t */
colors_t *colors_get_set (const colors_t colors);

/* Check if subgrid is consistent */
bool subgrid_consistency (const colors_t subgrid[], const size_t size);

/* Apply heristics to a subgrid */
bool subgrid_heuristics (colors_t *subgrid[], size_t size, size_t advance);

/* Apply cross hatching technique */
bool cross_hatching (colors_t *subgrid[], size_t size);

/* Apply lone number technique */
bool lone_number (colors_t *subgrid[], size_t size);

/* Apply naked subset technique */
bool naked_subset (colors_t *subgrid[], size_t size);

/* Apply hidden subset technique */
bool hidden_subset (colors_t *subgrid[], size_t size);

#endif /* COLORS_H */
