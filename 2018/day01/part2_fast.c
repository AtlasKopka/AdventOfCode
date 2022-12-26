#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void part1(int64_t *nums, size_t size) {
  int64_t sum = 0;
  for (size_t i = 0; i < size; i++) {
    sum += nums[i];
  }
  printf("[part1]: %ld\n", sum);
}

typedef struct entry {
  int64_t num;
  size_t count;
  bool exists;
} entry;

size_t incrCount(int64_t num, entry *entries) {
  for (size_t i = 0;; i++) {
    if (!entries[i].exists) {
      entries[i].num = num;
      entries[i].exists = true;
      entries[i].count = 1;
      if (i && i % 5000 == 0) {
        printf("added %ld (%ld entries)\n", num, i);
      }
      return 1;
    }
    if (entries[i].num == num) {
      entries[i].count++;
      return entries[i].count;
    }
  }
}

void part2(int64_t *nums, size_t size) {
  entry *entries = calloc(sizeof(entry), 1024 * 1024);

  int64_t sum = 0;
  for (;;) {
    for (size_t i = 0; i < size; i++) {
      sum += nums[i];
      if (incrCount(sum, entries) == 2) {
        printf("[part2]: %ld is the first sum to appear twice.\n", sum);
        goto done;
      }
    }
  }
done:
  free(entries);
}

int main(int argc, char **argv) {
  int64_t nums[1024] = {0};
  FILE *fh = fopen(argv[1], "rb");
  size_t size = 0;
  for (size = 0;; size++) {
    int64_t num = 0;
    if (fscanf(fh, "%ld", &num) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    nums[size] = num;
  }
  fclose(fh);
  part1(nums, size);
  part2(nums, size);
  return 0;
}