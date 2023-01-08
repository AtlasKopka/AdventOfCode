#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node node;
typedef struct node {
  int32_t marble;
  node *next;
  node *prev;
} node;

typedef struct game {
  node *available;
  node arena[];
} game;

game *init(size_t numMarbles) {
  game *g = calloc(sizeof(game) + sizeof(node) * numMarbles, 1);
  for (size_t i = 1; i < numMarbles - 1; i++) {
    g->arena[i].prev = &g->arena[i - 1];
    g->arena[i].next = &g->arena[i + 1];
  }
  g->arena[0].next = &g->arena[1];
  g->arena[0].prev = &g->arena[numMarbles - 1];
  g->arena[numMarbles - 1].next = &g->arena[0];
  g->arena[numMarbles - 1].prev = &g->arena[numMarbles - 2];
  g->available = &g->arena[0];
  return g;
}

void printGame(node *n) {
  node *first = n;
  for (;;) {
    if (n == first) {
      printf("(%d), ", n->marble);
    } else {
      printf("%d, ", n->marble);
    }
    n = n->next;
    if (n == first) {
      break;
    }
  }
  printf("\n");
}

uint64_t runGame(int32_t numPlayers, int32_t numMarbles) {
  uint64_t playerScores[1024] = {0};
  game *g = init(numMarbles);

  node *currentMarblePos = g->available;
  currentMarblePos->marble = 0;
  g->available = g->available->next;
  currentMarblePos->next = g->available;
  currentMarblePos->next->marble = 1;
  g->available = g->available->next;

  currentMarblePos->prev = currentMarblePos->next;
  currentMarblePos->next->next = currentMarblePos;
  currentMarblePos->next->prev = currentMarblePos;

  uint64_t numMarblesPlaced = 2;
  currentMarblePos = currentMarblePos->next;

  for (int32_t currentPlayer = 1;;
       currentPlayer = (currentPlayer + 1) % numPlayers) {
    if (numMarblesPlaced == numMarbles) {
      break;
    }

    if (numMarblesPlaced % 23 == 0) {
      playerScores[currentPlayer] += numMarblesPlaced;
      node *marbleToRemove = currentMarblePos;
      for (size_t i = 0; i < 7; i++) {
        marbleToRemove = marbleToRemove->prev;
      }
      playerScores[currentPlayer] += marbleToRemove->marble;
      currentMarblePos = marbleToRemove->next;
      node *prev = marbleToRemove->prev;
      prev->next = currentMarblePos;
      currentMarblePos->prev = prev;
      marbleToRemove->next = g->available;
      g->available = marbleToRemove;

      numMarblesPlaced++;
      continue;
    }

    currentMarblePos = currentMarblePos->next;
    node *newNode = g->available;
    g->available = g->available->next;
    node *nextNode = currentMarblePos->next;
    currentMarblePos->next = newNode;
    newNode->prev = currentMarblePos;
    newNode->next = nextNode;
    nextNode->prev = newNode;
    newNode->marble = numMarblesPlaced;
    currentMarblePos = newNode;
    numMarblesPlaced++;
  }
  free(g);

  uint64_t topScore = 0;
  for (size_t i = 0; i < numPlayers; i++) {
    if (playerScores[i] > topScore) {
      topScore = playerScores[i];
    }
  }
  return topScore;
}

int main(int argc, char **argv) {
  assert(argc == 2);
  FILE *fh = fopen(argv[1], "rb");
  int32_t numPlayers, numMarbles;
  fscanf(fh, "%d players; last marble is worth %d points", &numPlayers,
         &numMarbles);
  fclose(fh);
  printf("[part1]: %ld\n", runGame(numPlayers, numMarbles));
  printf("[part2]: %ld\n", runGame(numPlayers, numMarbles * 100));
  return 0;
}
