#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct point {
  int64_t x;
  int64_t y;
  int64_t dx;
  int64_t dy;
} point;

typedef struct points {
  int64_t timestamp;
  size_t numPoints;
  int64_t minmax[4];
  point arr[];
} points;

void printPoints(points *p) {
  for (int64_t y = p->minmax[1]; y <= p->minmax[3]; y++) {
    for (int64_t x = p->minmax[0]; x <= p->minmax[2]; x++) {
      bool pointFound = false;
      for (size_t i = 0; i < p->numPoints; i++) {
        if (p->arr[i].x == x && p->arr[i].y == y) {
          printf("#");
          pointFound = true;
          break;
        }
      }
      if (!pointFound) {
        printf(".");
      }
    }
    printf("\n");
  }
}

void runSimulation(points *p) {
  points *prevState = calloc(sizeof(points) + p->numPoints * sizeof(point), 1);
  int64_t prevwidth = INT64_MAX, prevheight = INT64_MAX;
  printf("Simulating %zd points...\n", p->numPoints);
  for (size_t iter = 0;; iter++) {
    memcpy(prevState, p, sizeof(points) + p->numPoints * sizeof(point));
    memcpy(p->minmax, (int64_t[]){INT64_MAX, INT64_MAX, INT64_MIN, INT64_MIN},
           sizeof(int64_t) * 4);
    for (size_t i = 0; i < p->numPoints; i++) {
      p->arr[i].x += p->arr[i].dx;
      p->arr[i].y += p->arr[i].dy;
      if (p->arr[i].x < p->minmax[0]) {
        p->minmax[0] = p->arr[i].x;
      }
      if (p->arr[i].x > p->minmax[2]) {
        p->minmax[2] = p->arr[i].x;
      }
      if (p->arr[i].y < p->minmax[1]) {
        p->minmax[1] = p->arr[i].y;
      }
      if (p->arr[i].y > p->minmax[3]) {
        p->minmax[3] = p->arr[i].y;
      }
    }
    p->timestamp++;
    int64_t width = labs(p->minmax[0] - p->minmax[2]);
    int64_t height = labs(p->minmax[1] - p->minmax[3]);
    if (width > prevwidth && height > prevheight) {
      printPoints(prevState);
      printf("[part2]: %ld seconds of real-time\n", prevState->timestamp);
      break;
    }
    prevwidth = width;
    prevheight = height;
  }
  free(prevState);
}

int main(int argc, char **argv) {
  points *p = calloc(sizeof(points) + sizeof(point) * 512, 1);
  FILE *fh = fopen(argv[1], "rb");
  for (;;) {
    int64_t x, y, dx, dy;
    if (fscanf(fh, "position=< %ld, %ld > velocity=< %ld, %ld >", &x, &y, &dx,
               &dy) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    p->arr[p->numPoints] = (point){.x = x, .y = y, .dx = dx, .dy = dy};
    p->numPoints++;
  }
  runSimulation(p);
  fclose(fh);
  free(p);
  return 0;
}