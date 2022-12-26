// Herein is a generalized solution to Day 22, Part 02 of AOC 2022. ('22022022'
// - a palindrome!) This should work with any flattened cube that conforms to
// the rules of the puzzle.

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum TileType { TileDoesNotExit, TileWall, TileOpen } TileType;
typedef enum Direction { Right, Down, Left, Up } Direction;

typedef struct Tile Tile;

typedef struct NextDirection {
  Direction moveRight;
  Direction moveLeft;
  Direction moveDown;
  Direction moveUp;
} NextDirection;

typedef struct EdgeCoordinate EdgeCoordinate;

typedef struct EdgeCoordinate {
  bool exists;
  Direction dir;
  union {
    int pos[3];
    struct {
      int x, y, z;
    };
  };
  Tile *t;
  EdgeCoordinate *next;
  EdgeCoordinate *prev;
  EdgeCoordinate *connectingCubeEdge;
} EdgeCoordinate;

typedef struct Tile {
  size_t x;
  size_t y;
  TileType type;
  bool isFirst;
  Tile *Left, *Right, *Down, *Up;
  NextDirection nextdir;
  ssize_t edgenum;
  bool isInnerCorner;
  bool isOuterCorner;
  bool visited;
  EdgeCoordinate edgecoords[4];
  bool isEdge;
} Tile;

typedef struct Grid {
  size_t width, height;
  Tile *first;
  Tile *curpos;
  Direction dir;
  size_t squareSize;
  Tile arr[];
} Grid;

#define TILEGET(gridptr, x, y) (&(gridptr)->arr[(y) * (gridptr)->width + (x)])

Grid *initGrid(size_t width, size_t height) {
  Grid *g = calloc(sizeof(Grid) + sizeof(Tile) * width * height, 1);
  g->width = width;
  g->height = height;
  g->squareSize = width / 4 > height / 4 ? width / 4 : height / 4;
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      Tile *t = TILEGET(g, x, y);
      t->x = x;
      t->y = y;
      t->type = TileDoesNotExit;
      t->edgenum = -1;

      memset(&t->edgecoords, 0x0, sizeof(EdgeCoordinate));
    }
  }
  g->dir = Right;
  return g;
}

void addRow(Grid *g, char *line, size_t y) {
  for (size_t x = 0; x < g->width; x++) {
    switch (line[x]) {
    case ' ': {
      TILEGET(g, x, y)->type = TileDoesNotExit;
      break;
    }
    case '.': {
      TILEGET(g, x, y)->type = TileOpen;
      break;
    }
    case '#': {
      TILEGET(g, x, y)->type = TileWall;
      break;
    }
    }
  }
}

void getMaxDim(char *filename, size_t *width, size_t *height) {
  FILE *fh = fopen(filename, "rb");
  *width = 0;
  *height = 0;
  for (;;) {
    char line[8192] = {0};
    if (fscanf(fh, "%[^\r\n]", line) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    if (line[0] >= '0' && line[0] <= '9') {
      break;
    }

    size_t i;
    for (i = strlen(line) - 1; i > 0; i--) {
      if (line[i] != ' ') {
        break;
      }
    }
    if (i > *width) {
      *width = i + 1;
    }
    (*height)++;
  }
  fclose(fh);
}

void printDirection(Direction dir) {
  switch (dir) {
  case Right: {
    printf(">");
    break;
  }
  case Left: {
    printf("<");
    break;
  }
  case Down: {
    printf("v");
    break;
  }
  case Up: {
    printf("^");
    break;
  }
  }
}

void printGrid(Grid *g) {
  printf("################\n");
  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = TILEGET(g, x, y);
      if (t == g->curpos) {
        printDirection(g->dir);
      } else if (t->edgenum != -1) {
        printf("%zd", t->edgenum);
      } else if (t->type == TileOpen) {
        printf(".");
      } else if (t->type == TileWall) {
        printf("#");
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }
}

void markCorners(Grid *g) {
  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = TILEGET(g, x, y);
      if (t->type == TileDoesNotExit) {
        continue;
      }
      if (t->x == 0 || t->x == g->width - 1 ||
          TILEGET(g, t->x + 1, t->y)->type == TileDoesNotExit ||
          TILEGET(g, t->x - 1, t->y)->type == TileDoesNotExit) {
        if (t->y == 0 || t->y == g->height - 1 ||
            TILEGET(g, t->x, t->y + 1)->type == TileDoesNotExit ||
            TILEGET(g, t->x, t->y - 1)->type == TileDoesNotExit) {
          TILEGET(g, t->x, t->y)->isOuterCorner = true;
        }
      }

      // inside corners:
      if (t->x != 0 && t->x + 1 < g->width && t->y != 0 &&
          t->y + 1 < g->height &&
          TILEGET(g, t->x + 1, t->y)->type != TileDoesNotExit &&
          TILEGET(g, t->x - 1, t->y)->type != TileDoesNotExit &&
          TILEGET(g, t->x, t->y + 1)->type != TileDoesNotExit &&
          TILEGET(g, t->x, t->y - 1)->type != TileDoesNotExit) {
        if (TILEGET(g, t->x - 1, t->y - 1)->type == TileDoesNotExit ||
            TILEGET(g, t->x - 1, t->y + 1)->type == TileDoesNotExit ||
            TILEGET(g, t->x + 1, t->y + 1)->type == TileDoesNotExit ||
            TILEGET(g, t->x + 1, t->y - 1)->type == TileDoesNotExit) {
          t->isInnerCorner = true;
        }
      }

      if (t->x == 0 || t->x == g->width - 1 || t->y == 0 ||
          t->y == g->height - 1) {
        t->isEdge = true;
      } else {
        if (TILEGET(g, t->x + 1, t->y)->type == TileDoesNotExit ||
            TILEGET(g, t->x - 1, t->y)->type == TileDoesNotExit ||
            TILEGET(g, t->x, t->y + 1)->type == TileDoesNotExit ||
            TILEGET(g, t->x, t->y - 1)->type == TileDoesNotExit) {
          t->isEdge = true;
        }
      }
    }
  }
}

