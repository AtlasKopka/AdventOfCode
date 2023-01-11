#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY 1723
#define HEIGHT 300
#define WIDTH 300

typedef struct scoreCache {
  int32_t score;
  bool set;
} scoreCache;

typedef struct tile {
  int32_t x, y;

  int32_t score;
  int32_t rowScore;
  int32_t cellScore;
  bool scoreComputed;
  bool rowScoreComputed;
  bool cellScoreComputed;
  scoreCache cache[300];
} tile;

typedef struct grid {
  tile tiles[WIDTH * HEIGHT];
} grid;

typedef struct getScoreStruct {
  int32_t x, y;
  grid *mygrid;
} getScoreStruct;
int32_t getScore(getScoreStruct args) {
  if (!args.mygrid->tiles[args.y * WIDTH + args.x].scoreComputed) {
    int32_t score = (((args.x + 10) * args.y) + KEY) * (args.x + 10);
    score = (score - ((score / 1000) * 1000)) / 100;
    score -= 5;
    args.mygrid->tiles[args.y * WIDTH + args.x].score = score;
    args.mygrid->tiles[args.y * WIDTH + args.x].scoreComputed = true;
  }
  return args.mygrid->tiles[args.y * WIDTH + args.x].score;
}

int32_t getRowScore(getScoreStruct args) {
  if (!args.mygrid->tiles[args.y * WIDTH + args.x].rowScoreComputed) {
    for (size_t i = 0; i < 3; i++) {
      args.mygrid->tiles[args.y * WIDTH + args.x].rowScore += getScore((
          getScoreStruct){.mygrid = args.mygrid, .x = args.x + i, .y = args.y});
    }
    args.mygrid->tiles[args.y * WIDTH + args.x].rowScoreComputed = true;
  }
  return args.mygrid->tiles[args.y * WIDTH + args.x].rowScore;
}

int32_t getCellScore(getScoreStruct args) {
  if (!args.mygrid->tiles[args.y * WIDTH + args.x].cellScoreComputed) {
    for (size_t i = 0; i < 3; i++) {
      args.mygrid->tiles[args.y * WIDTH + args.x].cellScore += getRowScore((
          getScoreStruct){.mygrid = args.mygrid, .x = args.x, .y = args.y + i});
    }
    args.mygrid->tiles[args.y * WIDTH + args.x].cellScoreComputed = true;
  }
  return args.mygrid->tiles[args.y * WIDTH + args.x].cellScore;
}

grid *initGrid() {
  int32_t topScore = 0;
  tile *topTile = NULL;

  grid *g = calloc(sizeof(grid), 1);
  for (size_t y = 0; y < HEIGHT; y++) {
    for (size_t x = 0; x < WIDTH; x++) {
      g->tiles[y * WIDTH + x].x = x;
      g->tiles[y * WIDTH + x].y = y;
      if ((x < WIDTH - 3) && (y < HEIGHT - 3)) {
        int32_t cellScore =
            getCellScore((getScoreStruct){.x = x, .y = y, .mygrid = g});
        if (cellScore > topScore) {
          topScore = cellScore;
          topTile = &g->tiles[y * WIDTH + x];
        }
      }
    }
  }

  printf("[part1]: %d,%d [has a score of %d)\n", topTile->x, topTile->y,
         topScore);

  return g;
}

int32_t getGridRowScore(int32_t row, int32_t x, int32_t gridSize, grid *g) {
  if (gridSize == 1) {
    return getScore((getScoreStruct){.mygrid = g, .x = x, .y = row});
  }
  if (g->tiles[row * WIDTH + x].cache[gridSize - 1].set) {
    return g->tiles[row * WIDTH + x].cache[gridSize - 1].score;
  }
  int32_t score = getScore((getScoreStruct){.mygrid = g, .x = x, .y = row});
  score += getGridRowScore(row, x + 1, gridSize - 1, g);
  g->tiles[row * WIDTH + x].cache[gridSize - 1] =
      (scoreCache){.score = score, .set = true};
  return score;
}

void getMaxGrid(grid *g) {
  int32_t maxScore = 0;
  int32_t topScoreGridSize = 0;
  tile *topScoreTile = NULL;
  for (size_t y = 0; y < HEIGHT; y++) {
    for (size_t x = 0; x < WIDTH; x++) {
      int32_t maxGridSize = x > y ? WIDTH - x : HEIGHT - y;
      for (size_t gridSize = 1; gridSize <= maxGridSize; gridSize++) {
        int32_t score = 0;
        for (int32_t row = y; row < y + gridSize; row++) {
          score += getGridRowScore(row, x, gridSize, g);
        }
        if (score > maxScore) {
          maxScore = score;
          topScoreGridSize = gridSize;
          topScoreTile = &g->tiles[y * HEIGHT + x];
          printf("top score so far: %d (grid size %d)\n", maxScore,
                 topScoreGridSize);
        }
      }
    }
  }
  printf("[part2]: %d,%d,%d (%d score)\n", topScoreTile->x, topScoreTile->y,
         topScoreGridSize, maxScore);
}

int main(int argc, char **argv) {
  grid *g = initGrid();
  getMaxGrid(g);
  free(g);
  return 0;
}