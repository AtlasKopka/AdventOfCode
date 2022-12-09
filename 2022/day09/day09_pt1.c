#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPWIDTH 30
#define MAPHEIGHT 30
#define DEBUG

typedef struct square {
  bool visitedByTail;
  bool ishead;
  bool istail;
  size_t pos[2];
} square;

typedef struct map {
  size_t width;
  size_t height;
  size_t headpos[2];
  size_t tailpos[2];
  square squares[];
} map;

square *getSquare(map *mymap, size_t x, size_t y) {
  return &(mymap->squares[y * mymap->width + x]);
}

square *getHead(map *mymap) {
  return getSquare(mymap, mymap->headpos[0], mymap->headpos[1]);
}

square *getTail(map *mymap) {
  return getSquare(mymap, mymap->tailpos[0], mymap->tailpos[1]);
}

map *initMap() {
  map *mymap = calloc(sizeof(map) + sizeof(square) * MAPHEIGHT * MAPWIDTH, 1);
  mymap->height = MAPHEIGHT;
  mymap->width = MAPWIDTH;
  mymap->headpos[0] = mymap->width / 2;
  mymap->headpos[1] = mymap->height / 2;
  memcpy(mymap->tailpos, mymap->headpos, sizeof(mymap->headpos));

  square *start = getHead(mymap);
  start->ishead = start->istail = start->visitedByTail = true;

  for (size_t i = 0; i < mymap->width; i++) {
    for (size_t j = 0; j < mymap->height; j++) {
      square *mysquare = getSquare(mymap, i, j);
      mysquare->pos[0] = i;
      mysquare->pos[1] = j;
    }
  }

  return mymap;
}

#define FREEMAP(_mapptr)                                                       \
  do {                                                                         \
    free(_mapptr);                                                             \
    _mapptr = NULL;                                                            \
  } while (0);

void printMap(map *mymap) {
#ifdef DEBUG
  for (size_t i = 0; i < mymap->height; i++) {
    for (size_t j = 0; j < mymap->width; j++) {
      square *mysquare = getSquare(mymap, j, i);
      if (mysquare->ishead) {
        printf("H");
      } else if (mysquare->istail) {
        printf("T");
      } else if (mysquare->visitedByTail) {
        printf("#");
      } else {
        printf(".");
      }
      printf(" ");
    }
    printf("\n");
  }
#endif
}

void moveHead(map *mymap, char direction, size_t distance) {
  square *head = getHead(mymap);
  size_t newHeadPos[2] = {head->pos[0], head->pos[1]};
  switch (direction) {
  case 'R': {
    newHeadPos[0] += distance;
    break;
  }
  case 'L': {
    newHeadPos[0] -= distance;
    break;
  }
  case 'U': {
    newHeadPos[1] -= distance;
    break;
  }
  case 'D': {
    newHeadPos[1] += distance;
    break;
  }
  }
  head->ishead = false;
  head = getSquare(mymap, newHeadPos[0], newHeadPos[1]);
  head->ishead = true;
  memcpy(mymap->headpos, newHeadPos, sizeof(newHeadPos));
}

bool isDiagonal(square *a, square *b) {
  // diagonal:
  if (llabs((ssize_t)a->pos[1] - (ssize_t)b->pos[1]) <= 1) {
    if (llabs((ssize_t)a->pos[0] - (ssize_t)b->pos[0]) <= 1) {
      return true;
    }
  }
  return false;
}

bool areSquaresTouching(square *a, square *b) {
  if (a->pos[0] == b->pos[0]) {
    if (llabs((ssize_t)a->pos[1] - (ssize_t)b->pos[1]) <= 1) {
      return true;
    }
  }

  if (a->pos[1] == b->pos[1]) {
    if (llabs((ssize_t)a->pos[0] - (ssize_t)b->pos[0]) <= 1) {
      return true;
    }
  }

  return isDiagonal(a, b);
}

void updateTail(map *mymap) {
  square *head = getHead(mymap);
  square *tail = getTail(mymap);
  if (areSquaresTouching(head, tail)) {
#ifdef DEBUG
    printf("SQUARES TOUCHING. Not updating tail.\n");
#endif
    return;
  }

  size_t newTailPos[2] = {tail->pos[0], tail->pos[1]};
  if (head->pos[1] == tail->pos[1]) {
    head->pos[0] > tail->pos[0] ? newTailPos[0]++ : newTailPos[0]--;
  } else if (head->pos[0] == tail->pos[0]) {
    head->pos[1] > tail->pos[1] ? newTailPos[1]++ : newTailPos[1]--;
  } else { // diagonal:
    if (head->pos[1] > tail->pos[1]) {
      newTailPos[1]++;
    } else {
      newTailPos[1]--;
    }
    if (head->pos[0] > tail->pos[0]) {
      newTailPos[0]++;
    } else {
      newTailPos[0]--;
    }
  }

  tail->istail = false;
  tail = getSquare(mymap, newTailPos[0], newTailPos[1]);
  tail->istail = true;
  tail->visitedByTail = true;
  memcpy(mymap->tailpos, tail->pos, sizeof(tail->pos));
}

void doMove(map *mymap, char direction, size_t distance) {
  for (size_t i = 0; i < distance; i++) {
#ifdef DEBUG
    printf("\n[%c %zd] step %zd\n", direction, distance, i + 1);
#endif
    moveHead(mymap, direction, 1);
    updateTail(mymap);
    printMap(mymap);
  }
}

size_t numSquaresVisistedByTail(map *mymap) {
  size_t numSquares = 0;
  for (size_t x = 0; x < mymap->width; x++) {
    for (size_t y = 0; y < mymap->height; y++) {
      square *s = getSquare(mymap, x, y);
      if (s->visitedByTail) {
        numSquares++;
      }
    }
  }
  return numSquares;
}

int main(int argc, char **argv) {
  map *mymap = initMap();
  FILE *fh = fopen(argv[1], "rb");
  assert(fh);

  printMap(mymap);

  for (;;) {
    char direction[10];
    size_t distance;
    if (EOF == fscanf(fh, "%[RULD] %zd", direction, &distance)) {
      break;
    }
    fscanf(fh, "%*[\r\n ]");
    doMove(mymap, direction[0], distance);
  }

  printf("[part1]: %zd\n", numSquaresVisistedByTail(mymap));

  fclose(fh);
  FREEMAP(mymap);
}