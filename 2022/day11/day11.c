#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MONKEYCAPACITY 1024
#define MAXITEMS 1024

typedef enum operator{ ADD, DIV, MULT, SUB } operator;

typedef struct operation {
  operator operatorType;
  int64_t operand0;
  int64_t operand1;
  bool isFirstOperandSelf;
  bool isSecondOperandSelf;
} operation;

void parseStringToOperation(char *line, operation *op) {
  char operand0[10] = {0}, operand1[10] = {0}, operator[10] = {0};
  if (sscanf(line, " Operation: new = %s %s %s", operand0, operator,
             operand1)) {
    if (operand0[0] == 'o') {
      op->isFirstOperandSelf = true;
    } else {
      sscanf(operand0, "%zd", &op->operand0);
    }
    if (operand1[0] == 'o') {
      op->isSecondOperandSelf = true;
    } else {
      sscanf(operand1, "%zd", &op->operand1);
    }
    switch (operator[0]) {
    case ('-'): {
      op->operatorType = SUB;
      break;
    }
    case ('+'): {
      op->operatorType = ADD;
      break;
    }
    case ('/'): {
      op->operatorType = DIV;
      break;
    }
    case ('*'): {
      op->operatorType = MULT;
      break;
    }
      assert(0);
    }
  }
}

typedef struct MonkeyTest {
  size_t falseMonkeyId;
  size_t trueMonkeyId;
  int64_t divisor;
} MonkeyTest;

typedef struct Monkey {
  size_t id;
  size_t numItems;
  operation op;
  MonkeyTest test;
  size_t numInspections;
  size_t items[MAXITEMS];
} Monkey;

typedef struct Monkeys {
  size_t numMonkeys;
  size_t monkeyCapacity;
  bool isPart2;
  int64_t modulo;
  Monkey zoo[];
} Monkeys;

void multiplyDivisorsTogether(Monkeys *mymonkeys) {
  mymonkeys->modulo = 1;
  for (size_t i = 0; i < mymonkeys->numMonkeys; i++) {
    mymonkeys->modulo *= mymonkeys->zoo[i].test.divisor;
  }
}

void addItemToMonkey(Monkeys *mymonkeys, size_t item, size_t monkeyid) {
  Monkey *monkeyptr = &mymonkeys->zoo[monkeyid];
  monkeyptr->items[monkeyptr->numItems] = item;
  monkeyptr->numItems++;
}

void monkeyThrowItem(Monkeys *mymonkeys, size_t sourceMonkeyId,
                     size_t dstMonkeyId, size_t itemIndex) {
  if (sourceMonkeyId != dstMonkeyId) {
    Monkey *srcMonkeyPtr = &mymonkeys->zoo[sourceMonkeyId];
    Monkey *dstMonkeyPtr = &mymonkeys->zoo[dstMonkeyId];

    int64_t val = srcMonkeyPtr->items[itemIndex];
    /*
    printf("Throwing %ld from %zd to %zd...\n", val, sourceMonkeyId,
           dstMonkeyId);
           */
    memmove(srcMonkeyPtr->items + itemIndex,
            srcMonkeyPtr->items + itemIndex + 1,
            sizeof(srcMonkeyPtr->items[0]) * (MAXITEMS - itemIndex - 1));
    srcMonkeyPtr->numItems--;

    dstMonkeyPtr->items[dstMonkeyPtr->numItems++] = val;
  }
}

bool testItem(Monkey *monkeyptr, size_t itemIndex) {
  int64_t val = monkeyptr->items[itemIndex];
  return val % monkeyptr->test.divisor == 0;
}

