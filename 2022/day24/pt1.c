#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef union Pos {
  struct {
    int32_t x;
    int32_t y;
  };
  int32_t coord[2];
} Pos;

#define NODISTANCECALCULATED -1
typedef enum DIRECTIONS { UP, RIGHT, DOWN, LEFT, LASTDIRECTION } DIRECTIONS;
typedef enum TILETYPE { WALL, EMPTY, BLIZZARDS } TILETYPE;

typedef struct Grid Grid;

typedef struct Tile {
  TILETYPE type;
  Pos loc;
  size_t blizzards[4];
  size_t tmpBlizzards[4];
  ssize_t distance;
  ssize_t tmpDistance;
  Grid *g;
} Tile;

typedef struct Grid {
  size_t width;
  size_t height;
  Tile *startSquare;
  Tile *endSquare;
  Tile tiles[];
} Grid;

Grid *initGrid() {
  Grid *g = calloc(sizeof(Grid) + sizeof(Tile) * 200 * 200, 1);
  assert(g);
  return g;
}

void parseLine(Grid *g, char *line) {
  if (!g->width) {
    g->width = strlen(line);
  }
  for (size_t i = 0; i < strlen(line); i++) {
    Tile *t = &g->tiles[g->height * g->width + i];
    t->g = g;
    t->loc.x = i;
    t->loc.y = g->height;
    t->distance = NODISTANCECALCULATED;
    t->tmpDistance = NODISTANCECALCULATED;
    switch (line[i]) {
    case '#': {
      t->type = WALL;
      break;
    }
    case '.': {
      t->type = EMPTY;
      break;
    }
    case '^': {
      t->type = BLIZZARDS;
      t->blizzards[UP] = 1;
      break;
    }
    case '>': {
      t->type = BLIZZARDS;
      t->blizzards[RIGHT] = 1;
      break;
    }
    case 'v': {
      t->type = BLIZZARDS;
      t->blizzards[DOWN] = 1;
      break;
    }
    case '<': {
      t->type = BLIZZARDS;
      t->blizzards[LEFT] = 1;
      break;
    }
    }
  }
  g->height++;
}

void printGrid(Grid *g) {
  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = &g->tiles[y * g->width + x];
      if (t->type == WALL) {
        printf(" # ");
      } else if (t->type == EMPTY) {
        if (t->distance != NODISTANCECALCULATED) {
          printf(" %.2zd", t->distance);
        } else {
          printf(" . ");
        }
      } else {
        size_t totalNumBlizzards = 0;
        for (size_t i = 0; i < 4; i++) {
          totalNumBlizzards += t->blizzards[i];
        }
        assert(totalNumBlizzards);
        if (totalNumBlizzards > 1) {
          printf(" %zd ", totalNumBlizzards);
        } else {
          for (size_t i = 0; i < 4; i++) {
            if (t->blizzards[i]) {
              switch (i) {
              case UP: {
                printf(" ^ ");
                break;
              }
              case DOWN: {
                printf(" v ");
                break;
              }
              case RIGHT: {
                printf(" > ");
                break;
              }
              case LEFT: {
                printf(" < ");
                break;
              }
              }
              break;
            }
          }
        }
      }
    }
    printf("\n");
  }
}

size_t numBlizzards(Tile *t) {
  size_t num = 0;
  for (size_t i = 0; i < 4; i++) {
    num += t->blizzards[i];
  }
  return num;
}

Tile *getNextBlizzardTile(Tile *t, DIRECTIONS dir) {
  ssize_t x = t->loc.x;
  ssize_t y = t->loc.y;
  switch (dir) {
  case UP: {
    do {
      y = y - 1 < 0 ? t->g->height - 1 : y - 1;
    } while ((t->g->tiles[y * t->g->width + x]).type == WALL);
    break;
  }
  case DOWN: {
    do {
      y = y + 1 == t->g->height ? 0 : y + 1;
    } while ((t->g->tiles[y * t->g->width + x]).type == WALL);
    break;
  }
  case LEFT: {
    do {
      x = x - 1 < 0 ? t->g->width - 1 : x - 1;
    } while ((t->g->tiles[y * t->g->width + x]).type == WALL);
    break;
  }
  case RIGHT: {
    do {
      x = x + 1 == t->g->width ? 0 : x + 1;
    } while ((t->g->tiles[y * t->g->width + x]).type == WALL);
    break;
  }
  default:
    assert(false);
  }

  return &(t->g->tiles[y * t->g->width + x]);
}

