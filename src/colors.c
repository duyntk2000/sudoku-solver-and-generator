#include "colors.h"

#include <stdlib.h>

colors_t colors_full (const size_t size)
{
  if (size >= MAX_COLORS)
    return UINT64_MAX;
  return (1ULL << size) - 1;
}

colors_t colors_empty ()
{
  return 0;
}

colors_t colors_set (const size_t color_id)
{
  if (color_id >= MAX_COLORS)
    return 0;
  return 1ULL << color_id;
}

colors_t colors_add (const colors_t colors, const size_t color_id)
{
  return colors | colors_set(color_id);
}

colors_t colors_discard (const colors_t colors, const size_t color_id)
{
  return colors & ~colors_set(color_id);
}

bool colors_is_in (const colors_t colors, const size_t color_id)
{ 
  if (color_id >= MAX_COLORS)
    return false;
  return (colors >> color_id) % 2 == 1;
}

colors_t colors_negate (const colors_t colors)
{ 
  return ~colors;
} 

colors_t colors_and (const colors_t colors1, const colors_t colors2)
{
  return colors1 & colors2;
}

colors_t colors_or (const colors_t colors1, const colors_t colors2)
{
  return colors1 | colors2;
}

colors_t colors_xor (const colors_t colors1, const colors_t colors2)
{
  return colors1 ^ colors2;
}

colors_t colors_subtract (const colors_t colors1, const colors_t colors2)
{
  return colors1 - (colors1 & colors2);
}

bool colors_is_equal (const colors_t colors1, const colors_t colors2)
{
  return colors1 == colors2;
}

bool colors_is_subset (const colors_t colors1, const colors_t colors2)
{ 
  return colors1 == (colors1 & colors2);
}

bool colors_is_singleton (const colors_t colors)
{
  if (colors == colors_empty())
    return false;

  return (colors & (colors - 1)) == 0;
}

size_t colors_count (const colors_t colors)
{ 
  colors_t x = colors;
  colors_t b5 = ~((-1ULL) << 32);
  colors_t b4 = b5 ^ (b5 << 16);
  colors_t b3 = b4 ^ (b4 << 8);
  colors_t b2 = b3 ^ (b3 << 4);
  colors_t b1 = b2 ^ (b2 << 2);
  colors_t b0 = b1 ^ (b1 << 1);

  x = ((x >> 1) & b0) + (x & b0);
  x = ((x >> 2) & b1) + (x & b1);
  x = ((x >> 4) + x) & b2;
  x = ((x >> 8) + x) & b3;
  x = ((x >> 16) + x) & b4;
  x = ((x >> 32) + x) & b5;
  return (size_t) x;
}

colors_t colors_rightmost (const colors_t colors)
{
  return colors & (~colors + 1);
}

colors_t colors_leftmost (const colors_t colors)
{
  if (colors == colors_empty())
    return colors_empty();

  colors_t x = colors >> 1;
  size_t count = 0; 
  while (x != 0) {
    x >>= 1;
    ++count;
  }
  
  return colors_set(count);
}

colors_t colors_random(const colors_t colors)
{
  if (!colors)
    return 0;
  
  size_t count = colors_count(colors);
  colors_t *set = malloc(count * sizeof(colors_t));
  if (!set)
    return 0;
  
  /* PRNG need to be initialized with srand() before calling this function */
  colors_t x = colors;
  size_t i = 0;
  size_t j = 0;
  while (x != 0) {
    if (x & 1)
      set[i++] = colors_set(j);
    x >>= 1;
    ++j;
  }
  x = set[rand() % count];
  free(set);
  return x;
}

bool subgrid_consistency (const colors_t subgrid[], const size_t size)
{ 
  colors_t singleton = 0;
  colors_t appeared = 0;
  for (size_t i = 0; i < size; ++i) {
    if (!subgrid[i]) {
      return false;
    }
    if (colors_is_singleton(subgrid[i])) {
      if ((subgrid[i] & singleton) == subgrid[i]) {
        return false;
      }
      singleton |= subgrid[i];
    }
    appeared |= subgrid[i];
  }

  return appeared == colors_full(size);
}

bool subgrid_heuristics(colors_t *subgrid[], size_t size, size_t level)
{ 
  if (level)
    return naked_subset(subgrid, size) || hidden_subset(subgrid, size);
  return cross_hatching(subgrid, size) | lone_number(subgrid, size);
}

bool cross_hatching (colors_t *subgrid[], size_t size)
{
  bool changed = false;
  colors_t colors = 0;
  for (size_t i = 0; i < size; ++i)
    if (colors_is_singleton(*subgrid[i]))
      colors |= *subgrid[i];
  for (size_t i = 0; i < size; ++i) {
    if (colors_is_singleton(*subgrid[i]))
      continue;

    colors_t new = colors_subtract(*subgrid[i], colors);
    if (*subgrid[i] != new) {
      changed = true;
      *subgrid[i] = new;
    }
  }
  return changed;
}

bool lone_number (colors_t *subgrid[], size_t size)
{
  bool changed = false;
  colors_t appeared = *subgrid[0];
  colors_t repeated = 0;
  for (size_t i = 1; i < size; ++i) {
    repeated |= appeared & *subgrid[i];
    appeared |= *subgrid[i];
  }
  colors_t lone = colors_subtract(appeared, repeated);
  if (lone == 0)
    return changed;

  for (size_t i = 0; i < size; ++i) {
    if (colors_is_singleton(*subgrid[i]))
      continue;
    colors_t new = *subgrid[i] & lone;
    if (colors_is_singleton(new)) {
      changed = true;
      *subgrid[i] = new;
    }
  }
  return changed;
}

bool naked_subset (colors_t *subgrid[], size_t size)
{ 
  bool changed = false;
  for (size_t i = 0; i < size; ++i) {
    if (colors_is_singleton(*subgrid[i]))
      continue;

    size_t count = 0;
    for (size_t j = 0; j < size; ++j) {
      if (colors_is_singleton(*subgrid[j]))
        continue;

      if (colors_is_subset(*subgrid[j], *subgrid[i]))
        ++count;
    }
    if (count != colors_count(*subgrid[i]))
      continue;
      
    for (size_t j = 0; j < size; ++j) {
      if (colors_is_subset(*subgrid[j], *subgrid[i]))
        continue;

      colors_t new = colors_subtract(*subgrid[j], *subgrid[i]);
      if (*subgrid[j] != new) {
        *subgrid[j] = new;
        changed = true;
      }
    }
  }
  return changed;
}

bool hidden_subset (colors_t *subgrid[], size_t size)
{
  bool changed = false;
  colors_t *position = malloc(size * sizeof(colors_t));
  if (!position)
    return changed;
  
  for (size_t i = 0; i < size; ++i) {
    position[i] = 0;
  }
  for (size_t i = 0; i < size; ++i) {
    for (size_t j = 0; j < size; ++j)
      if (colors_is_in(*subgrid[i], j))
        position[j] += colors_set(i);
  }

  for (size_t i = 0; i < size; ++i) {
    if (colors_is_singleton(position[i]))
      continue;

    colors_t set = 0;
    size_t count = 0;
    for (size_t j = 0; j < size; ++j) {
      if (colors_is_singleton(position[j]) || 
         !colors_is_subset(position[j], position[i]))
        continue;

      ++count;
      set += colors_set(j);
    }
    if (count != colors_count(position[i]))
      continue;

    for (size_t j = 0; j < size; ++j) {
      colors_t new = *subgrid[j] & set;
      if (new && new != *subgrid[j]) {
        *subgrid[j] = new;
        changed = true;
      }
    }
  }
  free(position);
  return changed;
}