void getRoot(Grid *g) {
  for (size_t i = 0;; i++) {
    if (g->arr[i].type != TileDoesNotExit) {
      size_t y = i % g->width;
      size_t x = i - g->width * y;
      g->first = TILEGET(g, x, y);
      g->curpos = g->first;
      TILEGET(g, x, y)->isFirst = true;
      break;
    }
  }
}

Tile *getNextTile(Grid *g, Tile *t) {
  // right
  if (t->x < g->width - 1) {
    Tile *tmp = TILEGET(g, t->x + 1, t->y);
    if (tmp->isEdge || tmp->isInnerCorner) {
      if (!tmp->visited) {
        tmp->visited = true;
        return tmp;
      }
    }
  }
  // up:
  if (t->y > 0) {
    Tile *tmp = TILEGET(g, t->x, t->y - 1);
    if (tmp->isEdge || tmp->isInnerCorner) {
      if (!tmp->visited) {
        tmp->visited = true;
        return tmp;
      }
    }
  }
  // down:
  if (t->y < g->height - 1) {
    Tile *tmp = TILEGET(g, t->x, t->y + 1);
    if (tmp->isEdge || tmp->isInnerCorner) {
      if (!tmp->visited) {
        tmp->visited = true;
        return tmp;
      }
    }
  }
  // left:
  if (t->x > 0) {
    Tile *tmp = TILEGET(g, t->x - 1, t->y);
    if (tmp->isEdge || tmp->isInnerCorner) {
      if (!tmp->visited) {
        tmp->visited = true;
        return tmp;
      }
    }
  }
  return g->first;
}

Tile *getTile(Grid *g, Tile *t, Direction dir) {
  size_t x = t->x;
  ssize_t y = t->y;
  switch (dir) {
  case Up: {
    y--;
    break;
  }
  case Down: {
    y++;
    break;
  }
  case Right: {
    x++;
    break;
  }
  case Left: {
    x--;
    break;
  }
  default:
    assert(false);
  }

  if (y < 0 || x < 0 || y >= g->height || x >= g->width) {
    return NULL;
  }

  Tile *adj = TILEGET(g, x, y);
  if (!adj || adj->type == TileDoesNotExit) {
    return NULL;
  }
  return adj;
}

