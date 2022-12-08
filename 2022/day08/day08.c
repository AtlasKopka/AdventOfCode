#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

typedef struct {
  size_t height;
  bool exists;
  bool visible[4];
  size_t numTreesVisible[4];
} Tree;

void part1(Tree *map, size_t width, size_t height) {
  for (size_t i = 0; i < 2048; i++) {
    if (!map[i * 2048].exists) {
      break;
    }

    for (size_t j = 0; j < 2048; j++) {
      if (map[i * 2048 + j].exists) {
        printf("%zd", map[i * 2048 + j].height);
        for (size_t left = 0; left <= j; left++) {
          if (left == j) {
            map[i * 2048 + j].visible[LEFT] = true;
            printf("[LEFT] ");
            break;
          }
          if (map[i * 2048 + left].height >= map[i * 2048 + j].height) {
            break;
          }
        }
        for (size_t right = width - 1; width >= j; right--) {
          if (right == j) {
            map[i * 2048 + j].visible[RIGHT] = true;
            printf("[RIGHT ]");
            break;
          }
          if (map[i * 2048 + right].height >= map[i * 2048 + j].height) {
            break;
          }
        }
        for (size_t top = 0; top <= i; top++) {
          if (top == i) {
            map[i * 2048 + j].visible[UP] = true;
            printf("[TOP ] ");
            break;
          }
          if (map[(top * 2048) + j].height >= map[i * 2048 + j].height) {
            break;
          }
        }
        for (size_t bottom = height - 1; bottom >= i; bottom--) {
          if (bottom == i) {
            map[i * 2048 + j].visible[DOWN] = true;
            printf("[BOTTOM] ");
            break;
          }
          if (map[(bottom * 2048) + j].height >= map[i * 2048 + j].height) {
            break;
          }
        }
      } else {
        printf("|\n");
        break;
      }
    }
  }

  // part1
  size_t totalVisible = 0;
  for (size_t i = 0; i < width; i++) {
    for (size_t j = 0; j < height; j++) {
      for (size_t k = 0; k < 4; k++) {
        if (map[i * 2048 + j].visible[k]) {
          totalVisible++;
          break;
        }
      }
    }
  }
  printf("[part1]: %zd\n", totalVisible);
}

void part2(Tree *map, size_t width, size_t height) {
  uint64_t topscore = 0;
  for (size_t i = 0; i < height; i++) {
    if (!map[i * 2048].exists) {
      break;
    }
    for (size_t j = 0; j < width; j++) {
      printf("%zd", map[i * 2048 + j].height);
      for (ssize_t top = i - 1; top >= 0; top--) {
        map[i * 2048 + j].numTreesVisible[UP]++;
        if (map[top * 2048 + j].height >= map[i * 2048 + j].height) {
          break;
        }
      }
      printf("[UP: %zd]", map[i * 2048 + j].numTreesVisible[UP]);
      for (ssize_t bottom = i + 1; bottom < height; bottom++) {
        map[i * 2048 + j].numTreesVisible[DOWN]++;
        if (map[bottom * 2048 + j].height >= map[i * 2048 + j].height) {
          break;
        }
      }
      printf("[DOWN: %zd]", map[i * 2048 + j].numTreesVisible[DOWN]);
      for (ssize_t left = j - 1; left >= 0; left--) {
        map[i * 2048 + j].numTreesVisible[LEFT]++;
        if (map[i * 2048 + left].height >= map[i * 2048 + j].height) {
          break;
        }
      }
      printf("[LEFT: %zd]", map[i * 2048 + j].numTreesVisible[LEFT]);
      for (ssize_t right = j + 1; right < width; right++) {
        map[i * 2048 + j].numTreesVisible[RIGHT]++;
        if (map[i * 2048 + right].height >= map[i * 2048 + j].height) {
          break;
        }
      }
      printf("[RIGHT: %zd]", map[i * 2048 + j].numTreesVisible[RIGHT]);
      size_t *scores = map[i * 2048 + j].numTreesVisible;
      uint64_t score = 1;
      for (size_t i = 0; i < sizeof(map[0].numTreesVisible) /
                                 sizeof(map[0].numTreesVisible[0]);
           i++) {
        score *= scores[i];
      }
      printf("[SCORE: %lu]", score);
      if (score > topscore) {
        topscore = score;
      }
    }
    printf("|\n");
  }
  printf("[part2]: %lu\n", topscore);
}

int main(int argc, char **argv) {
  FILE *fh = fopen(argv[1], "rb");
  assert(fh);

  Tree *map = calloc(sizeof(Tree) * 2048 * 2048, 1);
  assert(map);

  size_t height = 0, width = 0;
  for (size_t i = 0;; i++) {
    char line[2048];
    memset(line, 0x0, sizeof(line));
    if (EOF == fscanf(fh, "%[^\r\n ]", line)) {
      break;
    }
    fscanf(fh, "%*[\r\n ]");

    for (size_t j = 0; j < strlen(line); j++) {
      char height[2];
      memset(height, 0x0, sizeof(height));
      height[0] = line[j];
      sscanf(height, "%zd", &map[i * 2048 + j].height);
      map[i * 2048 + j].exists = true;
      if (j > width) {
        width++;
      }
    }
    if (i > height) {
      height++;
    }
  }
  fclose(fh);

  width++;
  height++;
  // part1(map, width, height);
  part2(map, width, height);

  free(map);
}