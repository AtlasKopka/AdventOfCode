#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct game {
  int32_t numPlayers, numMarbles;
  int32_t playerScores[512];
  int32_t marbleToBePlaced;
  int32_t currentMarblePos;
  int32_t numMarblesPlaced;
  int32_t circleSize;
  int32_t currentPlayer;
  int32_t *circle;
} game;

void printGame(game *g) {
#ifndef DEBUG
  return;
#endif
  printf("[%d] ", g->currentPlayer + 1);
  for (size_t i = 0; i < g->circleSize; i++) {
    if (g->currentMarblePos == i) {
      printf("(%d), ", g->circle[i]);
    } else {
      printf("%d, ", g->circle[i]);
    }
  }
  printf("\n");
}

void runGame(game *g) {
  bool cleanup = false;
  if (!g->circle) {
    g->circle = calloc(g->numMarbles * 2, sizeof(g->circle[0]));
    cleanup = true;
    g->currentMarblePos = 1;
    g->numMarblesPlaced = 2;
    g->circleSize = 2;
    g->circle[0] = 0;
    g->circle[1] = 1;
    g->currentPlayer = 1;
  }

  for (; g->numMarblesPlaced <= g->numMarbles;
       g->currentPlayer = (g->currentPlayer + 1) % g->numPlayers) {

    if (g->numMarblesPlaced % 23 == 0) {
      g->playerScores[g->currentPlayer] += g->numMarblesPlaced;
      int32_t marbleToRemove = g->currentMarblePos - 7;
      if (marbleToRemove < 0) {
        marbleToRemove += g->circleSize;
      }
      assert(marbleToRemove >= 0);
      g->playerScores[g->currentPlayer] += g->circle[marbleToRemove];
      int32_t numMarblesToShift = g->circleSize - marbleToRemove - 1;
      assert(numMarblesToShift >= 0);
      if (numMarblesToShift) {
        memmove(g->circle + marbleToRemove, g->circle + marbleToRemove + 1,
                numMarblesToShift * sizeof(int32_t));
      }
      g->circle[g->circleSize] = 0;
      g->circleSize--;
      g->numMarblesPlaced++;
      g->currentMarblePos = marbleToRemove;
      printGame(g);
      continue;
    }

    g->currentMarblePos = (g->currentMarblePos + 2) % g->circleSize;
    if (g->currentMarblePos == 0) {
      g->currentMarblePos = g->circleSize;
    } else {
      size_t marblesToShift = g->circleSize - g->currentMarblePos;
      assert(marblesToShift > 0);
      memmove(g->circle + g->currentMarblePos + 1,
              g->circle + g->currentMarblePos,
              marblesToShift * sizeof(int32_t));
    }
    g->circle[g->currentMarblePos] = g->numMarblesPlaced;
    g->numMarblesPlaced++;
    g->circleSize++;
    printGame(g);
  }

  if (cleanup) {
    free(g->circle);
  }

  int32_t highScore = 0;
  for (size_t i = 0; i < g->numPlayers; i++) {
    if (g->playerScores[i] > highScore) {
      highScore = g->playerScores[i];
    }
  }
  printf("[part1]: high score is %d\n", highScore);
}

int main(int argc, char **argv) {
  assert(argc == 2);
  FILE *fh = fopen(argv[1], "rb");
  int32_t numPlayers, numMarbles;
  fscanf(fh, "%d players; last marble is worth %d points", &numPlayers,
         &numMarbles);
  fclose(fh);
  runGame(&(game){.numPlayers = numPlayers, .numMarbles = numMarbles});
  return 0;
}