void traverseEdges(Grid *g) {
  int incr[3] = {1, 0, 0};
  int outerCornerIncr[3] = {0, 1, 0};
  int foldOverIncr[3] = {0, 0, 1};
  int coordinates[3] = {0, 0, 0};

  // we know from the problem specification that the root node is always going
  // to have its top edge free:
  Tile *t = g->first;
  t->visited = true;
  t->edgecoords[Up].exists = true;
  t->edgecoords[Up].dir = Up;
  t->edgecoords[Up].x = 0;
  t->edgecoords[Up].y = 0;
  t->edgecoords[Up].z = 0;
  t->edgecoords[Up].t = t;

  Tile *previousTile = t;

  size_t incrSinceLastCorner = 0;
  bool exitedCorner = false;
  for (t = getNextTile(g, t);; t = getNextTile(g, t)) {
    incrSinceLastCorner++;
    if (t->isOuterCorner) {
      coordinates[0] += incr[0];
      coordinates[1] += incr[1];
      coordinates[2] += incr[2];

      incrSinceLastCorner = 0;
      int tmp[3];
      memcpy(tmp, incr, sizeof(int) * 3);
      memcpy(incr, outerCornerIncr, sizeof(int) * 3);
      for (size_t i = 0; i < 3; i++) {
        tmp[i] = tmp[i] * -1;
      }
      memcpy(outerCornerIncr, tmp, sizeof(int) * 3);

      incrSinceLastCorner = 0;
    } else if (t->isInnerCorner) {
      incrSinceLastCorner = 0;
      for (size_t i = 0; i < 3; i++) {
        incr[i] = incr[i] * -1;
      }
      int tmp[3];
      memcpy(tmp, outerCornerIncr, sizeof(int) * 3);
      memcpy(outerCornerIncr, foldOverIncr, sizeof(int) * 3);
      memcpy(foldOverIncr, tmp, sizeof(int) * 3);
      exitedCorner = true;
    } else if (incrSinceLastCorner % g->squareSize == 0) {
      int tmp[3];
      memcpy(tmp, incr, sizeof(int) * 3);
      memcpy(incr, foldOverIncr, sizeof(int) * 3);
      for (size_t i = 0; i < 3; i++) {
        tmp[i] = tmp[i] * -1;
      }
      memcpy(foldOverIncr, tmp, sizeof(int) * 3);
    } else {
      if (!exitedCorner) {
        coordinates[0] += incr[0];
        coordinates[1] += incr[1];
        coordinates[2] += incr[2];
      } else {
        incrSinceLastCorner = 0;
      }
      exitedCorner = false;
    }

    bool previousEdgePointerSet = false;
    size_t numEdges = 0;
    for (size_t dir = 0; dir < 4; dir++) {
      if (!getTile(g, t, dir)) {
        t->edgecoords[dir].exists = true;
        t->edgecoords[dir].dir = dir;
        t->edgecoords[dir].t = t;
        numEdges++;
        memcpy(t->edgecoords[dir].pos, coordinates, sizeof(coordinates));
        g->dir = dir;

        if (previousTile->edgecoords[dir].exists) {
          previousTile->edgecoords[dir].next = &t->edgecoords[dir];
          t->edgecoords[dir].prev = &previousTile->edgecoords[dir];
          previousEdgePointerSet = true;
        }
      }
    }

    if (numEdges == 2) {
      for (size_t dir = 0; dir < 4; dir++) {
        if (t->edgecoords[dir].exists &&
            !previousTile->edgecoords[dir].exists) {
          for (size_t k = 0; k < 4; k++) {
            if (k == dir) {
              continue;
            }
            if (t->edgecoords[k].exists) {
              t->edgecoords[k].next = &t->edgecoords[dir];
              t->edgecoords[dir].prev = &t->edgecoords[k];
              previousEdgePointerSet = true;
              break;
            }
          }
        }
      }
    }

    if (!previousEdgePointerSet) {
      for (size_t dir = 0; dir < 4; dir++) {
        if (t->edgecoords[dir].exists) {
          for (size_t prevdir = 0; prevdir < 4; prevdir++) {
            if (previousTile->edgecoords[prevdir].exists) {
              t->edgecoords[dir].prev = &previousTile->edgecoords[prevdir];
              previousTile->edgecoords[prevdir].next = &t->edgecoords[dir];
              previousEdgePointerSet = true;
              break;
            }
          }
        }
      }
    }

    if (t->isEdge) {
      previousTile = t;
    }

    if (t == g->first) {
      break;
    }
  }
}

void connectEdges(Grid *g) {
  for (EdgeCoordinate *e = &g->first->edgecoords[Up];;) {
    assert(e->next);
    assert(e->t);

    if (!e->connectingCubeEdge) {
      bool forwardIteration = true;
      if (memcmp(e->next->pos, e->pos, sizeof(int) * 3) == 0) {
        forwardIteration = false;
      }
      for (EdgeCoordinate *adj = e;;) {
        adj = forwardIteration ? adj->next : adj->prev;
        if (memcmp(e->pos, adj->pos, sizeof(int) * 3) == 0) {
          e->connectingCubeEdge = adj;
          break;
        }
      }
      assert(e->connectingCubeEdge);
    }

    e = e->next;
    if (e == &g->first->edgecoords[Up]) {
      break;
    }
  }
}

void fixupGrid(Grid *g) {
  getRoot(g);
  markCorners(g);
  traverseEdges(g);
  connectEdges(g);
}

