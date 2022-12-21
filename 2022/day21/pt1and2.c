#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Monkey {
  char name[4];
  bool isNum;
  int64_t num;

  char operator;
  char sourceA[4];
  size_t sourceAidx;
  char sourceB[4];
  size_t sourceBidx;
  size_t idx;
  bool humnInvolved;
} Monkey;

typedef struct Monkeys {
  ssize_t rootIdx;
  size_t size;
  Monkey arr[];
} Monkeys;

Monkeys *initMonkeys() {
  Monkeys *m = calloc(sizeof(Monkeys) + sizeof(Monkey) * 2000, 1);
  m->rootIdx = -1;
  return m;
}

void printMonkey(Monkey *m) {
  if (m->isNum) {
    printf("%.4s: %ld\n", m->name, m->num);
  } else {
    printf("%.4s: %.4s[%zd] [%c] %.4s[%zd]\n", m->name, m->sourceA,
           m->sourceAidx, m->operator, m->sourceB, m->sourceBidx);
  }
}

typedef struct monkeyResult {
  int64_t num;
  bool humnInvolved;
} monkeyResult;

monkeyResult monkeyBusiness(Monkeys *m, Monkey *me) {
  if (me->isNum) {
    if (memcmp(me->name, "humn", 4) == 0) {
      me->humnInvolved = true;
    }
    return (monkeyResult){.num = me->num, .humnInvolved = me->humnInvolved};
  }

  monkeyResult a = monkeyBusiness(m, &m->arr[me->sourceAidx]);
  monkeyResult b = monkeyBusiness(m, &m->arr[me->sourceBidx]);
  me->humnInvolved = a.humnInvolved || b.humnInvolved;
  switch (me->operator) {
  case '-': {
    return (monkeyResult){.num = a.num - b.num,
                          .humnInvolved = me->humnInvolved};
  }
  case '+': {
    return (monkeyResult){.num = a.num + b.num,
                          .humnInvolved = me->humnInvolved};
  }
  case '*': {
    return (monkeyResult){.num = a.num * b.num,
                          .humnInvolved = me->humnInvolved};
  }
  case '/': {
    return (monkeyResult){.num = a.num / b.num,
                          .humnInvolved = me->humnInvolved};
  }
  }
  assert(false);
}

int64_t reverseCalc(Monkeys *m, Monkey *me, int64_t parentGoal) {
  assert(me->humnInvolved);
  assert(parentGoal);

  if (me->isNum) {
    return parentGoal;
  }

  int64_t child;
  bool firstOperandIsHuman = false;
  if (!m->arr[me->sourceAidx].humnInvolved) {
    child = monkeyBusiness(m, &m->arr[me->sourceAidx]).num;
  } else {
    child = monkeyBusiness(m, &m->arr[me->sourceBidx]).num;
    firstOperandIsHuman = true;
  }

  switch (me->operator) {
  case '-': {
    if (firstOperandIsHuman) {
      parentGoal = parentGoal + child;
    } else {
      parentGoal = child - parentGoal;
    }
    break;
  }
  case '+': {
    parentGoal = parentGoal - child;
    break;
  }
  case '*': {
    parentGoal = parentGoal / child;
    break;
  }
  case '/': {
    if (firstOperandIsHuman) {
      parentGoal = parentGoal * child;
    } else {
      parentGoal = child / parentGoal;
    }
    break;
  }
  default: {
    assert(false);
  }
  }

  if (firstOperandIsHuman) {
    return reverseCalc(m, &m->arr[me->sourceAidx], parentGoal);
  } else {
    return reverseCalc(m, &m->arr[me->sourceBidx], parentGoal);
  }
  assert(false);
}

int64_t pt2(Monkeys *m) {
  Monkey *r = &m->arr[m->rootIdx];
  Monkey *humnParent;
  int64_t goal;
  if (m->arr[r->sourceAidx].humnInvolved) {
    humnParent = &m->arr[r->sourceAidx];
    goal = monkeyBusiness(m, &m->arr[r->sourceBidx]).num;
  } else {
    humnParent = &m->arr[r->sourceBidx];
    goal = monkeyBusiness(m, &m->arr[r->sourceAidx]).num;
  }

  return reverseCalc(m, humnParent, goal);
}

int main(int argc, char **argv) {
  Monkeys *m = initMonkeys();
  FILE *fh = fopen(argv[1], "rb");
  assert(fh);
  for (;;) {
    int64_t num = 0;
    char newMonkey[5] = {0};
    char sourceMonkeyA[5] = {0};
    char sourceMonkeyB[5] = {0};
    char operator[2] = {0};
    char line[1024] = {0};
    if (fscanf(fh, "%[^\r\n]", line) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");

    if (sscanf(line, "%[a-z]: %ld", newMonkey, &num) == 2) {
      Monkey *mm = &m->arr[m->size];
      mm->isNum = true;
      mm->idx = m->size;
      memcpy(mm->name, newMonkey, 4);
      mm->num = num;
      m->size++;
    } else if (sscanf(line, "%[a-z]: %[a-z] %[+*/-] %[a-z]", newMonkey,
                      sourceMonkeyA, operator, sourceMonkeyB) == 4) {
      Monkey *mm = &m->arr[m->size];
      memcpy(mm->name, newMonkey, 4);
      mm->operator= operator[0];
      memcpy(mm->sourceA, sourceMonkeyA, 4);
      memcpy(mm->sourceB, sourceMonkeyB, 4);
      m->size++;
    } else {
      assert(false);
    }
  }
  fclose(fh);

  for (size_t i = 0; i < m->size; i++) {
    if (memcmp(m->arr[i].name, "root", 4) == 0) {
      m->rootIdx = i;
    }
    if (!m->arr[i].isNum) {
      for (size_t j = 0; j < 2; j++) {
        char *query = m->arr[i].sourceA;
        if (j) {
          query = m->arr[i].sourceB;
        }
        size_t idx;
        bool idxFound = false;
        for (size_t k = 0; k < m->size; k++) {
          if (k == i) {
            continue;
          }
          if (memcmp(m->arr[k].name, query, 4) == 0) {
            idx = k;
            idxFound = true;
            break;
          }
        }
        assert(idxFound);
        if (!j) {
          m->arr[i].sourceAidx = idx;
        } else {
          m->arr[i].sourceBidx = idx;
        }
      }
    }
  }

  monkeyResult part1 = monkeyBusiness(m, &m->arr[m->rootIdx]);
  printf("[part1]: %ld\n", part1.num);
  printf("[part2]: %ld\n", pt2(m));

  free(m);
  return 0;
}