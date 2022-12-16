#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Polygon {
  size_t num;
  int64_t vertices[1024];
} Polygon;

typedef struct Polygons {
  size_t num;
  Polygon polygons[1024];
} Polygons;

void mergePolygons(Polygons *shapes) {
  for (size_t i = 0; i < shapes->num; i++) {
    for (size_t j = 0; j < shapes->num; j++) {
      if (i == j) {
        continue;
      }
    }
  }
}

void addPolygon(Polygons *shapes, int64_t *sensorpos, int64_t *beaconpos) {
  int64_t dist =
      labs(beaconpos[0] - sensorpos[0]) + labs(beaconpos[1] - sensorpos[1]);
  Polygon *pgon = &shapes->polygons[shapes->num];
  shapes->num++;

  // top
  pgon->vertices[pgon->num++] = sensorpos[0];
  pgon->vertices[pgon->num++] = sensorpos[1] - dist;

  // bottom
  pgon->vertices[pgon->num++] = sensorpos[0];
  pgon->vertices[pgon->num++] = sensorpos[1] + dist;

  // left
  pgon->vertices[pgon->num++] = sensorpos[0] - dist;
  pgon->vertices[pgon->num++] = sensorpos[1];

  // right
  pgon->vertices[pgon->num++] = sensorpos[0] + dist;
  pgon->vertices[pgon->num++] = sensorpos[1];
}

typedef struct Block {
  bool exists;
  int64_t startx;
  int64_t endx;
} Block;

typedef struct Grid {
  size_t numentries;
  Block blocks[1024 * 1024];
} Grid;

Grid *initGrid() {
  Grid *mygrid = calloc(sizeof(Grid), 1);
  return mygrid;
}

typedef struct BlockedRow {
  int64_t rownum;
  int64_t startx;
  int64_t endx;
  bool none;
} BlockedRow;

BlockedRow processSensor(int64_t sensorx, int64_t sensory, int64_t beaconx,
                         int64_t beacony, int64_t rowofinterest) {
  int64_t dist = labs(sensorx - beaconx) + labs(sensory - beacony);

  int64_t startrow = sensory - dist;
  int64_t endrow = sensory + dist;

  if (rowofinterest < startrow || rowofinterest > endrow) {
    BlockedRow row = {.none = true, .rownum = rowofinterest};
    return row;
  }

  int64_t remainder = dist - labs(rowofinterest - sensory);
  int64_t start_columnofinterest = sensorx - remainder;
  int64_t end_columnofinterest = sensorx + remainder;

  BlockedRow row = {.rownum = rowofinterest,
                    .startx = start_columnofinterest,
                    .endx = end_columnofinterest};

  return row;
}

void mergeBlocks(BlockedRow row, Grid *mygrid, int64_t rowofinterest) {
  if (!row.none) {
    mygrid->blocks[mygrid->numentries].exists = true;
    mygrid->blocks[mygrid->numentries].startx = row.startx;
    mygrid->blocks[mygrid->numentries].endx = row.endx;
    mygrid->numentries++;
  }
}

void mergesort(Grid *mygrid) {
  for (size_t i = 0; i < mygrid->numentries; i++) {
    for (size_t j = 0; j < mygrid->numentries; j++) {
      if (i == j) {
        continue;
      }
      Block *a = &mygrid->blocks[i];
      Block *b = &mygrid->blocks[j];
      if (!a->exists || !b->exists) {
        continue;
      }
      if (b->startx >= a->startx && b->endx <= a->endx) {
        b->exists = false;
        continue;
      }
      if (a->startx >= b->startx && a->endx <= b->endx) {
        a->exists = false;
        continue;
      }

      if (b->endx < a->startx - 1) {
        continue;
      }
      if (b->startx > a->endx + 1) {
        continue;
      }
      // else, there must be an overlap
      if (b->endx >= a->startx - 1 && b->endx <= a->endx) {
        a->startx = b->startx;
        b->exists = false;
        continue;
      }
      if (b->startx <= a->endx + 1 && b->startx >= a->startx) {
        a->endx = b->endx;
        b->exists = false;
        continue;
      }
    }
  }
}

typedef struct Coordinates {
  size_t size;
  int64_t coords[1024];
} Coordinates;

