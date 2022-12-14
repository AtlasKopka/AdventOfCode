#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Elem {
  int64_t digit;
  bool isDigit;
  bool open;
  bool close;
} Elem;

typedef struct Elems {
  size_t numElems;
  Elem arr[1024];
} Elems;

struct Sublist;
typedef struct Sublist {
  bool isNumber;
  int64_t number;
  size_t numsublists;
  void *sublists[10];
} Sublist;

typedef struct Sublists {
  size_t num;
  Sublist *arr[1024];
} Sublists;

Sublist *addSublist(void *list, bool isDigit, int64_t number) {
  Sublist *sublist = calloc(sizeof(Sublist), 1);
  if (!list) {
    return sublist;
  }

  if (isDigit) {
    sublist->isNumber = true;
    sublist->number = number;
  }

  ((Sublist *)list)->sublists[((Sublist *)list)->numsublists++] = sublist;

  return sublist;
}

void printTabs(size_t n) {
  for (size_t i = 0; i < n; i++) {
    printf("\t");
  }
}

void tokenize(char *list, Elems *myelems) {
  for (size_t i = 0; i < strlen(list); i++) {
    if (list[i] == '[') {
      myelems->arr[myelems->numElems].open = true;
      myelems->numElems++;
    } else if (list[i] == ']') {
      myelems->arr[myelems->numElems].close = true;
      myelems->numElems++;
    } else {
      char num[10] = {0};
      size_t j = i;
      for (; j < strlen(list); j++) {
        if (list[j] >= '0' && list[j] <= '9') {
          num[strlen(num)] = list[j];
        } else {
          break;
        }
      }
      if (strlen(num)) {
        sscanf(num, "%ld", &myelems->arr[myelems->numElems].digit);
        myelems->arr[myelems->numElems].isDigit = true;
        myelems->numElems++;
      }
    }
  }
}

void freeSublist(Sublist *list) {
  for (size_t i = 0; i < list->numsublists; i++) {
    freeSublist(list->sublists[i]);
  }
  free(list);
}

void printSublist(Sublist *list) {
  if (list->isNumber) {
    printf("%ld ", list->number);
  } else {
    printf("[ ");
    for (size_t i = 0; i < list->numsublists; i++) {
      printSublist(list->sublists[i]);
    }
    printf(" ]");
  }
}

typedef enum comparison { INORDER, OUTOFORDER, KEEPCHECKING } comparison;

void convertNumberToList(Sublist *list) {
  int64_t number = list->number;
  list->number = 0;
  list->isNumber = false;
  addSublist(list, true, number);
}

comparison areSublistsInProperOrder(Sublist *a, Sublist *b) {
  if (a == NULL && b == NULL) {
    return KEEPCHECKING;
  }

  if (a == NULL) {
    return INORDER;
  }

  if (b == NULL) {
    return OUTOFORDER;
  }

  if (a->isNumber != b->isNumber) {
    if (a->isNumber) {
      convertNumberToList(a);
    } else {
      convertNumberToList(b);
    }
    return areSublistsInProperOrder(a, b);
  }

  if (a->isNumber) {
    if (a->number < b->number) {
      return INORDER;
    } else if (a->number > b->number) {
      return OUTOFORDER;
    }
    return KEEPCHECKING;
  }

  size_t numSublistsToCheck = a->numsublists;
  if (b->numsublists > numSublistsToCheck) {
    numSublistsToCheck = b->numsublists;
  }
  for (size_t i = 0; i <= numSublistsToCheck; i++) {
    comparison result =
        areSublistsInProperOrder(a->sublists[i], b->sublists[i]);
    if (result != KEEPCHECKING) {
      return result;
    }
  }
  return KEEPCHECKING;
}

Sublist *parseList(char *list) {
  Elems *myelems = calloc(sizeof(Elems), 1);
  tokenize(list, myelems);

  size_t sublists[1024] = {0};
  size_t sublistnum = 0;

  Sublist *stack[1024] = {0};
  size_t stacksize = 0;

  size_t listlen = myelems->numElems;
  for (size_t i = 0; i < listlen; i++) {
    if (myelems->arr[i].open) {
      sublists[sublistnum++] = i;
      if (stacksize) {
        stack[stacksize] = addSublist(stack[stacksize - 1], false, 0);
        stacksize++;
      } else {
        stack[0] = addSublist(NULL, false, 0);
        stacksize++;
      }
    }
    if (myelems->arr[i].close) {
      size_t startbracketpos = sublists[sublistnum - 1];
      sublists[--sublistnum] = 0;
      for (size_t j = startbracketpos; j <= i; j++) {
        if (myelems->arr[j].open) {
        } else if (myelems->arr[j].close) {
        } else {
        }
      }
      stacksize--;
    }
    if (myelems->arr[i].isDigit) {
      addSublist(stack[stacksize - 1], true, myelems->arr[i].digit);
    }
  }

  free(myelems);
  return (stack[0]);
}

Sublists *parseFile(char *filename) {
  Sublists *lists = calloc(sizeof(Sublists), 1);

  FILE *fh = fopen(filename, "rb");
  for (;;) {
    char line[1024] = {0};
    if (EOF == fscanf(fh, "%[^\r\n]", line)) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    if (strlen(line)) {
      lists->arr[lists->num++] = parseList(line);
    }
  }
  fclose(fh);
  return lists;
}

void freeSublists(Sublists *lists) {
  for (size_t i = 0; i < lists->num; i++) {
    freeSublist(lists->arr[i]);
  }
  free(lists);
}

void part1(Sublists *packets) {
  size_t index = 1;
  size_t score = 0;
  for (size_t i = 0; i < packets->num; i += 2) {
    if (areSublistsInProperOrder(packets->arr[i], packets->arr[i + 1]) ==
        INORDER) {
      score += index;
    }
    index++;
  }
  printf("Part 1 score: %ld\n", score);
}

void printSublists(Sublists *list) {
  for (size_t i = 0; i < list->num; i++) {
    printSublist(list->arr[i]);
    printf("\n");
  }
}

void part2(Sublists *packets) {
  char *dividers[] = {"[[2]]", "[[6]]"};
  Sublist *dividerptrs[2] = {0};

  for (size_t i = 0; i < 2; i++) {
    dividerptrs[i] = parseList(dividers[i]);
    packets->arr[packets->num++] = dividerptrs[i];
  }

  for (size_t i = 0; i < packets->num; i++) {
    for (size_t j = 0; j < packets->num; j++) {
      if (areSublistsInProperOrder(packets->arr[i], packets->arr[j]) ==
          INORDER) {
        Sublist *tmp = packets->arr[i];
        packets->arr[i] = packets->arr[j];
        packets->arr[j] = tmp;
      }
    }
  }

  size_t score = 1;
  for (size_t i = 0; i < packets->num; i++) {
    for (size_t j = 0; j < sizeof(dividerptrs) / sizeof(dividerptrs[0]); j++) {
      if (packets->arr[i] == dividerptrs[j]) {
        score *= (i + 1);
        break;
      }
    }
  }
  printf("Part 2 score: %ld\n", score);
}

int main(int argc, char **argv) {
  assert(argv[1]);
  char *filename = argv[1];

  Sublists *packets = parseFile(filename);
  part1(packets);
  part2(packets);

  freeSublists(packets);

  return 0;
}