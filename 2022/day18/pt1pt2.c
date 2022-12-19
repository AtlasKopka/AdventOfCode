#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum airPocketStatus {
  unknownIfAirPocket,
  airPocket,
  notAirPocket,
  visited
} airPocketStatus;

typedef struct Pos {
  union {
    struct {
      uint8_t x;
      uint8_t y;
      uint8_t z;
    };
    struct {
      uint8_t coords[3];
    };
  };
  bool rock;
  airPocketStatus status;
  uint64_t alreadyAsked;
} Pos;

typedef struct Positions {
  size_t numPositions;
  uint8_t max[3];
  Pos arr[];
} Positions;

Pos *getCube(Positions *map, Pos *cube) {
  return &map->arr[cube->y * (map->max[0] + 1) * (map->max[2] + 1) +
                   cube->z * (map->max[0] + 1) + cube->x];
}

Positions *initPositions(uint8_t *max) {
  size_t numElems = 1;
  for (size_t i = 0; i < 3; i++) {
    numElems *= (size_t)(max[i] + 1);
  }
  size_t size = sizeof(Positions) + sizeof(Pos) * numElems;
  printf("Allocating buffer with %zd elements with total size: %zd bytes\n",
         numElems, size);

  Positions *p = calloc(1, size);
  memcpy(p->max, max, sizeof(p->max[0]) * 3);
  assert(p);

  for (size_t x = 0; x <= p->max[0]; x++) {
    for (size_t y = 0; y <= p->max[1]; y++) {
      for (size_t z = 0; z <= p->max[2]; z++) {
        Pos query = {.x = x, .y = y, .z = z, .status = unknownIfAirPocket};
        Pos *cube = getCube(p, &query);
        memcpy(cube, &query, sizeof(Pos));
      }
    }
  }
  return p;
}

void printPos(Pos *p) {
  printf("[%d,%d,%d (%s)]", p->x, p->y, p->z, p->rock ? "rock" : "empty");
}

size_t getAdjacencies(Pos *p, Pos *adjacencies, Positions *map) {
  size_t cubenum = 0;
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < 2; j++) {
      int coord = j ? (int)p->coords[i] - 1 : (int)p->coords[i] + 1;
      if (coord < 0) {
        continue;
      }
      if (coord > map->max[i]) {
        continue;
      }
      memcpy(adjacencies + cubenum, p, sizeof(Pos));
      adjacencies[cubenum].coords[i] = j ? p->coords[i] - 1 : p->coords[i] + 1;
      cubenum++;
    }
  }
  return cubenum;
}

int countNonAdjacentSides(Pos *p, Positions *map) {
  Pos adjacencies[6] = {0};
  size_t numAdjacencies = getAdjacencies(p, adjacencies, map);
  int adjacentSides = 0;
  for (size_t i = 0; i < numAdjacencies; i++) {
    if (getCube(map, &adjacencies[i])->rock) {
      adjacentSides++;
    }
  }
  return 6 - adjacentSides;
}

void getMinMax(char *filename, uint8_t *min, uint8_t *max) {
  FILE *fh = fopen(filename, "rb");
  memset(min, 0x0, sizeof(uint8_t) * 3);
  memset(max, 0x0, sizeof(uint8_t) * 3);
  for (;;) {
    Pos p = {0};
    if (fscanf(fh, "%hhu,%hhu,%hhu", &p.x, &p.y, &p.z) != 3) {
      break;
    }
    for (size_t i = 0; i < 3; i++) {
      if (p.coords[i] > max[i]) {
        max[i] = p.coords[i];
      } else if (p.coords[i] < min[i]) {
        min[i] = p.coords[i];
      }
    }
  }
  for (size_t i = 0; i < 3; i++) {
    printf("Min[%s]: %d\n", i == 0 ? "x" : i == 1 ? "y" : "z", min[i]);
  }
  for (size_t i = 0; i < 3; i++) {
    printf("Max[%s]: %d\n", i == 0 ? "x" : i == 1 ? "y" : "z", max[i]);
  }

  fclose(fh);
}

bool checkAirPocket(Pos *cube, Positions *map) {
  if (cube->status != unknownIfAirPocket && cube->status != visited) {
    return cube->status == airPocket;
  }

  if (cube->rock) {
    cube->status = airPocket;
    return true;
  }

  for (size_t i = 0; i < 3; i++) {
    if (cube->coords[i] == map->max[i] || cube->coords[i] == 0) {
      cube->status = notAirPocket;
      return false;
    }
  }

  cube->status = visited;
  if (cube->alreadyAsked++ > 9) {
    return true;
  }

  Pos adjacencies[6] = {0};
  size_t numAdjacencies = getAdjacencies(cube, adjacencies, map);
  for (size_t i = 0; i < numAdjacencies; i++) {
    Pos *adj = getCube(map, &adjacencies[i]);
    if (1 || adj->status != visited) {
      if (!checkAirPocket(adj, map)) {
        cube->status = notAirPocket;
        return false;
      }
    }
  }

  cube->status = airPocket;
  return true;
}

int countNonAdjacentSidesCheckAirPocket(Pos *p, Positions *map) {
  Pos adjacencies[6] = {0};
  size_t numAdjacencies = getAdjacencies(p, adjacencies, map);
  int adjacentSides = 0;
  for (size_t i = 0; i < numAdjacencies; i++) {
    Pos *adjacentCube = getCube(map, &adjacencies[i]);
    if (checkAirPocket(adjacentCube, map)) {
      adjacentSides++;
    }
  }
  return 6 - adjacentSides;
}

int main(int argc, char **argv) {
  uint8_t min[3], max[3];
  getMinMax(argv[1], min, max);

  Positions *map = initPositions(max);
  FILE *fh = fopen(argv[1], "rb");
  for (;;) {
    Pos p = {0};
    if (fscanf(fh, "%hhu,%hhu,%hhu", &p.x, &p.y, &p.z) != 3) {
      break;
    }
    p.rock = true;
    memcpy(getCube(map, &p), &p, sizeof(Pos));
    map->numPositions++;
  }
  fclose(fh);

  printf("Found %zd cubes of rock\n", map->numPositions);

  int64_t area = 0;
  for (size_t x = 0; x <= map->max[0]; x++) {
    for (size_t y = 0; y <= map->max[1]; y++) {
      for (size_t z = 0; z <= map->max[2]; z++) {
        Pos p = {.x = x, .y = y, .z = z};
        Pos *cube = getCube(map, &p);
        if (cube->rock) {
          area += countNonAdjacentSides(cube, map);
        }
      }
    }
  }
  printf("Area: %ld before excluding air pockets\n", area);

  area = 0;
  for (size_t x = 0; x <= map->max[0]; x++) {
    for (size_t y = 0; y <= map->max[1]; y++) {
      for (size_t z = 0; z <= map->max[2]; z++) {
        Pos p = {.x = x, .y = y, .z = z};
        Pos *cube = getCube(map, &p);
        if (cube->rock) {
          area += countNonAdjacentSidesCheckAirPocket(cube, map);
        }
      }
    }
  }
  printf("Area: %ld after excluding air pockets\n", area);

  free(map);
  return 0;
}