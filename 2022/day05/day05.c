#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void printStacks(char *columns, size_t stride, size_t *numberOfRows,
                 size_t maxColumns) {
  for (size_t row = stride - *numberOfRows; row < stride; row++) {
    for (size_t column = 0; column < maxColumns; column++) {
      printf("[%zd][%zd]: %c ", row, column, columns[row * stride + column]);
    }
    printf("\n");
  }
}

void reverseCrates(char *columns, size_t target, size_t numcrates,
                   size_t stride, size_t *numberOfRows, size_t maxColumns) {
  for (size_t rownum = 0; rownum < stride; rownum++) {
    if (columns[rownum * stride + target] == 0x0) {
      continue;
    }
    for (size_t i = 0; i < numcrates / 2; i++) {
      char *a = columns + (rownum + i) * stride + target;
      char *b = columns + (rownum + numcrates - i - 1) * stride + target;
      char tmp = *a;
      *a = *b;
      *b = tmp;
    }
    break;
  }
}

void doMove(size_t numcrates, size_t source, size_t target, char *columns,
            size_t stride, size_t *numberOfRows, size_t maxColumns) {
  // printf("Moving 1 crate from %zd to %zd\n", source, target);
  size_t sourcerownum = 0;
  while (columns[sourcerownum * stride + source] == 0x0) {
    sourcerownum++;
  }
  // printf("Source row num: %zd\n", sourcerownum);

  size_t targetrownum = 0;
  while (columns[targetrownum * stride + target] == 0x0) {
    targetrownum++;
    if (targetrownum == stride) {
      break;
    }
  }
  targetrownum--;
  if (stride - targetrownum > *numberOfRows) {
    *numberOfRows = stride - targetrownum;
  }
  // printf("Target rownum: %zd\n", targetrownum);

  // printf("Sourcerownum: %zd\t targetrownum: %zd\n", sourcerownum,
  // targetrownum);
  columns[targetrownum * stride + target] =
      columns[sourcerownum * stride + source];

  columns[sourcerownum * stride + source] = 0x0;

  // printStacks(columns, stride, numberOfRows, maxColumns);

  if (numcrates > 1) {
    doMove(numcrates - 1, source, target, columns, stride, numberOfRows,
           maxColumns);
  }
}

void printTopCrates(char *columns, size_t maxColumns, size_t stride) {
  for (size_t column = 0; column < maxColumns; column++) {
    size_t rownum = 0;
    for (; columns[rownum * stride + column] == 0x0; rownum++) {
    }
    printf("%c", columns[rownum * stride + column]);
  }
}

void runcrane(char *buf) {
  size_t numberOfRows = 0;
  size_t maxColumns = 0;

  // parse header:
  size_t stride = 1024;
  char columns[1024 * 1024];
  char part2columns[1024 * 1024];
  memset(columns, 0x0, sizeof(columns));
  size_t pos = 0;
  size_t rownum = 0;
  for (;;) {
    char line[1024];
    memset(line, 0x0, sizeof(line));
    sscanf(buf + pos, "%[^\n]*", line);
    pos += strlen(line) + 1;
    if (line[1] >= '0' && line[1] <= '9') {
      break;
    }
    size_t columnnum = 0;
    for (size_t barrelchar = 1; barrelchar < strlen(line); barrelchar += 4) {
      if ((line[barrelchar] >= 'A' && line[barrelchar] <= 'Z') ||
          (line[barrelchar] >= 'a' && line[barrelchar] <= 'z')) {
        columns[rownum * stride + columnnum] = line[barrelchar];
      }
      columnnum++;
      if (columnnum > maxColumns) {
        maxColumns = columnnum;
      }
    }
    rownum++;
    if (rownum > numberOfRows) {
      numberOfRows++;
    }
  }

  // finished parsing header

  // shift everything down:
  size_t blocksize = numberOfRows * stride;
  memmove(columns + (sizeof(columns) - blocksize), columns, blocksize);
  memset(columns, 0x0, blocksize);

  size_t numberOfRowsPt2 = numberOfRows;
  memcpy(part2columns, columns, sizeof(columns));

  // parse instructions:
  bool done = false;
  for (;;) {
    while (buf[pos] != 'm' || buf[pos + 1] != 'o') {
      pos++;
      if (pos >= strlen(buf)) {
        done = true;
        break;
      }
    }
    if (done) {
      break;
    }

    size_t numCrates = 0, source = 0, target = 0;
    if (!sscanf(buf + pos, "move %zd from %zd to %zd", &numCrates, &source,
                &target)) {
      break;
    }
    pos++;

    source--;
    target--;
    // printf("[MOVE %zd FROM %zd TO %zd]\n", numCrates, source, target);
    doMove(numCrates, source, target, columns, stride, &numberOfRows,
           maxColumns);

    doMove(numCrates, source, target, part2columns, stride, &numberOfRowsPt2,
           maxColumns);
    reverseCrates(part2columns, target, numCrates, stride, &numberOfRowsPt2,
                  maxColumns);
  }

  printf("Number of rows: %zd\n", numberOfRows);
  printf("Number of columns: %zd\n", maxColumns);

  printf("[PART1]: ");
  printTopCrates(columns, maxColumns, stride);
  printf("\n");
  printf("[PART2]: ");
  printTopCrates(part2columns, maxColumns, stride);
  printf("\n");
}

int main(int argc, char **argv) {
  assert(argc == 2);
  char buf[1024 * 1024];
  memset(buf, 0x0, sizeof(buf));
  FILE *fh = fopen(argv[1], "rb");
  assert(fh);
  fread(buf, sizeof(buf), 1, fh);
  fclose(fh);
  runcrane(buf);
  return 0;
}