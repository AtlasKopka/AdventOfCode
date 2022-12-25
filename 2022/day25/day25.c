#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int64_t convertToDecimal(char *snafu) {
  int64_t sum = 0;
  size_t numDigits = strlen(snafu);
  for (size_t i = 1; i <= numDigits; i++) {
    size_t placeValue = powl(5, (numDigits - i));
    switch (snafu[i - 1]) {
    case '2': {
      sum += 2 * placeValue;
      break;
    }
    case '1': {
      sum += placeValue;
      break;
    }
    case '0': {
      break;
    }
    case '-': {
      sum -= placeValue;
      break;
    }
    case '=': {
      sum -= placeValue * 2;
      break;
    }
    }
  }
  return sum;
}

typedef struct SNAFU {
  bool found;
  int64_t decimalTarget;
  size_t numPowers;
  size_t currentPosition;
  char num[25];
} SNAFU;

char SNAFU_POSITIONS[] = {'2', '1', '0', '-', '='};

int64_t getMax(char *prefix, size_t length) {
  char buf[25] = {0};
  strcpy(buf, prefix);
  size_t curlen = strlen(buf);
  for (size_t i = curlen; i <= length; i++) {
    buf[i] = '2';
  }
  return convertToDecimal(buf);
}

int64_t getMin(char *prefix, size_t length) {
  char buf[25] = {0};
  strcpy(buf, prefix);
  size_t curlen = strlen(buf);
  for (size_t i = curlen; i <= length; i++) {
    buf[i] = '=';
  }
  return convertToDecimal(buf);
}

SNAFU findSnafu(SNAFU target) {
  int64_t curmax = getMax(target.num, target.numPowers);
  if (curmax < target.decimalTarget) {
    target.found = false;
    return target;
  }
  int64_t curmin = getMin(target.num, target.numPowers);
  if (curmin > target.decimalTarget) {
    target.found = false;
    return target;
  }

  if (target.currentPosition == target.numPowers) {
    for (size_t i = 0; i < 5; i++) {
      target.num[target.numPowers] = SNAFU_POSITIONS[i];
      if (convertToDecimal(target.num) == target.decimalTarget) {
        target.found = true;
        return target;
      }
    }
    return target;
  }

  size_t currentPosition = target.currentPosition;
  target.currentPosition++;
  for (size_t i = 0; i < 5; i++) {
    target.num[currentPosition] = SNAFU_POSITIONS[i];
    SNAFU result = findSnafu(target);
    if (result.found) {
      return result;
    }
  }
  return target;
}

char *convertToSnafu(int64_t decimal, char *buf) {
  for (size_t numPowers = 0; numPowers < 25; numPowers++) {
    SNAFU target = {0};
    target.decimalTarget = decimal;
    target.numPowers = numPowers;
    target.currentPosition = 0;
    SNAFU result = findSnafu(target);
    if (result.found) {
      strcpy(buf, result.num);
      break;
    }
  }

  return buf;
}

int main(int argc, char **argv) {
  FILE *fh = fopen(argv[1], "rb");
  int64_t total = 0;
  for (;;) {
    char line[25] = {0};
    if (fscanf(fh, "%[^\r\n]", line) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    int64_t conversion = convertToDecimal(line);
    total += conversion;
    assert(total < INT64_MAX);
  }
  char buf[25] = {0};
  printf("[part1]: %s in SNAFU (%ld decimal)\n", convertToSnafu(total, buf),
         total);
  fclose(fh);
  return 0;
}