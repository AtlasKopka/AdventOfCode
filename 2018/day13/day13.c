#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRAIGHT UP
typedef enum direction { LEFT, UP, RIGHT, DOWN } direction;

typedef struct car {
  bool crashed;
  direction currentDirection;
  direction nextDirection;
  size_t x, y;
} car;

typedef struct tile tile;
typedef struct tile {
  tile *connections[4];
  car *currentCar;
  size_t x, y;
  char glyph;
} tile;

typedef struct grid {
  size_t width, height;
  size_t numCars;
  ssize_t numCarsRemaining;
  car cars[64];
  tile tiles[200 * 200];
} grid;

int carcmp(const void *a, const void *b) {
  if (((car *)a)->y < ((car *)b)->y) {
    return -1;
  }
  if (((car *)a)->y > ((car *)b)->y) {
    return 1;
  }
  if (((car *)a)->x < ((car *)b)->x) {
    return -1;
  }
  if (((car *)a)->x > ((car *)b)->x) {
    return 1;
  }
  return 0;
}

void sortCars(grid *g) { qsort(g->cars, g->numCars, sizeof(car), carcmp); }

grid *initGrid() {
  grid *g = calloc(sizeof(grid), 1);
  return g;
}

void parse(grid *g, char *line) {
  for (size_t i = 0; i < strlen(line); i++) {
    tile *t = &g->tiles[g->height * g->width + i];
    t->x = i;
    t->y = g->height;
    t->glyph = line[i];
    if (t->glyph == '<' || t->glyph == '>') {
      t->glyph = '-';
    } else if (t->glyph == 'v' || t->glyph == '^') {
      t->glyph = '|';
    }
    memset(t->connections, 0x0, sizeof(t->connections[0]) * 4);

    switch (line[i]) {
    case '|':
    case '^':
    case 'v': {
      t->connections[DOWN] = &g->tiles[(g->height + 1) * g->width + i];
      t->connections[UP] = &g->tiles[(g->height - 1) * g->width + i];

      if (line[i] != '|') {
        car *c = &g->cars[g->numCars++];
        if (line[i] == '^') {
          c->currentDirection = UP;
        } else {
          c->currentDirection = DOWN;
        }
        c->x = t->x;
        c->y = t->y;
        t->currentCar = c;
      }
      break;
    }
    case '-':
    case '<':
    case '>': {
      t->connections[RIGHT] = &g->tiles[(g->height) * g->width + i + 1];
      t->connections[LEFT] = &g->tiles[(g->height) * g->width + i - 1];

      if (line[i] != '-') {
        car *c = &g->cars[g->numCars++];
        if (line[i] == '>') {
          c->currentDirection = RIGHT;
        } else {
          c->currentDirection = LEFT;
        }
        c->x = t->x;
        c->y = t->y;
        t->currentCar = c;
      }
      break;
    }
    case '/': {
      if (i && g->tiles[g->height * g->width + i - 1].connections[RIGHT] == t) {
        t->connections[LEFT] = &g->tiles[(g->height) * g->width + i - 1];
        t->connections[UP] = &g->tiles[(g->height - 1) * g->width + i];
      } else {
        t->connections[RIGHT] = &g->tiles[(g->height) * g->width + i + 1];
        t->connections[DOWN] = &g->tiles[(g->height + 1) * g->width + i];
      }
      break;
    }
    case '\\': {
      if (g->tiles[g->height * g->width + i - 1].connections[RIGHT] == t) {
        t->connections[LEFT] = &g->tiles[(g->height) * g->width + i - 1];
        t->connections[DOWN] = &g->tiles[(g->height + 1) * g->width + i];
      } else {
        t->connections[RIGHT] = &g->tiles[(g->height) * g->width + i + 1];
        t->connections[UP] = &g->tiles[(g->height - 1) * g->width + i];
      }
      break;
    }
    case '+': {
      t->connections[LEFT] = &g->tiles[(g->height) * g->width + i - 1];
      t->connections[DOWN] = &g->tiles[(g->height + 1) * g->width + i];
      t->connections[RIGHT] = &g->tiles[(g->height) * g->width + i + 1];
      t->connections[UP] = &g->tiles[(g->height - 1) * g->width + i];
    }
    case ' ': {
      break;
    }
    default:
      assert(false);
    }
  }
}

