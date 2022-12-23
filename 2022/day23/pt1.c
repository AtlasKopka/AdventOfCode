#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXY 1024
#define MAXX 1024

typedef struct Tile Tile;

typedef struct Tile {
  bool elf;
  size_t x;
  size_t y;
  Tile *N, *NW, *NE, *S, *SW, *SE, *W, *E;
  size_t numElvesThatWantToBeHere;
  Tile *desiredlocation;
} Tile;

typedef struct Grid {
  size_t minx, miny, maxx, maxy;
  size_t width;
  size_t height;
  Tile tiles[];
} Grid;

void fixup(Grid *g) {
  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = &g->tiles[y * g->width + x];
      if (y != 0) {
        t->N = &g->tiles[(y - 1) * g->width + x];
      }
      if (y != g->height - 1) {
        t->S = &g->tiles[(y + 1) * g->width + x];
      }
      if (x != 0 && y != 0) {
        t->NW = &g->tiles[(y - 1) * g->width + x - 1];
      }
      if (x != g->width - 1 && y != 0) {
        t->NE = &g->tiles[(y - 1) * g->width + x + 1];
      }
      if (y != g->height - 1 && x != 0) {
        t->SW = &g->tiles[(y + 1) * g->width + x - 1];
      }
      if (y != g->height - 1 && x != g->width - 1) {
        t->SE = &g->tiles[(y + 1) * g->width + x + 1];
      }
      if (x != 0) {
        t->W = &g->tiles[(y)*g->width + x - 1];
      }
      if (x != g->width - 1) {
        t->E = &g->tiles[(y)*g->width + x + 1];
      }
    }
  }
}

Grid *initGrid() {
  Grid *g = calloc(sizeof(Grid) + sizeof(Tile) * MAXY * MAXX, 1);
  g->width = MAXX;
  g->height = MAXY;

  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = &g->tiles[y * g->width + x];
      t->x = x;
      t->y = y;
    }
  }
  return g;
}

void printGrid(Grid *g) {
  for (size_t y = g->miny; y <= g->maxy; y++) {
    for (size_t x = g->minx; x <= g->maxx; x++) {
      if (g->tiles[y * g->width + x].elf) {
        printf("#");
      } else {
        printf(".");
      }
    }
    printf("\n");
  }
  printf("min: [%zd,%zd]; max: [%zd,%zd]\n", g->minx, g->miny, g->maxx,
         g->maxy);
  printf("__________________________\n");
}

void parseLine(Grid *g, char *line, size_t rownum) {
  if (g->minx == 0) {
    g->minx = g->width / 2 - 50;
    g->miny = g->height / 2 - 50;
    g->maxx = g->minx;
    g->maxy = g->miny;
  }
  for (size_t i = 0; i < strlen(line); i++) {
    size_t x = g->minx + i;
    size_t y = g->miny + rownum;
    Tile *t = &g->tiles[y * g->width + x];
    if (line[i] == '#') {
      t->elf = true;
    }
    if (x > g->maxx) {
      g->maxx = x;
    }
    if (y > g->maxy) {
      g->maxy = y;
    }
  }
}

typedef bool searchfunc(Grid *, Tile *);

typedef struct Instructions {
  searchfunc *searchFuncs[5];
  size_t firstSearchFunc;
  bool init;
} Instructions;

Instructions elfstructions;

bool searchNorth(Grid *g, Tile *t) {
  if (t->N && t->NE && t->NW) {
    if (!t->N->elf && !t->NW->elf && !t->NE->elf) {
      t->N->numElvesThatWantToBeHere++;
      t->desiredlocation = t->N;
      return true;
    }
  }
  return false;
}

bool searchSouth(Grid *g, Tile *t) {
  if (t->S && t->SE && t->SW) {
    if (!t->S->elf && !t->SW->elf && !t->SE->elf) {
      t->S->numElvesThatWantToBeHere++;
      t->desiredlocation = t->S;
      return true;
    }
  }
  return false;
}

bool searchWest(Grid *g, Tile *t) {
  if (t->W && t->SW && t->NW) {
    if (!t->W->elf && !t->NW->elf && !t->SW->elf) {
      t->W->numElvesThatWantToBeHere++;
      t->desiredlocation = t->W;
      return true;
    }
  }
  return false;
}

bool searchEast(Grid *g, Tile *t) {
  if (t->E && t->SE && t->NE) {
    if (!t->E->elf && !t->NE->elf && !t->SE->elf) {
      t->E->numElvesThatWantToBeHere++;
      t->desiredlocation = t->E;
      return true;
    }
  }
  return false;
}

void calcscore(Grid *);

