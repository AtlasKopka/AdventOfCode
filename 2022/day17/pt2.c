#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 7

typedef struct Tile {
  bool isRock;
} Tile;

typedef enum ShapeType {
  line,
  cross,
  lshape,
  downline,
  square,
  numShapeTypes
} ShapeType;

typedef struct Shape {
  size_t startX;
  size_t startY;
  size_t numSquares;
  size_t coords[10];
  ShapeType type;
  bool isPetrified;
  bool airJetSimulated;
} Shape;

typedef struct Grid {
  size_t numBlocksCompleted;
  size_t highestRockY;
  size_t highestRockX;
  size_t airJetSequencePos;
  char airJetSequence[1024 * 1024];
  Tile tiles[4096 * 4096];
} Grid;

#define getTile(g, x, y) (&(g)->tiles[(y)*WIDTH + (x)])

bool isShapeOnSquare(Shape *s, size_t x, size_t y) {
  if (!s) {
    return false;
  }
  for (size_t i = 0; i < s->numSquares; i++) {
    if (s->coords[i * 2] == x && s->coords[i * 2 + 1] == y) {
      return true;
    }
  }
  return false;
}

void printGrid(Grid *g, Shape *s) {
  printf("|");
  for (size_t x = 0; x < 7; x++) {
    printf("_");
  }
  printf("|\n");
  size_t starty = 0;
  if (g->highestRockY > 100) {
    starty = g->highestRockY - 100;
  }
  for (size_t y = starty; y <= g->highestRockY + (WIDTH - 1); y++) {
    printf("%.5ld|", y);
    for (size_t x = 0; x < 7; x++) {
      if (getTile(g, x, y)->isRock) {
        if (g->highestRockX == x && g->highestRockY == y) {
          printf("H");
        } else {
          printf("#");
        }
      } else {
        if (isShapeOnSquare(s, x, y)) {
          printf("@");
        } else {
          printf(".");
        }
      }
    }
    printf("|\n");
  }
  printf("\n");
}

void setShapeStartPosition(Shape *s, Grid *g) {
  s->startX = 2;
  if (!g->numBlocksCompleted) {
    s->startY = 3;
  } else {
    s->startY = g->highestRockY + 4;
  }
}

void startShape(Shape *s, Grid *g) {
  ShapeType type = s->type;
  memset(s, 0x0, sizeof(Shape));
  s->type = type;

  setShapeStartPosition(s, g);

  switch (type) {
  case (cross): {
    s->numSquares = 5;

    s->coords[0] = s->startX + 1;
    s->coords[1] = s->startY;

    s->coords[2] = s->startX + 1;
    s->coords[3] = s->startY + 1;

    s->coords[4] = s->startX;
    s->coords[5] = s->startY + 1;

    s->coords[6] = s->startX + 2;
    s->coords[7] = s->startY + 1;

    s->coords[8] = s->startX + 1;
    s->coords[9] = s->startY + 2;
    break;
  }
  case (line): {
    s->numSquares = 4;

    s->coords[0] = s->startX;
    s->coords[1] = s->startY;

    s->coords[2] = s->startX + 1;
    s->coords[3] = s->startY;

    s->coords[4] = s->startX + 2;
    s->coords[5] = s->startY;

    s->coords[6] = s->startX + 3;
    s->coords[7] = s->startY;
    break;
  }
  case (lshape): {
    s->numSquares = 5;

    s->coords[0] = s->startX;
    s->coords[1] = s->startY;

    s->coords[2] = s->startX + 1;
    s->coords[3] = s->startY;

    s->coords[4] = s->startX + 2;
    s->coords[5] = s->startY;

    s->coords[6] = s->startX + 2;
    s->coords[7] = s->startY + 1;

    s->coords[8] = s->startX + 2;
    s->coords[9] = s->startY + 2;
    break;
  }
  case (downline): {
    s->numSquares = 4;

    s->coords[0] = s->startX;
    s->coords[1] = s->startY;

    s->coords[2] = s->startX;
    s->coords[3] = s->startY + 1;

    s->coords[4] = s->startX;
    s->coords[5] = s->startY + 2;

    s->coords[6] = s->startX;
    s->coords[7] = s->startY + 3;
    break;
  }
  case (square): {
    s->numSquares = 4;

    s->coords[0] = s->startX;
    s->coords[1] = s->startY;

    s->coords[2] = s->startX;
    s->coords[3] = s->startY + 1;

    s->coords[4] = s->startX + 1;
    s->coords[5] = s->startY;

    s->coords[6] = s->startX + 1;
    s->coords[7] = s->startY + 1;
    break;
  }
  default:
    return;
  }

  s->type++;
  s->type %= numShapeTypes;
}

