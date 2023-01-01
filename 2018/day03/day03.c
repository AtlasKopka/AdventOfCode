#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tile {
  size_t numIds;
  int16_t ids[10];
} tile;

typedef struct grid {
  size_t width;
  size_t height;
  size_t maxx;
  size_t maxy;
  tile tiles[];
} grid;

grid *initGrid() {
  grid *g = calloc(sizeof(grid) + sizeof(tile) * 1024 * 1024, 1);
  g->width = 1024;
  g->height = 1024;
  assert(g);
  return g;
}

void addPatch(grid *g, int32_t id, int32_t left, int32_t top, int32_t width,
              int32_t height) {
  for (size_t x = 0; x < width; x++) {
    for (size_t y = 0; y < height; y++) {
      size_t ycoord = y + top + 1;
      size_t xcoord = x + left + 1;
      tile *t = &g->tiles[ycoord * g->width + xcoord];
      t->ids[t->numIds] = id;
      t->numIds++;
      if (ycoord > g->maxy) {
        g->maxy = ycoord;
      }
      if (xcoord > g->maxx) {
        g->maxx = xcoord;
      }
    }
  }
}

void sumOverlaps(grid *g) {
  size_t overlaps = 0;
  for (size_t y = 0; y <= g->maxy; y++) {
    for (size_t x = 0; x < g->maxx; x++) {
      if (g->tiles[y * g->width + x].numIds >= 2) {
        overlaps++;
      }
    }
  }
  printf("There are %zd overlaps\n", overlaps);
}

void findFreeClaim(grid *g) {
  int8_t overlaps[4096] = {0};
  for (size_t x = 0; x <= g->maxx; x++) {
    for (size_t y = 0; y <= g->maxy; y++) {
      tile *t = &g->tiles[y * g->width + x];
      for (size_t i = 0; i < t->numIds; i++) {
        if (t->numIds > overlaps[t->ids[i]]) {
          overlaps[t->ids[i]] = t->numIds;
        }
      }
    }
  }
  for (size_t i = 0; i < sizeof(overlaps) / sizeof(overlaps[0]); i++) {
    if (overlaps[i] == 1) {
      printf("ID #%zu does not have any overlaps\n", i);
    }
  }
}

int main(int argc, char **argv) {
  grid *g = initGrid();
  FILE *fh = fopen(argv[1], "rb");
  for (;;) {
    int32_t id, left, top, width, height;
    if (fscanf(fh, "#%d @ %d,%d: %dx%d", &id, &left, &top, &width, &height) ==
        EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    addPatch(g, id, left, top, width, height);
  }
  fclose(fh);
  sumOverlaps(g);
  findFreeClaim(g);
  free(g);
  return 0;
}