void printGrid(grid *g) {
  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      bool carPrinted = false;
      for (size_t i = 0; i < g->numCars; i++) {
        if (g->cars[i].crashed) {
          continue;
        }
        if (g->cars[i].x == x && g->cars[i].y == y) {
          switch (g->cars[i].currentDirection) {
          case UP: {
            printf("^");
            break;
          }
          case DOWN: {
            printf("v");
            break;
          }
          case RIGHT: {
            printf(">");
            break;
          }
          case LEFT: {
            printf("<");
            break;
          }
          default: {
            assert(false);
          }
          }
          carPrinted = true;
        }
      }
      if (!carPrinted) {
        printf("%c", g->tiles[y * g->width + x].glyph);
      }
    }
    printf("\n");
  }
}

void simulate(grid *g) {
  g->numCarsRemaining = g->numCars;
  for (size_t tick = 0;; tick++) {
    sortCars(g);
    for (size_t i = 0; i < g->numCars; i++) {
      car *c = &g->cars[i];
      if (c->crashed) {
        continue;
      }
      tile *t = &g->tiles[c->y * g->width + c->x];
      tile *nextTile = NULL;
      switch (t->glyph) {
      case '+': {
        if (c->nextDirection == STRAIGHT) {
          nextTile = t->connections[c->currentDirection];
        } else {
          if (c->currentDirection == RIGHT) {
            if (c->nextDirection == RIGHT) {
              c->currentDirection = DOWN;
            } else {
              c->currentDirection = UP;
            }
          } else if (c->currentDirection == LEFT) {
            if (c->nextDirection == RIGHT) {
              c->currentDirection = UP;
            } else {
              c->currentDirection = DOWN;
            }
          } else if (c->currentDirection == UP) {
            if (c->nextDirection == LEFT) {
              c->currentDirection = LEFT;
            } else {
              c->currentDirection = RIGHT;
            }
          } else {
            if (c->nextDirection == RIGHT) {
              c->currentDirection = LEFT;
            } else {
              c->currentDirection = RIGHT;
            }
          }
          nextTile = t->connections[c->currentDirection];
        }
        c->nextDirection = (c->nextDirection + 1) % 3;
        break;
      }
      case '|':
      case '-': {
        nextTile = t->connections[c->currentDirection];
        break;
      }
      case '/':
      case '\\': {
        if (c->currentDirection == RIGHT || c->currentDirection == LEFT) {
          if (t->connections[UP]) {
            c->currentDirection = UP;
          } else {
            c->currentDirection = DOWN;
          }
        } else {
          if (t->connections[LEFT]) {
            c->currentDirection = LEFT;
          } else {
            c->currentDirection = RIGHT;
          }
        }
        nextTile = t->connections[c->currentDirection];
        break;
      }
      case ' ':
        break;
      default:
        printf("Unupported glyph: %c\n", t->glyph);
        assert(false);
      }

      if (nextTile->currentCar) {
        if (g->numCarsRemaining == g->numCars) {
          printf("[part1]: %zd,%zd\n", nextTile->x, nextTile->y);
        }
        c->crashed = true;
        nextTile->currentCar->crashed = true;
        g->numCarsRemaining -= 2;
        t->currentCar = NULL;
        nextTile->currentCar = NULL;
      } else {
        c->x = nextTile->x;
        c->y = nextTile->y;
        t->currentCar = NULL;
        nextTile->currentCar = c;
      }
    }
    if (g->numCarsRemaining <= 1) {
      for (size_t i = 0; i < g->numCars; i++) {
        if (!g->cars[i].crashed) {
          printf("[part2]: %zd,%zd\n", g->cars[i].x, g->cars[i].y);
          return;
        }
      }
    }
  }
}

int main(int argc, char **argv) {
  grid *g = initGrid();
  FILE *fh = fopen(argv[1], "rb");
  for (;;) {
    char line[256] = {0};
    if (fscanf(fh, "%[^\r\n]", line) == EOF) {
      break;
    }
    if (!g->width) {
      g->width = strlen(line);
    }
    fscanf(fh, "%*[\r\n]");
    parse(g, line);
    g->height++;
  }
  fclose(fh);
  simulate(g);
  free(g);
  return 0;
}
