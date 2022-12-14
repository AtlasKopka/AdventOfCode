#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum Material { empty, rock, stillsand, activesand } Material;

typedef struct Tile {
  size_t pos[2];
  Material material;
} Tile;

#define GRIDWIDTH 1024
#define GRIDHEIGHT 1024

typedef struct Grid {
  size_t maxWidth, maxHeight;
  ssize_t minRockWidth, minRockHeight;
  Tile tiles[GRIDWIDTH * GRIDHEIGHT];
} Grid;

#define GETTILE(_gridptr, x, y) (&_gridptr->tiles[(y)*GRIDWIDTH + (x)])

Grid *initGrid() {
  Grid *mygrid = calloc(sizeof(Grid), 1);
  for (size_t y = 0; y < GRIDHEIGHT; y++) {
    for (size_t x = 0; x < GRIDWIDTH; x++) {
      GETTILE(mygrid, x, y)->pos[0] = x;
      GETTILE(mygrid, x, y)->pos[1] = y;
    }
  }
  mygrid->minRockWidth = -1;
  mygrid->minRockHeight = -1;
  return mygrid;
}

#define FREEGRID(_gridptr)                                                     \
  do {                                                                         \
    free(_gridptr);                                                            \
    _gridptr = NULL;                                                           \
  } while (0)

void printGrid(Grid *mygrid) {
  for (size_t y = 0; y < mygrid->maxHeight; y++) {
    printf("%.3zd ", y);
    for (size_t x = mygrid->minRockWidth - 1; x < mygrid->maxWidth; x++) {
      switch (GETTILE(mygrid, x, y)->material) {
      case (rock): {
        printf("#");
        break;
      }
      case (stillsand): {
        printf("o");
        break;
      }
      case (activesand): {
        printf("+");
        break;
      }
      case (empty): {
        printf(".");
        break;
      }
      }
    }
    printf("\n");
  }
}

void addRock(Grid *mygrid, ssize_t *pos0, ssize_t *pos1) {
  ssize_t *tmp;
  if (pos0[0] > pos1[0]) {
    tmp = pos0;
    pos0 = pos1;
    pos1 = tmp;
  } else if (pos0[1] > pos1[1]) {
    tmp = pos0;
    pos0 = pos1;
    pos1 = tmp;
  }

  for (size_t x = pos0[0]; x <= pos1[0]; x++) {
    for (size_t y = pos0[1]; y <= pos1[1]; y++) {
      GETTILE(mygrid, x, y)->material = rock;
      if (x >= mygrid->maxWidth) {
        mygrid->maxWidth = x + 1;
      }
      if (y >= mygrid->maxHeight) {
        mygrid->maxHeight = y + 1;
      }
      if (x <= mygrid->minRockWidth || mygrid->minRockWidth == -1) {
        mygrid->minRockWidth = x;
      }
      if (y <= mygrid->minRockHeight || mygrid->minRockHeight == -1) {
        mygrid->minRockHeight = y;
      }
    }
  }
}

void parseLine(char *line, Grid *mygrid) {
  ssize_t pos0[2] = {-1};
  ssize_t pos1[2] = {-1};

  size_t pos = 0;
  for (;;) {
    char coord[1024] = {0};
    if (!sscanf(line + pos, "%[0-9,]", coord)) {
      break;
    }
    pos += strlen(coord);

    ssize_t *firstCoord = &pos0[0];
    ssize_t *secondCoord = &pos0[1];
    if (*firstCoord != -1) {
      firstCoord = &pos1[0];
      secondCoord = &pos1[1];

      if (*firstCoord != -1) {
        memcpy(pos0, pos1, sizeof(pos0));
      }
    }

    sscanf(coord, "%zd,%zd", firstCoord, secondCoord);
    if (pos1[0] != -1) {
      addRock(mygrid, pos0, pos1);
    }

    char skip[20] = {0};
    if (!sscanf(line + pos, "%[^0-9\r\n]", skip)) {
      break;
    }
    if (!strlen(skip)) {
      break;
    }
    pos += strlen(skip);
  }
}

Grid *parseFile(char *filename) {
  Grid *mygrid = initGrid();
  FILE *fh = fopen(filename, "rb");
  assert(fh);
  for (;;) {
    char line[1024];
    memset(line, 0x0, sizeof(line));
    if (fscanf(fh, "%[^\r\n]", line) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    parseLine(line, mygrid);
  }
  fclose(fh);

  return mygrid;
}

bool traceSand(Grid *mygrid, Tile *mytile, bool traceTop) {
  assert(mytile->material == activesand);

  if (!traceTop && mytile->pos[1] >= mygrid->maxHeight) {
    return true;
  }

  Tile *down = GETTILE(mygrid, mytile->pos[0], mytile->pos[1] + 1);
  if (down->material == empty) {
    mytile->material = empty;
    down->material = activesand;
    return traceSand(mygrid, down, traceTop);
  }
  Tile *lowerLeft = GETTILE(mygrid, mytile->pos[0] - 1, mytile->pos[1] + 1);
  if (lowerLeft->material == empty) {
    mytile->material = empty;
    lowerLeft->material = activesand;
    return traceSand(mygrid, lowerLeft, traceTop);
  }
  Tile *lowerRight = GETTILE(mygrid, mytile->pos[0] + 1, mytile->pos[1] + 1);
  if (lowerRight->material == empty) {
    mytile->material = empty;
    lowerRight->material = activesand;
    return traceSand(mygrid, lowerRight, traceTop);
  }

  // no where to go:
  mytile->material = stillsand;
  if (traceTop && mytile->pos[0] == 500 && mytile->pos[1] == 0) {
    return true;
  }
  return false;
}

void part1(char *filename) {
  Grid *mygrid = parseFile(filename);

  printf("Part1: ");
  for (size_t i = 0;; i++) {
    GETTILE(mygrid, 500, 0)->material = activesand;
    if (traceSand(mygrid, GETTILE(mygrid, 500, 0), false)) {
      printf("# of sand drops: %zd\n", i);
      break;
    }
  }

  FREEGRID(mygrid);
}

void part2(char *filename) {
  Grid *mygrid = parseFile(filename);

  printf("Part2: ");
  for (size_t x = 0; x < GRIDWIDTH; x++) {
    GETTILE(mygrid, x, mygrid->maxHeight + 1)->material = rock;
  }
  mygrid->maxHeight += 2;

  for (size_t i = 1;; i++) {
    GETTILE(mygrid, 500, 0)->material = activesand;
    if (traceSand(mygrid, GETTILE(mygrid, 500, 0), true)) {
      printf("# of sand drops: %zd\n", i);
      break;
    }
  }

  FREEGRID(mygrid);
}

int main(int argc, char **argv) {
  assert(argc == 2);
  char *filename = argv[1];
  part1(filename);
  part2(filename);

  return 0;
}