#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Tile {
  size_t distance;
  bool start;
  bool end;
  bool climbed;
  int pos[2];
  char height;
} Tile;

typedef struct Grid {
  size_t width;
  size_t height;
  size_t startpos[2];
  size_t endpos[2];
  Tile tiles[];
} Grid;

Tile *getTile(Grid *mygrid, size_t x, size_t y) {
  assert(x < mygrid->width);
  assert(y < mygrid->height);
  return &mygrid->tiles[mygrid->width * y + x];
}

Grid *initGrid(char *filename) {
  Grid *mygrid = calloc(sizeof(Grid) + sizeof(Tile) * 1024 * 1024, 1);
  assert(mygrid);

  FILE *fh = fopen(filename, "rb");
  assert(fh);
  for (;;) {
    char line[1024];
    memset(line, 0x0, sizeof(line));
    if (EOF == fscanf(fh, "%[^\r\n]", line)) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    if (!mygrid->width) {
      mygrid->width = strlen(line);
    }
    for (size_t i = 0; i < strlen(line); i++) {
      Tile *mytile = &mygrid->tiles[mygrid->height * mygrid->width + i];
      mytile->height = line[i];
      mytile->pos[0] = i;
      mytile->pos[1] = mygrid->height;
      mytile->distance = -1;
      switch (line[i]) {
      case 'S': {
        mytile->start = true;
        mytile->height = 'a';
        mygrid->startpos[0] = i;
        mygrid->startpos[1] = mygrid->height;
        break;
      }
      case 'E': {
        mytile->end = true;
        mytile->height = 'z';
        mygrid->endpos[0] = i;
        mygrid->endpos[1] = mygrid->height;
        break;
      }
      default:
        break;
      }
    }
    mygrid->height++;
  }
  fclose(fh);

  return mygrid;
}

#define FREEGRID(_grid)                                                        \
  do {                                                                         \
    free(_grid);                                                               \
    _grid = NULL;                                                              \
  } while (0)

void printGrid(Grid *mygrid) {
  printf("Width: %zd, Height: %zd\n", mygrid->width, mygrid->height);
  for (size_t y = 0; y < mygrid->height; y++) {
    for (size_t x = 0; x < mygrid->width; x++) {
      Tile *mytile = getTile(mygrid, x, y);
      printf("%c", mytile->height);
    }
    printf("\n");
  }
}

void printDistances(Grid *mygrid) {
  for (size_t y = 0; y < mygrid->height; y++) {
    for (size_t x = 0; x < mygrid->width; x++) {
      Tile *mytile = getTile(mygrid, x, y);
      if (mytile->climbed) {
        printf("%3zd ", mytile->distance);
      } else {
        printf("x");
      }
    }
    printf("\n");
  }
}

void climb(Grid *mygrid, Tile *me) {
  if (!me->climbed) { // in the beginning...
    me->climbed = true;
    me->distance = 0;
  }

  int incr[] = {-1, 0, 1, 0, 0, -1, 0, 1};
  for (size_t i = 0; i < 7; i += 2) {
    int x = me->pos[0] + incr[i];
    int y = me->pos[1] + incr[i + 1];
    if (x >= 0 && x < mygrid->width && y >= 0 && y < mygrid->height) {
      Tile *tileToClimb = getTile(mygrid, x, y);
      if (!tileToClimb->climbed || tileToClimb->distance > me->distance + 1) {
        if (tileToClimb->height >= me->height - 1) {
          tileToClimb->distance = me->distance + 1;
          tileToClimb->climbed = true;
          climb(mygrid, tileToClimb);
        }
      }
    }
  }
}

void part1(char *filename) {}

int main(int argc, char **argv) {
  assert(argc == 2);
  char *filename = argv[1];
  Grid *mygrid = initGrid(filename);
  printGrid(mygrid);
  printf("\n");

  Tile *endTile = getTile(mygrid, mygrid->endpos[0], mygrid->endpos[1]);
  climb(mygrid, endTile);

  printf("Shortest distance from start to finish (part1): %zd\n",
         getTile(mygrid, mygrid->startpos[0], mygrid->startpos[1])->distance);

  size_t shortestDistanceForStaringPoint = 0;
  for (size_t y = 0; y < mygrid->height; y++) {
    for (size_t x = 0; x < mygrid->width; x++) {
      Tile *mytile = getTile(mygrid, x, y);
      if (mytile->height == 'a') {
        if (mytile->distance < shortestDistanceForStaringPoint ||
            !shortestDistanceForStaringPoint) {
          shortestDistanceForStaringPoint = mytile->distance;
        }
      }
    }
  }
  printf("Shortest distance for starting point (part2): %zd\n",
         shortestDistanceForStaringPoint);

  FREEGRID(mygrid);
  return 0;
}