typedef struct Instruction {
  size_t dist;
  char turn;
  bool isTurn;
} Instruction;

typedef struct Instructions {
  size_t size;
  size_t capacity;
  Instruction arr[];
} Instructions;

Instructions *initInstructions() {
  Instructions *d =
      calloc(sizeof(Instructions) + sizeof(Instruction) * 8192, 1);
  d->capacity = 8192;
  return d;
}

void addInstructions(Instructions *d, char *raw) {
  size_t len = strlen(raw);
  for (size_t i = 0; i < len; i++) {
    char instr[1024] = {0};
    if (sscanf(raw, "%[0-9]", instr) == 1) {
      raw += strlen(instr);
      size_t dist = 0;
      sscanf(instr, "%zd", &dist);
      d->arr[d->size].dist = dist;
      d->arr[d->size].isTurn = false;
    } else if (sscanf(raw, "%[LR]", instr) == 1) {
      raw += strlen(instr);
      d->arr[d->size].turn = instr[0];
      d->arr[d->size].isTurn = true;
    } else {
      break;
    }
    d->size++;
  }
}

Direction getOppositeDirection(Direction d) {
  switch (d) {
  case Right: {
    return Left;
  }
  case Left: {
    return Right;
  }
  case Up: {
    return Down;
  }
  case Down: {
    return Up;
  }
  }
  assert(false);
}

void move(Grid *g) {
  Tile *t = getTile(g, g->curpos, g->dir);
  if (t) {
    if (t->type != TileWall) {
      g->curpos = t;
    }
  } else {
    assert(g->curpos->isEdge);
    EdgeCoordinate *e = g->curpos->edgecoords[g->dir].connectingCubeEdge;
    assert(e);
    assert(e->exists);
    assert(e->connectingCubeEdge);
    if (e->t->type != TileWall) {
      g->curpos = e->t;
      g->dir = getOppositeDirection(e->dir);
    }
  }
}

void run(Grid *g, Instructions *instr, bool manual) {
  if (manual) {
    printGrid(g);
  }

  g->dir = Right;
  for (size_t i = 0; i < instr->size; i++) {
    if (manual) {
      i = 0;
    }
    char cmd[10] = {0};
    if (manual) {
      scanf("%s", cmd);
      if (cmd[0] == 'l' || cmd[0] == 'r') {
        instr->arr[i].isTurn = true;
        cmd[0] ^= 0x20;
        instr->arr[i].turn = cmd[0];
      } else {
        instr->arr[i].isTurn = false;
        sscanf(cmd, "%zd", &instr->arr[i].dist);
      }
    }
    if (instr->arr[i].isTurn) {
      if (manual) {
        printf("TURN: %c\n", instr->arr[i].turn);
      }
      switch (instr->arr[i].turn) {
      case 'l':
      case 'L': {
        g->dir = (g->dir + 3) % 4;
        continue;
        break;
      }
      case 'r':
      case 'R': {
        g->dir = (g->dir + 1) % 4;
        continue;
        break;
      }
      default:
        break;
      }
    } else {
      if (manual) {
        printf("MOVE: %zd\n", instr->arr[i].dist);
      }
      for (size_t j = 0; j < instr->arr[i].dist; j++) {
        move(g);
      }
    }
    if (manual) {
      printGrid(g);
    }
  }
}

void part1Score(Grid *g) {
  int64_t score = 1000 * (g->curpos->y + 1) + 4 * (g->curpos->x + 1);
  switch (g->dir) {
  case Right: {
    break;
  }
  case Left: {
    score += 2;
    break;
  }
  case Down: {
    score++;
    break;
  }
  case Up: {
    score += 3;
    break;
  }
  }
  printf("[part1]: %zd\n", score);
}

int main(int argc, char **argv) {
  size_t width, height;
  getMaxDim(argv[1], &width, &height);
  Grid *g = initGrid(width, height);
  Instructions *inst = initInstructions();

  FILE *fh = fopen(argv[1], "rb");
  bool isDirections = false;
  size_t rownum = 0;
  for (;;) {
    char line[8192] = {0};
    if (fscanf(fh, "%[^\r\n]", line) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");

    if (line[0] >= '0' && line[0] <= '9') {
      isDirections = true;
    }

    if (!isDirections) {
      addRow(g, line, rownum);
    } else {
      addInstructions(inst, line);
    }
    rownum++;
  }
  fclose(fh);

  fixupGrid(g);
  run(g, inst, false);
  part1Score(g);

  free(g);
  free(inst);
  return 0;
}