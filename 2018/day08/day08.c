#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int pt1(int32_t *data, size_t *pos, size_t numData) {
  size_t numNodes = data[(*pos)++];
  size_t numMetadata = data[(*pos)++];

  int sum = 0;
  for (size_t i = 0; i < numNodes; i++) {
    sum += pt1(data, pos, numData);
  }

  for (size_t i = 0; i < numMetadata; i++) {
    sum += data[(*pos)++];
  }
  return sum;
}

int pt2(int32_t *data, size_t *pos, size_t numData) {
  size_t numNodes = data[(*pos)++];
  size_t numMetadata = data[(*pos)++];

  int sum = 0;
  size_t startOfSubNodes = *pos;
  for (size_t i = 0; i < numNodes; i++) {
    pt2(data, pos, numData);
  }

  for (size_t i = 0; i < numMetadata; i++) {
    int32_t metadata = data[(*pos)++];
    if (!numNodes) {
      sum += metadata;
      continue;
    }
    if (metadata > numNodes) {
      continue;
    }
    size_t nodePos = startOfSubNodes;
    for (size_t j = 0; j < metadata; j++) {
      int tmpsum = pt2(data, &nodePos, numData);
      if (j == metadata - 1) {
        sum += tmpsum;
      }
    }
  }
  return sum;
}

int main(int argc, char **argv) {
  int32_t *data = calloc(1024 * 1024, sizeof(int32_t));
  size_t numData = 0;
  FILE *fh = fopen(argv[1], "rb");
  for (;;) {
    if (fscanf(fh, "%d", data + numData) == EOF) {
      break;
    }
    if (fscanf(fh, "%*[ ]") == EOF) {
      break;
    }
    numData++;
  }
  fclose(fh);
  size_t startPos = 0;
  printf("[part1]: %d\n", pt1(data, &startPos, numData));
  startPos = 0;
  printf("[part2]: %d\n", pt2(data, &startPos, numData));
  free(data);
  return 0;
}