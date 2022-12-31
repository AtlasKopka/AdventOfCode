#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Element {
  int64_t num;
  size_t pos;
  int64_t remainder;
} Element;

typedef struct Group {
  int64_t numIterationsToRepeat;
  Element *positionOfFirstRepeat;
  size_t size;
  bool exists;
  int64_t remainder;
  Element *members[100];
} Group;

typedef struct Groups {
  int64_t sum;
  size_t numElements;
  size_t numGroups;
  Element elements[1024];
  Group groups[1024];
} Groups;

void printGroup(Group *g) {
  printf("Group[remainder: %ld, iterations to repeat: %ld]: ", g->remainder,
         g->numIterationsToRepeat);
  for (size_t i = 0; i < g->size; i++) {
    printf("%ld [pos: %zd],", g->members[i]->num, g->members[i]->pos);
  }
  printf("\n");
}

int groupSort(const void *a, const void *b) {
  Element *elementa = *(Element **)a;
  Element *elementb = *(Element **)b;

  if (elementa->num < elementb->num) {
    return -1;
  } else if (elementa->num > elementb->num) {
    return 1;
  }
  return 0;
}

void addToGroup(Groups *g, Element *e) {
  size_t i;
  for (i = 0;; i++) {
    if (g->groups[i].exists) {
      if (g->groups[i].remainder == e->remainder) {
        break;
      }
    } else {
      g->groups[i].exists = true;
      g->groups[i].remainder = e->remainder;
      g->numGroups++;
      break;
    }
  }
  g->groups[i].members[g->groups[i].size++] = e;
}

void calcNumIterationsToRepeat(Group *g, int64_t sum) {
  if (g->size <= 1) {
    g->numIterationsToRepeat = INT64_MAX;
  } else {
    int64_t best = INT64_MAX;
    Element *pos;
    for (size_t i = 0; i < g->size - 1; i++) {
      int64_t iter;
      if (sum) {
        iter = (labs(g->members[i + 1]->num - g->members[i]->num) / sum);
      } else {
        iter = 1;
      }
      if (iter < best) {
        best = iter;
        pos = g->members[i];
      } else if (iter == best) {
        if (g->members[i]->pos < pos->pos) {
          best = iter;
          pos = g->members[i];
        }
      }
    }
    g->numIterationsToRepeat = best;
    g->positionOfFirstRepeat = pos;
  }
}

void makeGroups(Groups *g) {
  for (size_t i = 0; i < g->numElements; i++) {
    if (g->sum) {
      int64_t remainder = g->elements[i].num % g->sum;
      if (remainder < 0) {
        remainder =
            (g->elements[i].num +
             ((((int64_t)(g->elements[i].num / g->sum)) + 1) * g->sum)) %
            g->sum;
      }
      g->elements[i].remainder = remainder;
    } else {
      g->elements[i].remainder = 1;
    }
    addToGroup(g, &g->elements[i]);
  }

  for (size_t i = 0; i < g->numGroups; i++) {
    qsort(g->groups[i].members, g->groups[i].size, sizeof(Element *),
          groupSort);
    calcNumIterationsToRepeat(&g->groups[i], g->sum);
  }
}

void getFirstDuplicate(Groups *g) {
  Group *bestGroup = &g->groups[0];
  for (size_t i = 0; i < g->numGroups; i++) {
    if (g->groups[i].numIterationsToRepeat < bestGroup->numIterationsToRepeat) {
      bestGroup = &g->groups[i];
    } else if (g->groups[i].numIterationsToRepeat ==
                   bestGroup->numIterationsToRepeat &&
               g->groups[i].positionOfFirstRepeat) {
      if (g->groups[i].positionOfFirstRepeat->pos <
          bestGroup->positionOfFirstRepeat->pos) {
        bestGroup = &g->groups[i];
      }
    }
  }
  for (size_t i = 0; i < bestGroup->size; i++) {
    if (bestGroup->members[i] == bestGroup->positionOfFirstRepeat) {
      printf("First # to repeat is: %ld\n", bestGroup->members[i + 1]->num);
    }
  }
}

int main(int argc, char **argv) {
  Groups *g = calloc(sizeof(Groups), 1);
  FILE *fh = fopen(argv[1], "rb");
  size_t pos = 0;
  int64_t sum = 0;
  for (;;) {
    int64_t num = 0;
    if (fscanf(fh, "%ld", &num) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    g->sum += num;
    sum += num;
    g->elements[g->numElements].pos = pos;
    g->elements[g->numElements].num = sum;
    pos++;
    g->numElements++;
  }
  fclose(fh);
  makeGroups(g);
  getFirstDuplicate(g);
  free(g);
  return 0;
}