void monkeyInspectItem(Monkeys *mymonkeys, size_t monkeyid) {
  Monkey *monkeyptr = &mymonkeys->zoo[monkeyid];
  for (ssize_t i = (ssize_t)monkeyptr->numItems - 1; i >= 0; i--) {
    int64_t old = monkeyptr->items[i];
    int64_t newvalue;
    if (monkeyptr->op.isFirstOperandSelf) {
      newvalue = old;
    } else {
      newvalue = monkeyptr->op.operand0;
    }
    int64_t operand;
    if (monkeyptr->op.isSecondOperandSelf) {
      operand = old;
    } else {
      operand = monkeyptr->op.operand1;
    }
    switch (monkeyptr->op.operatorType) {
    case (ADD): {
      newvalue += operand;
      break;
    }
    case (MULT): {
      newvalue *= operand;
      break;
    }
    case (SUB): {
      newvalue *= operand;
      break;
    }
    case (DIV): {
      newvalue /= operand;
    }
    }
    if (!mymonkeys->isPart2) {
      newvalue /= 3;
    } else {
      newvalue %= mymonkeys->modulo;
    }

    monkeyptr->items[i] = newvalue;

    if (testItem(monkeyptr, i)) {
      monkeyThrowItem(mymonkeys, monkeyid, monkeyptr->test.trueMonkeyId, i);
    } else {
      monkeyThrowItem(mymonkeys, monkeyid, monkeyptr->test.falseMonkeyId, i);
    }
    monkeyptr->numInspections++;
  }
  // printf("\n");
}

void printMonkey(Monkeys *mymonkeys, size_t monkeyid) {
  Monkey *monkeyptr = &mymonkeys->zoo[monkeyid];
  assert(monkeyptr);
  printf("[Monkey %zd]:\n", monkeyptr->id);

  printf("\t[Items: ");
  for (size_t i = 0; i < monkeyptr->numItems; i++) {
    printf("%zd, ", monkeyptr->items[i]);
  }
  printf("]");
  printf("\n");

  printf("\t[Operation:");
  if (monkeyptr->op.isFirstOperandSelf) {
    printf(" (old) ");
  } else {
    printf("%zd", monkeyptr->op.operand0);
  }
  switch (monkeyptr->op.operatorType) {
  case ADD: {
    printf("+");
    break;
  }
  case MULT: {
    printf("*");
    break;
  }
  case DIV: {
    printf("\n");
    break;
  }
  case SUB: {
    printf("-");
    break;
  }
  }
  if (monkeyptr->op.isSecondOperandSelf) {
    printf(" (old) ");
  } else {
    printf("%zd", monkeyptr->op.operand1);
  }
  printf("]\n");

  printf("\t[Test: ");
  printf("divisor: %ld, ", monkeyptr->test.divisor);
  printf("on true throw to monkey: %zd, ", monkeyptr->test.trueMonkeyId);
  printf("on false throw to monkey: %zd, ", monkeyptr->test.falseMonkeyId);
  printf("]");
  printf("\n");

  printf("\n");
}

void parseStringToTest(char *line, Monkeys *mymonkeys, size_t monkeyid) {
  MonkeyTest *testptr = &mymonkeys->zoo[monkeyid].test;
  if (sscanf(line, " Test: divisible by %ld", &testptr->divisor)) {
  } else if (sscanf(line, " If true: throw to monkey %zd",
                    &testptr->trueMonkeyId)) {
  } else if (sscanf(line, " If false: throw to monkey %zd",
                    &testptr->falseMonkeyId)) {
  }
}

#define FREETHEMONKEYS(_monkeysptr)                                            \
  do {                                                                         \
    free(_monkeysptr);                                                         \
    _monkeysptr = NULL;                                                        \
  } while (0)