Coordinates *read(char *filename) {
  Coordinates *coords = calloc(sizeof(Coordinates), 1);
  assert(coords);
  FILE *fh = fopen(filename, "rb");
  assert(fh);
  for (;;) {
    char line[1024] = {0};
    if (fscanf(fh, "%[^\r\n]", line) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");

    for (size_t i = 0; i < strlen(line); i++) {
      if (line[i] == '=') {
        i++;
        assert(sscanf(line + i, "%ld", &coords->coords[coords->size]));
        coords->size++;
      }
    }
  }
  fclose(fh);
  return coords;
}

#define FREEGRID(_gridptr)                                                     \
  do {                                                                         \
    free(_gridptr);                                                            \
    (_gridptr) = NULL;                                                         \
  } while (0)

void part1(Coordinates *sensors) {
  Grid *mygrid = initGrid();
  int64_t rowofinterest = 2000000;

  for (size_t i = 0; i < sensors->size; i += 4) {
    int64_t sensorx = sensors->coords[i];
    int64_t sensory = sensors->coords[i + 1];
    int64_t beaconx = sensors->coords[i + 2];
    int64_t beacony = sensors->coords[i + 3];

    BlockedRow row =
        processSensor(sensorx, sensory, beaconx, beacony, rowofinterest);
    mergeBlocks(row, mygrid, rowofinterest);
  }
  mergesort(mygrid);

  int64_t numexcluded = 0;
  for (size_t i = 0; i < mygrid->numentries; i++) {
    if (mygrid->blocks[i].exists) {
      numexcluded += labs(mygrid->blocks[i].startx - mygrid->blocks[i].endx);
    }
  }
  printf("[part1]: %ld\n", numexcluded);
  FREEGRID(mygrid);
}

void part2_slow(Coordinates *sensors) {
  Grid *mygrid = initGrid();
  int64_t miny = 0;
  int64_t minx = 0;
  int64_t maxy = 4000000;
  int64_t maxx = 4000000;

  for (int64_t rowofinterest = miny; rowofinterest <= maxy; rowofinterest++) {
    if (rowofinterest % 1000 == 0) {
      printf("row %ld\n", rowofinterest);
    }
    memset(mygrid->blocks, 0x0, sizeof(mygrid->blocks));
    mygrid->numentries = 0;

    for (size_t i = 0; i < sensors->size; i += 4) {
      int64_t sensorx = sensors->coords[i];
      int64_t sensory = sensors->coords[i + 1];
      int64_t beaconx = sensors->coords[i + 2];
      int64_t beacony = sensors->coords[i + 3];

      BlockedRow row =
          processSensor(sensorx, sensory, beaconx, beacony, rowofinterest);
      mergeBlocks(row, mygrid, rowofinterest);
    }
    mergesort(mygrid);

    int64_t squaresRemaining = 0;
    int64_t squaresExcluded = 0;
    Block *a = NULL, *b = NULL;
    for (size_t i = 0; i < mygrid->numentries; i++) {
      if (mygrid->blocks[i].exists) {
        int64_t startx = mygrid->blocks[i].startx;
        int64_t endx = mygrid->blocks[i].endx;
        if (startx < minx) {
          startx = minx;
        }
        if (endx > maxx) {
          endx = maxx;
        }
        squaresExcluded += (endx - startx + 1);
        if (!a) {
          a = &mygrid->blocks[i];
        } else {
          b = &mygrid->blocks[i];
        }
      }
    }
    squaresRemaining = maxx - squaresExcluded + 1;
    if (squaresRemaining == 1) {
      int64_t x, y = rowofinterest;
      if (a->endx < b->startx) {
        x = a->endx + 1;
      } else {
        x = b->endx + 1;
      }
      printf("X: %ld, Y: %ld\n", x, y);
      int64_t score = x * 4000000 + y;
      printf("[part2]: %ld\n", score);
      break;
    }
  }

  FREEGRID(mygrid);
}

void part2_new(Coordinates *sensors) {
  Polygons *shapes = calloc(sizeof(Polygons), 1);

  for (size_t i = 0; i < sensors->size; i += 4) {
    int64_t sensorpos[2] = {sensors->coords[i], sensors->coords[i + 1]};
    int64_t beaconpos[2] = {sensors->coords[i + 2], sensors->coords[i + 3]};
    addPolygon(shapes, sensorpos, beaconpos);
  }
  printf("There are now %ld shapes\n", shapes->num);

  mergePolygons(shapes);

  free(shapes);
}

int main(int argc, char **argv) {
  Coordinates *sensors = read(argv[1]);
  part1(sensors);
  part2_slow(sensors);
  free(sensors);
  return 0;
}