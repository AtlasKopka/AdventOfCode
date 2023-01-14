#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT 702831

typedef struct elem elem;
typedef struct elem {
  uint8_t num;
  elem *prev, *next;
} elem;

typedef struct elems {
  elem *elfpos[2];
  elem *end;
  size_t used;
  elem arena[];
} elems;

void printWorld(elems *world) {
  for (elem *s = world->end->next;; s = s->next) {
    if (world->elfpos[0] == s) {
      printf("(%d)", s->num);
    } else if (world->elfpos[1] == s) {
      printf("[%d]", s->num);
    } else {
      printf("%d", s->num);
    }
    if (s == world->end) {
      break;
    }
    printf(" ");
  }
  printf("\n");
}

void append(elems *world, uint8_t num) {
  elem *n = &world->arena[world->used++];
  n->num = num;
  n->prev = world->end;
  n->next = world->end->next;
  world->end->next->prev = n;
  world->end->next = n;
  world->end = n;

  if (world->used > INPUT && world->used <= INPUT + 10) {
    printf("%hhu", num);
  }
}

bool matchFound(elems *world) {
  uint32_t key = INPUT;
  size_t numDigits = 0;
  size_t trailingDigits = 0;
  elem *e = world->end;
  for (numDigits = 0; key; key /= 10, numDigits++) {
    trailingDigits += e->num * powl(10, numDigits);
    e = e->prev;
  }
  if (trailingDigits == INPUT) {
    printf("[part2]: %d appears after %zd recepies\n", INPUT,
           world->used - numDigits);
    return true;
  }
  return false;
}

int main(int argc, char **argv) {
  elems *world = calloc(sizeof(elems) + sizeof(elem) * 1024 * 1024 * 512, 1);
  elem *s = &world->arena[world->used++];
  s->num = 3;
  world->elfpos[0] = s;
  s->next = &world->arena[world->used++];
  s->next->num = 7;
  world->elfpos[1] = s->next;
  s->prev = s->next;
  s->next->prev = s;
  s->next->next = s;
  world->end = s->next;

  printf("[part1]: ");

  bool part2done = false;
  for (size_t i = world->used; i <= INPUT + 10 || !part2done; i++) {
    // printWorld(world);
    uint32_t sum = world->elfpos[0]->num + world->elfpos[1]->num;
    if (sum >= 10) {
      append(world, sum / 10);
      if (matchFound(world)) {
        part2done = true;
      }
    }
    append(world, sum % 10);
    if (matchFound(world)) {
      part2done = true;
    }

    for (size_t j = 0; j < 2; j++) {
      size_t move = world->elfpos[j]->num + 1;
      for (size_t k = 0; k < move; k++) {
        world->elfpos[j] = world->elfpos[j]->next;
      }
    }

    if (i == INPUT + 10) {
      printf("\n");
    }
  }

  free(world);
  return 0;
}