Monkeys *readMonkeys(char *filename) {
  Monkeys *mymonkeys =
      calloc(1, sizeof(Monkeys) + sizeof(Monkey) * MONKEYCAPACITY);
  mymonkeys->monkeyCapacity = MONKEYCAPACITY;

  FILE *fh = fopen(filename, "rb");
  assert(filename);
  size_t monkeyid = 0;
  for (;;) {
    char line[1024];
    memset(line, 0x0, sizeof(line));
    if (EOF == fscanf(fh, "%[^\r\n]", line)) {
      break;
    }
    fscanf(fh, "%*[\r\n]");

    // printf("LINE: %s ", line);

    if (sscanf(line, "Monkey %zd:", &monkeyid)) {
      mymonkeys->numMonkeys++;
      mymonkeys->zoo[monkeyid].id = monkeyid;
    }
    size_t item = 0;
    if (sscanf(line, " Starting items: %zd", &item)) {
      addItemToMonkey(mymonkeys, item, monkeyid);
      for (char *linepos = line;;) {
        linepos = strstr(linepos, ",");
        if (!linepos) {
          break;
        }
        linepos++;
        if (!sscanf(linepos, "%zd", &item)) {
          break;
        }
        addItemToMonkey(mymonkeys, item, monkeyid);
      }
    }
    parseStringToOperation(line, &mymonkeys->zoo[mymonkeys->numMonkeys - 1].op);
    parseStringToTest(line, mymonkeys, monkeyid);
    // printf("\n");
  }
  fclose(fh);

  multiplyDivisorsTogether(mymonkeys);
  return mymonkeys;
}

void printAllMonkeys(Monkeys *mymonkeys) {
  printf("There are %zd monkeys:\n", mymonkeys->numMonkeys);
  for (size_t i = 0; i < mymonkeys->numMonkeys; i++) {
    printMonkey(mymonkeys, i);
  }
}

void printItemsHeldByAllMonkeys(Monkeys *mymonkeys) {
  for (size_t i = 0; i < mymonkeys->numMonkeys; i++) {
    printf("[%zd]:", i);
    for (size_t j = 0; j < mymonkeys->zoo[i].numItems; j++) {
      printf(" %zd,", mymonkeys->zoo[i].items[j]);
    }
    printf(" [%zd inspections]", mymonkeys->zoo[i].numInspections);
    printf("\n");
  }
}

void runRound(Monkeys *mymonkeys) {
  for (size_t i = 0; i < mymonkeys->numMonkeys; i++) {
    // printMonkey(mymonkeys, i);
    // printf("AFTER RUNNING ROUND ON MONEY %zd:\n", i);
    monkeyInspectItem(mymonkeys, i);
    // printMonkey(mymonkeys, i);
  }
}

size_t getMonkeyBusiness(Monkeys *mymonkeys) {
  int64_t max[2] = {0};
  size_t ids[2] = {0};

  for (size_t i = 0; i < mymonkeys->numMonkeys; i++) {
    if (mymonkeys->zoo[i].numInspections > max[0]) {
      ids[1] = ids[0];
      max[1] = max[0];
      max[0] = mymonkeys->zoo[i].numInspections;
      ids[0] = i;
    } else if (mymonkeys->zoo[i].numInspections > max[1]) {
      max[1] = mymonkeys->zoo[i].numInspections;
      ids[1] = i;
    }
  }
  return max[0] * max[1];
}

void part1(char *filename) {
  Monkeys *mymonkeys = readMonkeys(filename);
  for (size_t i = 0; i < 20; i++) {
    runRound(mymonkeys);
  }
  printf("Part 1 Monkey Business: %ld\n", getMonkeyBusiness(mymonkeys));
  FREETHEMONKEYS(mymonkeys);
}

void part2(char *filename) {
  Monkeys *mymonkeys = readMonkeys(filename);
  mymonkeys->isPart2 = true;
  for (size_t i = 0; i < 10000; i++) {
    runRound(mymonkeys);
  }
  // printItemsHeldByAllMonkeys(mymonkeys);
  printf("Part 2 Monkey Business: %ld\n", getMonkeyBusiness(mymonkeys));
  FREETHEMONKEYS(mymonkeys);
}

int main(int argc, char **argv) {
  part1(argv[1]);
  printf("\n\n");
  part2(argv[1]);
  return 0;
}