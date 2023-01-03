#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void reduce(char **origbuf, size_t len) {
  char *buf = *origbuf;
  size_t reducedLength = len;
  for (;;) {
    bool reductionFound = false;
    for (size_t i = 0; i < len; i++) {
      if (buf[i] == '_') {
        continue;
      }
      size_t j;
      bool subsequentFound = false;
      for (j = i + 1; j < len; j++) {
        if (buf[j] == '_') {
          continue;
        }
        subsequentFound = true;
        break;
      }
      if (!subsequentFound) {
        break;
      }
      if (abs(buf[i] - buf[j]) == 32) {
        buf[i] = '_';
        buf[j] = '_';
        reductionFound = true;
        reducedLength -= 2;
      }
    }
    if (!reductionFound) {
      break;
    }
  }

  char *newbuf = calloc(reducedLength + 1, 1);
  size_t buflen = 0;
  for (size_t i = 0; i < len; i++) {
    if (buf[i] != '_') {
      newbuf[buflen++] = buf[i];
    }
  }

  char *tmp = *origbuf;
  *origbuf = newbuf;
  free(tmp);
}

char fixPolymer(char **origbuf, size_t buflen) {
  char *buf = *origbuf;

  size_t numReactionsPrevented[26] = {0};

  for (size_t i = 1; i < buflen - 1; i++) {
    for (size_t j = 1; j <= i && i + j < buflen; j++) {
      if (abs(buf[i - j] - buf[i + j]) == 32) {
        char preventer = buf[i];
        if (preventer > 'Z') {
          preventer -= 32;
        }
        numReactionsPrevented[(int)(preventer - 'A')]++;
      } else {
        break;
      }
    }
  }

  char bestPreventer;
  size_t bestPreventerScore = 0;
  for (size_t i = 0; i < 26; i++) {
    if (numReactionsPrevented[i] > bestPreventerScore) {
      bestPreventerScore = numReactionsPrevented[i];
      bestPreventer = (char)(i + 'A');
    }
  }

  for (size_t i = 0; i < buflen; i++) {
    if (buf[i] == bestPreventer || buf[i] == (bestPreventer + 32)) {
      buf[i] = '_';
    }
  }
  reduce(origbuf, buflen);
  return bestPreventer;
}

int main(int argc, char **argv) {
  FILE *fh = fopen(argv[1], "rb");
  assert(fh);
  fseek(fh, 0, SEEK_END);
  char *buf = calloc(ftell(fh) + 1, 1);
  assert(buf);
  fseek(fh, 0, SEEK_SET);
  fscanf(fh, "%[^\r\n]", buf);
  fclose(fh);

  reduce(&buf, strlen(buf));
  printf("[part1]: reduced length is %zd\n", strlen(buf));
  char removePolymer = fixPolymer(&buf, strlen(buf));
  printf("[part2]: %zd is the length of the shortest polymer that can be "
         "reduced be removing '%c'\n",
         strlen(buf), removePolymer);

  free(buf);
  return 0;
}