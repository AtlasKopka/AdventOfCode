#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOUBLES 0
#define TRIPLES 1

typedef struct corpus {
  size_t size;
  size_t linelen;
  char lines[];
} corpus;

corpus *initCorpus() {
  corpus *c = calloc(sizeof(corpus) + 1024 * 1024, 1);
  c->linelen = 1024;
  return c;
}

void part2(corpus *c) {
  for (size_t i = 0; i < c->size; i++) {
    for (size_t j = i + 1; j < c->size; j++) {
      size_t numCharactersDiff = 0;
      for (size_t k = 0; k < strlen(&c->lines[i * c->linelen]); k++) {
        if (c->lines[i * c->linelen + k] != c->lines[j * c->linelen + k]) {
          numCharactersDiff++;
        }
      }
      if (numCharactersDiff == 1) {
        printf("[part2]:\t");
        for (size_t w = 0; w < strlen(&c->lines[j * c->linelen]); w++) {
          if (c->lines[i * c->linelen + w] == c->lines[j * c->linelen + w]) {
            printf("%c", c->lines[i * c->linelen + w]);
          }
        }
        printf("\n");
        break;
      }
    }
  }
}

int main(int argc, char **argv) {
  corpus *c = initCorpus();
  FILE *fh = fopen(argv[1], "rb");
  size_t totals[2] = {0};
  for (;;) {
    char counts[52] = {0};
    char line[1024] = {0};
    if (fscanf(fh, "%[^\r\n]", line) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    for (size_t i = 0; i < strlen(line); i++) {
      counts[line[i] - 'a']++;
    }
    bool doublesfound = false, triplesfound = false;
    for (size_t i = 0; i < sizeof(counts); i++) {
      if (counts[i] == 2 && !doublesfound) {
        totals[DOUBLES]++;
        doublesfound = true;
      }
      if (counts[i] == 3 && !triplesfound) {
        totals[TRIPLES]++;
        triplesfound = true;
      }
    }
    memcpy(c->lines + c->size * c->linelen, line, 1024);
    c->size++;
  }
  fclose(fh);
  int32_t checksum = totals[DOUBLES] * totals[TRIPLES];
  printf("[part1] checksum: %d\n", checksum);
  part2(c);
  free(c);
  return 0;
}