void moveBlizzards(Grid *g) {
  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = &g->tiles[y * g->width + x];
      if (t->type == WALL) {
        continue;
      }
      if (!numBlizzards(t)) {
        continue;
      }
      for (size_t i = 0; i < 4; i++) {
        if (t->blizzards[i]) {
          Tile *next = getNextBlizzardTile(t, i);
          next->tmpBlizzards[i] += t->blizzards[i];
        }
      }
    }
  }

  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = &g->tiles[y * g->width + x];
      if (t->type == WALL) {
        continue;
      }
      memcpy(t->blizzards, t->tmpBlizzards, sizeof(t->tmpBlizzards[0]) * 4);
      memset(t->tmpBlizzards, 0x0, sizeof(t->tmpBlizzards[0]) * 4);
      size_t winds = numBlizzards(t);
      if (!winds) {
        t->type = EMPTY;
      } else {
        t->type = BLIZZARDS;
        t->distance = NODISTANCECALCULATED; // todo
      }
    }
  }
}

void getStartStopSquares(Grid *g) {
  for (size_t x = 0; x < g->width; x++) {
    Tile *t = &g->tiles[x];
    if (t->type != WALL) {
      g->startSquare = t;
      g->startSquare->distance = 0;
      assert(g->startSquare->type == EMPTY);
      break;
    }
  }

  for (size_t x = 0; x < g->width; x++) {
    Tile *t = &g->tiles[(g->height - 1) * g->width + x];
    if (t->type != WALL) {
      g->endSquare = t;
      assert(g->endSquare->type == EMPTY);
      break;
    }
  }
}

Tile *getNextTile(Tile *t, DIRECTIONS dir) {
  ssize_t x = t->loc.x;
  ssize_t y = t->loc.y;
  switch (dir) {
  case UP: {
    if (y == 0) {
      return NULL;
    }
    y--;
    break;
  }
  case DOWN: {
    if (y == t->g->height - 1) {
      return NULL;
    }
    y++;
    break;
  }
  case LEFT: {
    if (x == 0) {
      return NULL;
    }
    x--;
    break;
  }
  case RIGHT: {
    if (x == t->g->width - 1) {
      return NULL;
    }
    x++;
    break;
  }
  default:
    assert(false);
  }

  if (t->g->tiles[y * t->g->width + x].type == WALL) {
    return NULL;
  }
  return &(t->g->tiles[y * t->g->width + x]);
}

void propagate(Grid *g) {
  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = &g->tiles[y * g->width + x];
      if (t->type == WALL) {
        continue;
      }
      if (t->distance == NODISTANCECALCULATED) {
        continue;
      }
      for (size_t dir = 0; dir != LASTDIRECTION; dir++) {
        Tile *next = getNextTile(t, dir);
        if (!next) {
          continue;
        }
        if (next->distance > t->distance + 1 ||
            next->distance == NODISTANCECALCULATED) {
          if (next->tmpDistance > t->distance + 1 ||
              next->tmpDistance == NODISTANCECALCULATED) {
            next->tmpDistance = t->distance + 1;
          }
        }
      }
      if (t->tmpDistance > t->distance + 1 ||
          t->tmpDistance == NODISTANCECALCULATED) {
        t->tmpDistance = t->distance + 1; // wait-in-place
      }
    }
  }

  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = &g->tiles[y * g->width + x];
      if (t->tmpDistance != NODISTANCECALCULATED) {
        t->distance = t->tmpDistance;
        t->tmpDistance = NODISTANCECALCULATED;
      }
    }
  }
}

void resetAndSwitchStartAndEnd(Grid *g) {
  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = &g->tiles[y * g->width + x];
      t->distance = NODISTANCECALCULATED;
      t->tmpDistance = NODISTANCECALCULATED;
    }
  }
  Tile *tmp = g->endSquare;
  g->endSquare = g->startSquare;
  g->startSquare = tmp;
  g->startSquare->distance = 0;
}

void play(Grid *g) {
  size_t totalMoves = 0;
  for (size_t i = 0; i < 3; i++) {
    for (;;) {
      // printGrid(g);
      // getchar();
      propagate(g);
      moveBlizzards(g);
      if (g->endSquare->distance != NODISTANCECALCULATED) {
        totalMoves += g->endSquare->distance;
        if (!i) {
          printf("[part1]: %zd\n", totalMoves);
        }
        break;
      }
    }
    resetAndSwitchStartAndEnd(g);
  }
  printf("[part2]: %zd\n", totalMoves);
}

int main(int argc, char **argv) {
  Grid *g = initGrid();
  FILE *fh = fopen(argv[1], "rb");
  for (;;) {
    char line[1024] = {0};
    if (fscanf(fh, "%[^\r\n]", line) == EOF) {
      break;
    }
    parseLine(g, line);
    fscanf(fh, "%*[\r\n]");
  }
  fclose(fh);
  getStartStopSquares(g);
  play(g);
  free(g);
  return 0;
}