void play(Grid *g) {
  memset(&elfstructions, 0x0, sizeof(elfstructions));
  if (!elfstructions.init) {
    elfstructions.searchFuncs[0] = searchNorth;
    elfstructions.searchFuncs[1] = searchSouth;
    elfstructions.searchFuncs[2] = searchWest;
    elfstructions.searchFuncs[3] = searchEast;
  }

  for (size_t i = 0;; i++) {
    // printf("Round# %zd:\n", i);
    // printGrid(g);
    // calcscore(g);
    // getchar();
    size_t numElvesMoved = 0;
    for (size_t y = g->miny; y <= g->maxy; y++) {
      for (size_t x = g->minx; x <= g->maxx; x++) {
        Tile *t = &g->tiles[y * g->width + x];
        if (!t->elf) {
          continue;
        }

        if (!t->N->elf && !t->NW->elf && !t->NE->elf && !t->E->elf &&
            !t->W->elf && !t->S->elf && !t->SW->elf && !t->SE->elf) {
          continue;
        }

        for (size_t i = 0; i < 4; i++) {
          size_t funcidx = (i + elfstructions.firstSearchFunc) % 4;
          if (elfstructions.searchFuncs[funcidx](g, t)) {
            break;
          }
        }
      }
    }
    elfstructions.firstSearchFunc++;
    elfstructions.firstSearchFunc %= 4;

    size_t starty = g->miny;
    size_t endy = g->maxy;
    size_t startx = g->minx;
    size_t endx = g->maxx;
    for (size_t y = starty; y <= endy; y++) {
      for (size_t x = startx; x <= endx; x++) {
        Tile *t = &g->tiles[y * g->width + x];
        if (!t->elf) {
          continue;
        }
        if (!t->desiredlocation ||
            t->desiredlocation->numElvesThatWantToBeHere != 1) {
          continue;
        }
        t->desiredlocation->elf = true;
        t->elf = false;
        numElvesMoved++;
        if (t->desiredlocation->x < g->minx) {
          g->minx = t->desiredlocation->x;
        } else if (t->desiredlocation->x > g->maxx) {
          g->maxx = t->desiredlocation->x;
        }
        if (t->desiredlocation->y < g->miny) {
          g->miny = t->desiredlocation->y;
        } else if (t->desiredlocation->y > g->maxy) {
          g->maxy = t->desiredlocation->y;
        }
      }
    }

    for (size_t y = g->miny; y <= g->maxy; y++) {
      for (size_t x = g->minx; x <= g->maxx; x++) {
        Tile *t = &g->tiles[y * g->width + x];
        t->numElvesThatWantToBeHere = 0;
        t->desiredlocation = NULL;
      }
    }

    if (i == 9) {
      calcscore(g);
    }
    if (!numElvesMoved) {
      printf("[part2]: round # %zd was the first round were no elves moved\n",
             i + 1);
      return;
    }
  }
}

void calcscore(Grid *g) {
  size_t minmax[4] = {g->width, g->height, 0, 0};
  for (size_t y = 0; y < g->height; y++) {
    for (size_t x = 0; x < g->width; x++) {
      Tile *t = &g->tiles[y * g->width + x];
      if (!t->elf) {
        continue;
      }
      if (x < minmax[0]) {
        minmax[0] = x;
      }
      if (y < minmax[1]) {
        minmax[1] = y;
      }
      if (x > minmax[2]) {
        minmax[2] = x;
      }
      if (y > minmax[3]) {
        minmax[3] = y;
      }
    }
  }
  printf("Min: [%zd,%zd] max: [%zd,%zd]\n", minmax[0], minmax[1], minmax[2],
         minmax[3]);

  size_t numEmptySquares = 0;
  for (size_t y = minmax[1]; y <= minmax[3]; y++) {
    for (size_t x = minmax[0]; x <= minmax[2]; x++) {
      Tile *t = &g->tiles[y * g->width + x];
      if (!t->elf) {
        numEmptySquares++;
      }
    }
  }
  printf("[part1]: %zd\n", numEmptySquares);
}

int main(int argc, char **argv) {
  Grid *g = initGrid();
  FILE *fh = fopen(argv[1], "rb");
  size_t rownum = 0;
  printf("reading...\n");
  for (;;) {
    char line[1024];
    if (EOF == fscanf(fh, "%[^\r\n]", line)) {
      break;
    }
    parseLine(g, line, rownum++);
    fscanf(fh, "%*[\r\n]");
  }
  fclose(fh);
  printf("Fixing up grid...\n");
  fixup(g);
  printf("Playing...\n");
  play(g);
  free(g);
  return 0;
}