#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum TileType { TileDoesNotExit, TileWall, TileOpen } TileType;
typedef enum Direction { Right, Down, Left, Up } Direction;

typedef struct Tile Tile;

typedef struct Tile {
  size_t x;
  size_t y;
  TileType type;
  bool isFirst;
  Tile *Left, *Right, *Down, *Up;
  char defaultDirection;
  bool defaultDirectionExists;
} Tile;

typedef struct Grid {
  size_t width, height;
  Tile *first;
  Tile *curpos;
  Direction dir;
  Tile arr[];
} Grid;

#define TILEGET(gridptr, x, y) (&(gridptr)->arr[(y) * (gridptr)->width + (x)])

Grid *initGrid(size_t width, size_t height) {
  Grid *g = calloc(sizeof(Grid) + sizeof(Tile) * width * height, 1);
  g->width = width;
  g->height = height;
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      TILEGET(g, x, y)->x = x;
      TILEGET(g, x, y)->y = y;
      TILEGET(g, x, y)->type = TileDoesNotExit;
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

void printDirection(char dir) {
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

void printGrid(Grid *g, bool showDefaultDirection) {
  printf("################\n");
  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = TILEGET(g, x, y);
      if (t == g->curpos && !showDefaultDirection) {
        printDirection(g->dir);
      } else if (showDefaultDirection && t->defaultDirectionExists) {
        printDirection(t->defaultDirection);
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

void fixupGrid(Grid *g) {
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

  // do each column, first:
  for (size_t x = 0; x < g->width; x++) {
    size_t firstcoord = 0;
    bool found = false;
    for (firstcoord = 0; firstcoord < g->height; firstcoord++) {
      if (TILEGET(g, x, firstcoord)->type != TileDoesNotExit) {
        found = true;
        break;
      }
    }
    if (!found) {
      continue;
    }
    Tile *first = TILEGET(g, x, firstcoord);
    Tile *curr = first;
    Tile *prev = NULL;
    for (curr = first; curr->type != TileDoesNotExit;
         curr = TILEGET(g, x, curr->y + 1)) {
      if (prev != NULL && prev->type != TileWall && curr->type != TileWall) {
        prev->Down = curr;
        curr->Up = prev;
      }
      prev = curr;
      if (curr->y + 1 == g->height) {
        break;
      }
    }
    if (first->type != TileWall && prev->type != TileWall &&
        curr->type != TileWall) {
      first->Up = prev;
      prev->Down = first;
    }
  }

  // do each row
  for (size_t y = 0; y < g->height; y++) {
    size_t firstcoord = 0;
    bool found = false;
    for (firstcoord = 0; firstcoord < g->width; firstcoord++) {
      if (TILEGET(g, firstcoord, y)->type != TileDoesNotExit) {
        found = true;
        break;
      }
    }
    if (!found) {
      continue;
    }
    Tile *first = TILEGET(g, firstcoord, y);
    Tile *curr = first;
    Tile *prev = NULL;
    for (curr = first; curr->type != TileDoesNotExit;
         curr = TILEGET(g, curr->x + 1, y)) {
      if (prev != NULL && prev->type != TileWall && curr->type != TileWall) {
        prev->Right = curr;
        curr->Left = prev;
      }
      prev = curr;
      if (curr->x + 1 == g->width) {
        break;
      }
    }
    if (first->type != TileWall && prev->type != TileWall) {
      first->Left = prev;
      prev->Right = first;
    }
  }
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

void move(Grid *g, size_t dist) {
  for (size_t j = 0; j < dist; j++) {
    switch (g->dir) {
    case Right: {
      if (g->curpos->Right) {
        g->curpos = g->curpos->Right;
      } else {
        return;
      }
      break;
    }
    case Left: {
      if (g->curpos->Left) {
        g->curpos = g->curpos->Left;
      } else {
        return;
      }
      break;
    }
    case Down: {
      if (g->curpos->Down) {
        g->curpos = g->curpos->Down;
      } else {
        return;
      }
      break;
    }
    case Up: {
      if (g->curpos->Up) {
        g->curpos = g->curpos->Up;
      } else {
        return;
      }
      break;
    }
    }
  }
}

void run(Grid *g, Instructions *instr) {
  for (size_t i = 0; i < instr->size; i++) {
    // printGrid(g);
    //  getchar();
    if (instr->arr[i].isTurn) {
      // printf("TURN: %c\n", instr->arr[i].turn);
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
      // printf("MOVE: %zd\n", instr->arr[i].dist);
      move(g, instr->arr[i].dist);
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
    score += 2;
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
  run(g, inst);
  part1Score(g);

  free(g);
  free(inst);
  return 0;
}