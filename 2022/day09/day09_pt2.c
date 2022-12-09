#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPWIDTH 1024
#define MAPHEIGHT 1024
#define NUMKNOTS 10

typedef struct knot {
  size_t id;
  size_t pos[2];
} knot;

typedef struct square {
  bool visitedByTail;
  size_t pos[2];
  knot *knotptrs[NUMKNOTS];
} square;

typedef struct map {
  size_t width;
  size_t height;
  knot knots[NUMKNOTS];
  square squares[];
} map;

square *getSquare(map *mymap, size_t *pos) {
  return &(mymap->squares[pos[1] * mymap->width + pos[0]]);
}

map *initMap() {
  map *mymap = calloc(sizeof(map) + sizeof(square) * MAPHEIGHT * MAPWIDTH, 1);
  mymap->height = MAPHEIGHT;
  mymap->width = MAPWIDTH;
  for (size_t i = 0; i < NUMKNOTS; i++) {
    mymap->knots[i].id = i;
    mymap->knots[i].pos[0] = mymap->width / 2;
    mymap->knots[i].pos[1] = mymap->height / 2;
  }

  square *mysquare = getSquare(mymap, mymap->knots[0].pos);
  mysquare->visitedByTail = true;
  for (size_t i = 0; i < NUMKNOTS; i++) {
    mysquare->knotptrs[i] = &mymap->knots[i];
  }

  for (size_t i = 0; i < mymap->width; i++) {
    for (size_t j = 0; j < mymap->height; j++) {
      size_t pos[2] = {i, j};
      square *mysquare = getSquare(mymap, pos);
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
      size_t pos[2];
      pos[0] = j;
      pos[1] = i;
      square *mysquare = getSquare(mymap, pos);
      bool knotfound = false;
      for (size_t k = 0; k < NUMKNOTS; k++) {
        if (mysquare->knotptrs[k]) {
          printf("%zd", mysquare->knotptrs[k]->id);
          knotfound = true;
          break;
        }
      }
      if (!knotfound) {
        if (mysquare->visitedByTail) {
          printf("#");
        } else {
          printf(".");
        }
      }
      printf(" ");
    }
    printf("\n");
  }
#endif
}

void moveHead(map *mymap, char direction, size_t distance) {
  knot *head = &mymap->knots[0];
  size_t newHeadPos[2] = {head->pos[0], head->pos[1]};
  square *mysquare = getSquare(mymap, newHeadPos);
  mysquare->knotptrs[0] = NULL;
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
  mysquare = getSquare(mymap, newHeadPos);
  mysquare->knotptrs[0] = head;
  memcpy(head->pos, newHeadPos, sizeof(newHeadPos));
}

bool isDiagonal(knot *a, knot *b) {
  // diagonal:
  if (llabs((ssize_t)a->pos[1] - (ssize_t)b->pos[1]) <= 1) {
    if (llabs((ssize_t)a->pos[0] - (ssize_t)b->pos[0]) <= 1) {
      return true;
    }
  }
  return false;
}

bool areKnotsTouching(knot *a, knot *b) {
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

bool updateKnot(map *mymap, size_t knotnum) {
  knot *head = &mymap->knots[knotnum - 1];
  knot *tail = &mymap->knots[knotnum];
  if (areKnotsTouching(head, tail)) {
    return true;
  }

  square *mysquare = getSquare(mymap, tail->pos);
  mysquare->knotptrs[knotnum] = NULL;

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

  mysquare = getSquare(mymap, newTailPos);
  mysquare->knotptrs[knotnum] = tail;
  memcpy(tail->pos, newTailPos, sizeof(newTailPos));
  if (knotnum == NUMKNOTS - 1) {
    mysquare->visitedByTail = true;
  }
  return false;
}

void doMove(map *mymap, char direction, size_t distance) {
  for (size_t i = 0; i < distance; i++) {
#ifdef DEBUG
    printf("\n[%c %zd] step %zd\n", direction, distance, i + 1);
#endif
    for (size_t knotnum = 0; knotnum < NUMKNOTS; knotnum++) {
      if (knotnum == 0) {
        moveHead(mymap, direction, 1);
      } else {
        if (updateKnot(mymap, knotnum)) {
          break;
        }
      }
      printMap(mymap);
    }
  }
}

size_t numSquaresVisistedByTail(map *mymap) {
  size_t numSquares = 0;
  for (size_t x = 0; x < mymap->width; x++) {
    for (size_t y = 0; y < mymap->height; y++) {
      size_t pos[] = {x, y};
      square *s = getSquare(mymap, pos);
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

  printf("[part2]: %zd\n", numSquaresVisistedByTail(mymap));

  fclose(fh);
  FREEMAP(mymap);
}