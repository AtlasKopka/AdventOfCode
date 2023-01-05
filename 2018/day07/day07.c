#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dependency dependency;

typedef struct dependency {
  char id;
  size_t numNext;
  size_t numPrev;
  bool done;
  bool successorsNotified;
  size_t numPredecessorsDone;
  size_t secondsUntilDone;
  bool isAvailable;
  dependency *prev[100];
  dependency *next[100];
} dependency;

typedef struct dependencies {
  size_t num;
  size_t numAvailable;
  dependency *available[100];
  size_t numBytes;
  dependency arr[];
} dependencies;

dependencies *allocDependencies() {
  size_t numBytes = sizeof(dependencies) + sizeof(dependency) * 500;
  dependencies *d = calloc(numBytes, 1);
  d->numBytes = numBytes;
  return d;
}

dependency *getOrMakeDependency(dependencies *d, char id) {
  for (size_t i = 0; i < d->num; i++) {
    if (d->arr[i].id == id) {
      return &d->arr[i];
    }
  }
  d->arr[d->num].id = id;
  d->num++;
  return &d->arr[d->num - 1];
}

int dependencyCmp(const void *a, const void *b) {
  if ((*(dependency **)a)->id == (*(dependency **)b)->id) {
    return 0;
  }
  return (*(dependency **)a)->id < (*(dependency **)b)->id ? -1 : 1;
}

void traversept1(dependencies *d) {
  printf("[part1]: ");
  for (;;) {
    bool progressMade = false;
    for (size_t i = 0; i < d->numAvailable; i++) {
      if (d->available[i]->done) {
        continue;
      }
      progressMade = true;
      bool doSort = false;
      printf("%c", d->available[i]->id);
      d->available[i]->done = true;
      for (size_t j = 0; j < d->available[i]->numNext; j++) {
        d->available[i]->next[j]->numPredecessorsDone++;
        if (d->available[i]->next[j]->numPredecessorsDone ==
            d->available[i]->next[j]->numPrev) {
          if (!d->available[i]->next[j]->isAvailable &&
              !d->available[i]->next[j]->done) {
            d->available[d->numAvailable] = d->available[i]->next[j];
            d->numAvailable++;
            d->available[i]->next[j]->isAvailable = true;
            doSort = true;
          }
        }
      }
      if (doSort) {
        qsort(d->available, d->numAvailable, sizeof(d->available[0]),
              dependencyCmp);
      }
      break;
    }
    if (!progressMade) {
      break;
    }
  }
  printf("\n");
}

void traversept2(dependencies *d) {
  size_t agentsAvailable = 5;
  size_t secondsElapsed = 0;
  size_t numDone = 0;

  printf("[part2]:");
  for (; numDone != d->num; secondsElapsed++) {
    for (size_t i = 0; i < d->numAvailable; i++) {
      if (d->available[i]->done) {
        continue;
      }
      if (d->available[i]->secondsUntilDone == 1) {
        d->available[i]->secondsUntilDone = 0;
        d->available[i]->done = true;
        agentsAvailable++;
        numDone++;
        continue;
      } else if (d->available[i]->secondsUntilDone) {
        d->available[i]->secondsUntilDone--;
      }
    }

    for (size_t i = 0; i < d->numAvailable; i++) {
      bool doSort = false;
      if (d->available[i]->done && !d->available[i]->successorsNotified) {
        d->available[i]->successorsNotified = true;
        for (size_t j = 0; j < d->available[i]->numNext; j++) {
          d->available[i]->next[j]->numPredecessorsDone++;
          if (d->available[i]->next[j]->numPredecessorsDone ==
              d->available[i]->next[j]->numPrev) {
            if (!d->available[i]->next[j]->isAvailable &&
                !d->available[i]->next[j]->done) {
              d->available[d->numAvailable] = d->available[i]->next[j];
              d->numAvailable++;
              d->available[i]->next[j]->isAvailable = true;
              doSort = true;
            }
          }
        }
      }
      if (doSort) {
        qsort(d->available, d->numAvailable, sizeof(d->available[0]),
              dependencyCmp);
      }
    }

    for (size_t i = 0; i < d->numAvailable; i++) {
      if (d->available[i]->done) {
        continue;
      }
      if (!d->available[i]->secondsUntilDone && agentsAvailable) {
        agentsAvailable--;
        d->available[i]->secondsUntilDone = 60 + d->available[i]->id - 'A' + 1;
      }
    }
  }
  secondsElapsed--;
  printf(" %zd seconds elapsed\n", secondsElapsed);
}

int main(int argc, char **argv) {
  for (size_t part = 0; part < 2; part++) {
    dependencies *d = allocDependencies();
    FILE *fh = fopen(argv[1], "rb");
    assert(fh);
    for (;;) {
      char a, b;
      if (fscanf(fh, "Step %c must be finished before step %c can begin.", &a,
                 &b) == EOF) {
        break;
      }
      fscanf(fh, "%*[\r\n]");
      dependency *pre = getOrMakeDependency(d, a);
      dependency *post = getOrMakeDependency(d, b);
      pre->next[pre->numNext++] = post;
      post->prev[post->numPrev++] = pre;
    }
    fclose(fh);

    for (size_t i = 0; i < d->num; i++) {
      if (!d->arr[i].numPrev) {
        d->available[d->numAvailable++] = &d->arr[i];
        d->arr[i].isAvailable = true;
      }
    }
    qsort(d->available, d->numAvailable, sizeof(d->available[0]),
          dependencyCmp);

    switch (part) {
    case 0:
      traversept1(d);
      break;
    default:
      traversept2(d);
    }

    free(d);
  }
  return 0;
}