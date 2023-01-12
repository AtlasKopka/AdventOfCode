#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAP (8192 * 2)

void printState(char *state, int32_t generation) {
  ssize_t i;
  for (i = 0; i < CAP; i++) {
    if (state[i] != '.') {
      break;
    }
  }
  size_t len;
  for (len = CAP - 1; len >= 0; len--) {
    if (state[len] != '.') {
      break;
    }
  }
  printf("<#%d> [%d]%*.*s[%zd]\n", generation, (int)(i - CAP / 2),
         (int)(len - i), (int)(len - i), state + i, len - CAP / 2);
}

void run(char *state, size_t numGenerations, char *rules, size_t numRules) {
  char nextState[CAP];
  memcpy(nextState, state, CAP);

  for (size_t generation = 0; generation < numGenerations; generation++) {
    for (size_t i = 0; i < CAP - 5; i++) {
      if (memcmp(state + i, ".....", 5) == 0) {
        continue;
      }
      for (size_t j = 0; j < numRules; j++) {
        if (memcmp(rules + j * 6, state + i, 5) == 0) {
          if (nextState[i + 2] != rules[j * 6 + 5]) {
            nextState[i + 2] = rules[j * 6 + 5];
          }
          break;
        }
      }
    }
    memcpy(state, nextState, CAP);
  }
}

int32_t getScore(char *state) {
  int32_t score = 0;
  for (size_t i = 0; i < CAP; i++) {
    if (state[i] == '#') {
      score += (i - CAP / 2);
    }
  }
  return score;
}

void part1(char *state, char *rules, size_t numRules) {
  run(state, 20, rules, numRules);
  printf("[part1]: %d\n", getScore(state));
}

void part2(char *state, char *rules, size_t numRules) {
  int64_t repeatGen = 512;
  run(state, repeatGen, rules, numRules);
  int64_t startPos;
  for (startPos = 0; state[startPos] == '.'; startPos++) {
  }
  startPos -= (CAP / 2);
  int64_t startPosForGen = startPos - repeatGen;

  bool startFound = false;
  int64_t score = 0;
  int64_t posWeight = 50000000000ll + startPosForGen;
  for (size_t i = 0; i < CAP; i++) {
    if (state[i] == '#') {
      startFound = true;
      score += posWeight;
    }
    if (startFound) {
      posWeight++;
    }
  }
  printf("[part2]: %ld\n", score);
}

int main(int argc, char **argv) {
  size_t numRules = 0;
  char rules[64 * 6] = {0};
  FILE *fh = fopen(argv[1], "rb");
  char state[CAP] = {0};
  fscanf(fh, "initial state: %[#.]", state + (CAP / 2));
  for (size_t i = 0; i < CAP; i++) {
    if (!state[i]) {
      state[i] = '.';
    }
  }
  fscanf(fh, "%*[\r\n]");
  for (;;) {
    char trigger[6] = {0};
    char result[2] = {0};
    if (fscanf(fh, "%[#.] => %[#.]", trigger, result) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    memcpy(rules + numRules * 6, trigger, 5);
    rules[numRules * 6 + 5] = result[0];
    numRules++;
  }
  fclose(fh);

  char tmpState[CAP];
  memcpy(tmpState, state, CAP);
  part1(tmpState, rules, numRules);
  part2(state, rules, numRules);

  return 0;
}