void moveShape(Shape *s, Grid *g, char direction) {
  if (direction == 'v') {
    bool cannotMoveDown = false;
    for (size_t i = 0; i < s->numSquares; i++) {
      size_t x = s->coords[i * 2];
      size_t y = s->coords[i * 2 + 1];
      if (y == 0 || getTile(g, x, y - 1)->isRock) {
        cannotMoveDown = true;
        break;
      }
    }
    if (cannotMoveDown) {
      for (size_t i = 0; i < s->numSquares; i++) {
        size_t x = s->coords[i * 2];
        size_t y = s->coords[i * 2 + 1];
        getTile(g, x, y)->isRock = true;
        if (y > g->highestRockY) {
          g->highestRockY = y;
          g->highestRockX = x;
        }
      }
      s->isPetrified = true;
      return;
    } else {
      // move down:
      for (size_t i = 0; i < s->numSquares; i++) {
        s->coords[i * 2 + 1]--;
      }
      return;
    }
  }
  switch (direction) {
  case '<': {
    bool cannotMove = false;
    for (size_t i = 0; i < s->numSquares; i++) {
      size_t x = s->coords[i * 2];
      size_t y = s->coords[i * 2 + 1];
      if (x == 0 || getTile(g, x - 1, y)->isRock) {
        cannotMove = true;
        break;
      }
    }
    if (!cannotMove) {
      for (size_t i = 0; i < s->numSquares; i++) {
        s->coords[i * 2]--;
      }
    }
    break;
  }
  case '>': {
    bool cannotMove = false;
    for (size_t i = 0; i < s->numSquares; i++) {
      size_t x = s->coords[i * 2];
      size_t y = s->coords[i * 2 + 1];
      if (x == WIDTH - 1 || getTile(g, x + 1, y)->isRock) {
        cannotMove = true;
        break;
      }
    }
    if (!cannotMove) {
      for (size_t i = 0; i < s->numSquares; i++) {
        s->coords[i * 2]++;
      }
    }
    break;
  }
  default:
    assert(false);
  }
}

void readAirJetSequence(char *filename, Grid *g) {
  FILE *fh = fopen(filename, "rb");
  assert(fscanf(fh, "%[^\r\n]", g->airJetSequence));
}

char getNextAirJetSequnece(Grid *g) {

  char seq = g->airJetSequence[g->airJetSequencePos];
  g->airJetSequencePos++;
  g->airJetSequencePos %= strlen(g->airJetSequence);
  return seq;
}

uint64_t blockNumToHeight[1024 * 1024];

int main(int argc, char **argv) {
  Grid *g = calloc(sizeof(Grid), 1);
  readAirJetSequence(argv[1], g);

  memset(blockNumToHeight, 0x0, sizeof(blockNumToHeight));

  Shape s = {0};
  bool done = false;
  while (!done) {
    if (s.isPetrified || !g->numBlocksCompleted) {
      size_t blocks = g->numBlocksCompleted;
      blockNumToHeight[blocks] = g->highestRockY + 1;
      if (blocks % 50000 == 0) {
        printf("%ld blocks completed\n", blocks);

        for (size_t windowSize = (blocks) / 3; windowSize > 10; windowSize--) {
          size_t blocksCompletedInWindow =
              blockNumToHeight[blocks] - blockNumToHeight[blocks - windowSize];

          size_t blocksCompletedInPreviousWindow =
              blockNumToHeight[blocks - windowSize - 1] -
              blockNumToHeight[blocks - windowSize * 2 - 1];

#ifdef debugme
          if (blocksCompletedInWindow == blocksCompletedInPreviousWindow) {
            printf("%ld blocks from %ld-%ld and %ld-%ld had the same height of "
                   "%ld\n",
                   windowSize, blocks - windowSize, blocks,
                   blocks - windowSize * 2 - 1, blocks - windowSize - 1,
                   blocksCompletedInWindow);
          }
#endif

          ssize_t l;
          size_t startingLocation;
          for (l = 1; blocks > windowSize * (l + 1) - 1 > 0; l++) {
            startingLocation = blocks - windowSize * (l + 1) - 1;
            size_t prev = blockNumToHeight[blocks - l * windowSize - 1] -
                          blockNumToHeight[startingLocation];
            if (prev == blocksCompletedInWindow) {
            } else {
              break;
            }
          }
          if (l >= 100) {
            uint64_t remainder =
                (1000000000000l - startingLocation) % (uint64_t)windowSize;
            uint64_t heightAddedByRemainder =
                blockNumToHeight[startingLocation + remainder] -
                blockNumToHeight[startingLocation];
            uint64_t answer = blockNumToHeight[startingLocation];
            answer += ((1000000000000l - startingLocation) / windowSize) *
                      (uint64_t)blocksCompletedInWindow;
            answer += heightAddedByRemainder;
            printf("%lu (part2)\n", answer);
            done = true;
            break;
          }
        }
      }
      if (g->numBlocksCompleted == 1000000) {
        printf("Did not find solution\n");
        break;
      }
      startShape(&s, g);
      g->numBlocksCompleted++;
    } else {
      if (s.airJetSimulated) {
        moveShape(&s, g, 'v');
        s.airJetSimulated = false;
      } else {
        moveShape(&s, g, getNextAirJetSequnece(g));
        s.airJetSimulated = true;
      }
    }
  }
  //}

  free(g);
  return 0;
}