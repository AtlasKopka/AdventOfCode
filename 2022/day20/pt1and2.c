#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 7000

typedef struct Val Val;

typedef struct Val {
  int64_t value;
  int64_t origPos;
  int64_t sortedPos;
  Val *next;
  Val *prev;
} Val;

typedef struct Values {
  int64_t size;
  int64_t capacity;
  Val **sortedArr;
  Val origArr[];
} Values;

Values *initValues() {
  Values *v = calloc(sizeof(Values) + sizeof(Val) * CAPACITY, 1);
  v->sortedArr = calloc(sizeof(Val *) * CAPACITY, 1);
  v->capacity = CAPACITY;
  return v;
}

void printList(Values *msg) {
  Val *v = &msg->origArr[0];
  for (int64_t i = 0; i < msg->size; i++) {
    printf("%ld", v->value);
    v = v->next;
    printf(" -> ");
    assert(v->prev->next == v);
    assert(v->next->prev == v);
  }
  printf("\n");
}

Val *getValByIncrement(Val *start, int64_t increment) {
  for (int64_t i = 0; i < increment; i++) {
    start = start->next;
  }
  return start;
}

int64_t computeScore(Values *msg, int64_t key) {
  Val *start = NULL;
  for (int64_t i = 0; i < msg->size; i++) {
    if (msg->origArr[i].value == 0) {
      start = &msg->origArr[i];
      break;
    }
  }

  assert(start->value == 0);

  int64_t indices[] = {1000, 2000, 3000};
  int64_t score = 0;
  for (int64_t i = 0; i < 3; i++) {
    int64_t coord = getValByIncrement(start, indices[i])->value;
    score += coord * key;
  }
  return score;
}

void doSortPt2(Values *msg) {
  int64_t key = 811589153l % (msg->size - 1);

  for (size_t z = 0; z < 10; z++) {
    for (int64_t i = 0; i < msg->size; i++) {
      Val *v = &msg->origArr[i];
      int64_t coord = v->value * key;
      if (coord == 0) {
        continue;
      }

      int64_t normalizedValue = coord % ((int64_t)msg->size - 1);
      if (normalizedValue < 0) {
        normalizedValue += msg->size - 1;
        normalizedValue %= ((int64_t)msg->size - 1);
      }

      if (!normalizedValue) {
        continue;
      }

      Val *target = v;
      for (int64_t j = 0; j < normalizedValue; j++) {
        target = target->next;
      }

      v->prev->next = v->next;
      v->next->prev = v->prev;

      v->next = target->next;
      v->prev = target;

      target->next->prev = v;
      target->next = v;
    }
  }

  printf("[part2]: %ld\n", computeScore(msg, 811589153l));
}

void doSortPt1(Values *msg) {
  for (int64_t i = 0; i < msg->size; i++) {
    Val *v = &msg->origArr[i];
    if (v->value == 0) {
      continue;
    }

    int64_t normalizedValue = msg->origArr[i].value % ((int64_t)msg->size - 1);
    if (normalizedValue < 0) {
      normalizedValue += msg->size - 1;
    }

    if (!normalizedValue) {
      continue;
    }

    Val *target = v;
    for (int64_t j = 0; j < normalizedValue; j++) {
      target = target->next;
    }

    v->prev->next = v->next;
    v->next->prev = v->prev;

    v->next = target->next;
    v->prev = target;

    target->next->prev = v;
    target->next = v;
  }

  printf("[part1]: %ld\n", computeScore(msg, 1));
}

int main(int argc, char **argv) {
  for (size_t part = 0; part < 2; part++) {
    Values *msg = initValues();
    FILE *fh = fopen(argv[1], "rb");
    assert(fh);
    int64_t min = 0, max = 0;

    for (;;) {
      int64_t val;
      if (fscanf(fh, "%ld", &val) == EOF) {
        break;
      }
      if (val > max) {
        max = val;
      }
      if (val < min) {
        min = val;
      }

      msg->origArr[msg->size].origPos = msg->size;
      msg->origArr[msg->size].sortedPos = msg->size;
      msg->origArr[msg->size].value = val;
      *(msg->sortedArr + msg->size) = &msg->origArr[msg->size];

      if (msg->size) {
        msg->origArr[msg->size].prev = &msg->origArr[msg->size - 1];
        msg->origArr[msg->size - 1].next = &msg->origArr[msg->size];
      }

      msg->size++;
    }
    msg->origArr[0].prev = &msg->origArr[msg->size - 1];
    msg->origArr[msg->size - 1].next = &msg->origArr[0];

    fclose(fh);
    if (!part) {
      doSortPt1(msg);
    } else {
      doSortPt2(msg);
    }
    free(msg->sortedArr);
    free(msg);
  }

  return 0;
}