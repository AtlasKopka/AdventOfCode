#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t DIMENSIONX;
int32_t DIMENSIONY;

typedef struct loc {
  int32_t x, y;
  bool isInfinite;
  int32_t area;
} loc;

typedef struct tile {
  bool equidistant;
  loc *closest;
  int32_t dist;
  int64_t totalDist;
} tile;

typedef struct grid {
  int32_t minmax[4];
  int32_t width, height;
  size_t numLocs;
  loc locs[100];
  tile thegrid[];
} grid;

void fill(grid *g) {
  for (size_t i = 0; i < g->numLocs; i++) {
    loc *l = &g->locs[i];
    for (int32_t x = 0; x < DIMENSIONX; x++) {
      for (int32_t y = 0; y < DIMENSIONY; y++) {
        tile *t = &g->thegrid[y * DIMENSIONX + x];
        int32_t dist = abs(l->x - x) + abs(l->y - y);
        t->totalDist += dist;
        if (t->closest && t->closest != l && t->dist == dist) {
          t->closest = NULL;
          t->equidistant = true;
        } else if ((!t->closest && !t->equidistant) ||
                   (t->closest && t->dist > dist) ||
                   (t->equidistant && t->dist > dist)) {
          t->closest = l;
          t->dist = dist;
          t->equidistant = false;
        }
      }
    }
  }

  for (size_t x = 0; x < DIMENSIONX; x++) {
    for (size_t y = 0; y < DIMENSIONY; y += (DIMENSIONY - 1)) {
      if (g->thegrid[y * DIMENSIONX + x].closest) {
        g->thegrid[y * DIMENSIONX + x].closest->isInfinite = true;
      }
      if (g->thegrid[x * DIMENSIONX + y].closest) {
        g->thegrid[x * DIMENSIONX + y].closest->isInfinite = true;
      }
    }
  }

  int64_t areaWithinBounds = 0;
  for (size_t x = 0; x < DIMENSIONX; x++) {
    for (size_t y = 0; y < DIMENSIONY; y++) {
      loc *l = g->thegrid[y * DIMENSIONX + x].closest;
      if (l && !l->isInfinite) {
        l->area++;
      }
      if (g->thegrid[y * DIMENSIONX + x].totalDist < 10000) {
        areaWithinBounds++;
      }
    }
  }

  int32_t score = 0;
  loc *best;
  for (size_t i = 0; i < g->numLocs; i++) {
    if (g->locs[i].isInfinite) {
      continue;
    }
    if (g->locs[i].area > score) {
      score = g->locs[i].area;
      best = &g->locs[i];
    }
  }
  assert(best);
  printf("[part1]: %d\n", score);
  printf("[part2]: %ld\n", areaWithinBounds);
}

int main(int argc, char **argv) {
  FILE *fh = fopen(argv[1], "rb");
  int32_t maxx = 0, maxy = 0;
  for (;;) {
    int32_t x, y;
    if (fscanf(fh, "%d, %d", &x, &y) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    if (x > maxx) {
      maxx = x;
    }
    if (y > maxy) {
      maxy = y;
    }
    DIMENSIONX = maxx;
    DIMENSIONY = maxy;
  }

  grid *g = calloc(sizeof(grid) + sizeof(tile) * DIMENSIONX * DIMENSIONY, 1);

  fseek(fh, 0, SEEK_SET);
  for (;;) {
    int32_t x, y;
    if (fscanf(fh, "%d, %d", &x, &y) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    g->locs[g->numLocs].x = x;
    g->locs[g->numLocs].y = y;
    g->numLocs++;
  }
  fclose(fh);
  fill(g);
  free(g);
  return 0;
}