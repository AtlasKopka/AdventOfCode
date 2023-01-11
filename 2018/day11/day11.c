#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY 1723

int main(int argc, char **argv) {
  int32_t arr[301 * 301] = {0};
  for (size_t y = 1; y < 301; y++) {
    for (size_t x = 1; x < 301; x++) {
      int32_t r = (x - 1) + 10;
      int32_t score = (((((r * (y - 1)) + KEY) * r) / 100) % 10) - 5;
      arr[y * 301 + x] = score + arr[y * 301 + x - 1] + arr[(y - 1) * 301 + x] -
                         arr[(y - 1) * 301 + x - 1];
    }
  }

  int32_t maxScore = 0, bestX = 0, bestY = 0;
  for (size_t y = 1; y < 301 - 2; y++) {
    for (size_t x = 1; x < 301 - 2; x++) {
      int32_t score = arr[(y + 2) * 301 + x + 2] - arr[(y + 2) * 301 + x - 1] -
                      arr[(y - 1) * 301 + x + 2] + arr[(y - 1) * 301 + x - 1];
      if (score > maxScore) {
        maxScore = score;
        bestX = x - 1;
        bestY = y - 1;
      }
    }
  }
  printf("[part1]: %d,%d (score: %d)\n", bestX, bestY, maxScore);

  maxScore = 0, bestX = 0, bestY = 0;
  int32_t bestGridSize;
  for (size_t y = 1; y < 301; y++) {
    for (size_t x = 1; x < 301; x++) {
      size_t maxGridSize = x > y ? 301 - x : 301 - y;
      for (size_t gridSize = 0; gridSize < maxGridSize; gridSize++) {
        int32_t score = arr[(y + gridSize) * 301 + (x + gridSize)] +
                        arr[(y - 1) * 301 + x - 1] -
                        arr[(y - 1) * 301 + x + gridSize] -
                        arr[(y + gridSize) * 301 + x - 1];
        if (score > maxScore) {
          maxScore = score;
          bestGridSize = gridSize + 1;
          bestX = x - 1;
          bestY = y - 1;
        }
      }
    }
  }
  printf("[part2]: %d,%d,%d [score: %d]\n", bestX, bestY, bestGridSize,
         maxScore);
  return 0;
}