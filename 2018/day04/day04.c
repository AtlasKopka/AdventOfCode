#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct guard {
  size_t id;
  size_t totalSleepTime;
  size_t sleepytime[60];
} guard;

typedef struct guards {
  size_t numGuards;
  guard *sleepiestGuard;
  guard guardarr[1024];
} guards;

int sort(const void *linea, const void *lineb) {
  return strncmp(linea, lineb, 512);
}

guard *getGuard(guards *g, size_t id) {
  for (size_t i = 0; i < g->numGuards; i++) {
    if (g->guardarr[i].id == id) {
      return &g->guardarr[i];
    }
  }
  guard *newguard = &g->guardarr[g->numGuards];
  g->numGuards++;
  newguard->id = id;
  return newguard;
}

void populate(guards *g, char *lines, size_t linenum) {
  size_t guardID;
  int startSleep = 0;
  int endSleep = 0;
  guard *currguard = NULL;
  for (size_t i = 0; i < linenum; i++) {
    if (sscanf(lines + i * 512, "%*[^G] Guard #%zd", &guardID) == 1) {
      currguard = getGuard(g, guardID);
    } else if (strstr(lines + i * 512, "falls")) {
      assert(sscanf(lines + i * 512, "%*[^:]:%d] falls", &startSleep) == 1);
    } else if (strstr(lines + i * 512, "wakes")) {
      assert(sscanf(lines + i * 512, "%*[^:]:%d] wakes", &endSleep) == 1);
      for (size_t j = startSleep; j < endSleep; j++) {
        currguard->sleepytime[j]++;
      }
      currguard->totalSleepTime += (endSleep - startSleep);
      if (!g->sleepiestGuard ||
          g->sleepiestGuard->totalSleepTime < currguard->totalSleepTime) {
        g->sleepiestGuard = currguard;
      }
    }
  }
}

void findBestMinute(guards *g) {
  int sleepiestMinute;
  int timeSleptDuringSleepiestMinute = 0;
  for (size_t i = 0; i < 60; i++) {
    if (g->sleepiestGuard->sleepytime[i] > timeSleptDuringSleepiestMinute) {
      sleepiestMinute = i;
      timeSleptDuringSleepiestMinute = g->sleepiestGuard->sleepytime[i];
    }
  }
  printf("[part1]: %d\n", sleepiestMinute * (int)g->sleepiestGuard->id);
}

void findSleepiestMinute(guards *g) {
  guard *sleepiestGuard;
  int sleepiestMinute;
  int totalTimeForSleepiestMinute = 0;
  for (size_t i = 0; i < g->numGuards; i++) {
    for (size_t j = 0; j < 60; j++) {
      if (g->guardarr[i].sleepytime[j] > totalTimeForSleepiestMinute) {
        sleepiestGuard = &g->guardarr[i];
        sleepiestMinute = j;
        totalTimeForSleepiestMinute =
            sleepiestGuard->sleepytime[sleepiestMinute];
      }
    }
  }
  printf("[part2]: %d\n", (int)sleepiestGuard->id * sleepiestMinute);
}

int main(int argc, char **argv) {
  guards *g = calloc(sizeof(guards), 1);
  size_t linenum = 0;
  char lines[1024 * 1024] = {0};
  FILE *fh = fopen(argv[1], "rb");
  for (;;) {
    if (fscanf(fh, "%[^\r\n]", lines + linenum * 512) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    linenum++;
  }
  fclose(fh);
  qsort(lines, linenum, 512, sort);
  populate(g, lines, linenum);
  findBestMinute(g);
  findSleepiestMinute(g);
  free(g);
  return 0;
}