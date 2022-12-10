#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STARTCYCLE 20
#define CYCLEINCREMENT 40
#define NUMCYCLES 6

typedef enum operationType { NOOP, ADD } operationType;

typedef struct instruction {
  int64_t operand;
  operationType type;
  size_t numCyclesRemaining;
  size_t registernum;
} instruction;

typedef struct program {
  size_t numInstruction;
  instruction instructions[1024 * 1024];
  size_t nextInstruction;
} program;

instruction *fetch(program *myprogram) {
  if (myprogram->nextInstruction < myprogram->numInstruction) {
    return &myprogram->instructions[myprogram->nextInstruction++];
  }
  return NULL;
}

void addInstruction(program *myprogram, char *instruction) {
  if (instruction[0] == 'n') {
    myprogram->instructions[myprogram->numInstruction].type = NOOP;
    myprogram->instructions[myprogram->numInstruction].numCyclesRemaining = 1;
  } else {
    myprogram->instructions[myprogram->numInstruction].type = ADD;
    myprogram->instructions[myprogram->numInstruction].numCyclesRemaining = 2;
    assert(sscanf(instruction, "addx %ld",
                  &myprogram->instructions[myprogram->numInstruction].operand));
  }
  myprogram->numInstruction++;
}

bool execute(instruction *myinstruction, int64_t *registers) {
  myinstruction->numCyclesRemaining--;
  if (myinstruction->numCyclesRemaining) {
    return true;
  }
  if (myinstruction->type == ADD) {
    registers[myinstruction->registernum] += myinstruction->operand;
  }
  return false;
}

bool isCycleInteresting(size_t currentCycle) {
  if (((ssize_t)currentCycle - STARTCYCLE) % CYCLEINCREMENT == 0) {
    return true;
  }
  return false;
}

void runProgram(program *myprogram) {
  int64_t totalScore = 0;
  size_t currentCycle = 1;
  size_t crtxpos = currentCycle;
  int64_t registers[10] = {[0] = 1};

  for (;;) {
    instruction *currentInstruction = fetch(myprogram);
    if (!currentInstruction) {
      break;
    }

    for (;;) {
      if (crtxpos >= registers[0] && crtxpos <= (registers[0] + 2)) {
        printf("#");
      } else {
        printf(".");
      }
      bool moreToExecute = execute(currentInstruction, registers);

      if (isCycleInteresting(currentCycle)) {
        int64_t score = registers[0] * currentCycle;
        totalScore += score;
      }

      currentCycle++;
      crtxpos = (currentCycle - 1) % CYCLEINCREMENT + 1;
      if (crtxpos == 1) {
        printf("\n");
      }

      if (!moreToExecute) {
        break;
      }
    }
  }
  printf("[part1]: %ld\n", totalScore);
}

int main(int argc, char **argv) {
  program *myprogram = calloc(sizeof(program), 1);
  assert(myprogram);

  FILE *fh = fopen(argv[1], "rb");
  assert(fh);
  for (;;) {
    char instruction[20];
    memset(instruction, 0x0, sizeof(instruction));
    if (EOF == fscanf(fh, "%[^\r\n]", instruction)) {
      break;
    }
    fscanf(fh, "%*[\r\n ]");
    addInstruction(myprogram, instruction);
  }

  runProgram(myprogram);

  fclose(fh);
  free(myprogram);
  return 0;
}