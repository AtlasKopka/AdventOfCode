#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Valve {
  size_t index;
  char name[2];
  int64_t rate;
  bool isOpen;
  size_t numTunnels;
  size_t tunnels[100];
} Valve;

#define MAXIDX (8192 * 8)

typedef struct Valves {
  size_t num;
  size_t positivePressureValves;
  int64_t maxPressure;
  int64_t sortedValves[100];
  uint16_t cachedState[MAXIDX][27][66][66];
  Valve arr[1024];
} Valves;

typedef struct State {
  int64_t runningScore;
  size_t numValvesOpen;
  size_t numPositivePressureValvesOpen;
  size_t agentLocations[2];
  bool valvesOpen[100];
} State;

int64_t ALLTIMEBESTSCORE = 0;

uint32_t getIdx(State *mystate, Valves *volcano) {
  uint32_t idx = 0;
  for (size_t i = 0; i < volcano->positivePressureValves; i++) {
    if (mystate->valvesOpen[volcano->sortedValves[i]]) {
      idx |= (1 << i);
    }
  }
  return idx;
}

int64_t solve(Valves *volcano, int64_t time, int64_t pressure, State mystate) {
  if (time == 1) {
    return pressure;
  }

  if (mystate.numPositivePressureValvesOpen ==
      volcano->positivePressureValves) {
    return pressure * time;
  }

  // is it possible for us to beat the all-time best score?
  if ((volcano->maxPressure * (time - 1)) + pressure + mystate.runningScore <
      ALLTIMEBESTSCORE) {
    return 0;
  }

  uint32_t idx = getIdx(&mystate, volcano);
  if (idx < MAXIDX) {
    if (volcano->cachedState[idx][time][mystate.agentLocations[0]]
                            [mystate.agentLocations[1]]) {
      return (volcano->cachedState[idx][time][mystate.agentLocations[0]]
                                  [mystate.agentLocations[1]]);
    }
    if (volcano->cachedState[idx][time][mystate.agentLocations[1]]
                            [mystate.agentLocations[0]]) {
      return (volcano->cachedState[idx][time][mystate.agentLocations[1]]
                                  [mystate.agentLocations[0]]);
    }
  }

  // what is the best-case scenario?
  State tmpstate;
  size_t tmpPressure = pressure;
  memcpy(&tmpstate, &mystate, sizeof(State));
  for (size_t j = 0; j < time; j++) {
    tmpstate.runningScore += tmpPressure;
    if (tmpstate.numPositivePressureValvesOpen <
        volcano->positivePressureValves) {
      for (size_t l = 0; l < 2; l++) {
        for (size_t k = 0; k < volcano->num; k++) {
          if (!tmpstate.valvesOpen[volcano->sortedValves[k]]) {
            tmpstate.valvesOpen[volcano->sortedValves[k]] = true;
            tmpPressure += volcano->arr[volcano->sortedValves[k]].rate;
            tmpstate.numPositivePressureValvesOpen++;
            break;
          }
        }
      }
    }
  }
  if (tmpstate.runningScore < ALLTIMEBESTSCORE) {
    return 0;
  }

  int64_t bestScore = 0;

  for (size_t agent = 0; agent < 2; agent++) {
    size_t a = 0;
    size_t b = 1;
    if (agent) {
      a = 1;
      b = 0;
    }

    for (size_t i = 0; i < volcano->arr[mystate.agentLocations[a]].numTunnels;
         i++) {
      for (size_t j = 0; j < volcano->arr[mystate.agentLocations[b]].numTunnels;
           j++) {
        State newstate = {0};
        memcpy(&newstate, &mystate, sizeof(State));
        newstate.runningScore += pressure;

        newstate.agentLocations[a] =
            volcano->arr[mystate.agentLocations[a]].tunnels[i];
        newstate.agentLocations[b] =
            volcano->arr[mystate.agentLocations[b]].tunnels[j];

        int64_t result =
            pressure + solve(volcano, time - 1, pressure, newstate);
        if (result > bestScore) {
          bestScore = result;
        }
      }
    }

    if (volcano->arr[mystate.agentLocations[a]].rate &&
        !mystate.valvesOpen[mystate.agentLocations[a]]) {
      for (size_t i = 0; i < volcano->arr[mystate.agentLocations[b]].numTunnels;
           i++) {
        State newstate = {0};
        memcpy(&newstate, &mystate, sizeof(State));
        newstate.valvesOpen[mystate.agentLocations[a]] = true;
        newstate.numValvesOpen++;
        newstate.numPositivePressureValvesOpen++;
        newstate.runningScore += pressure;

        newstate.agentLocations[b] =
            volcano->arr[mystate.agentLocations[b]].tunnels[i];

        int64_t result =
            pressure +
            solve(volcano, time - 1,
                  pressure + volcano->arr[mystate.agentLocations[a]].rate,
                  newstate);
        if (result > bestScore) {
          bestScore = result;
        }
      }

      if (mystate.agentLocations[a] == mystate.agentLocations[b]) {
        break;
      }

      if (volcano->arr[mystate.agentLocations[b]].rate &&
          !mystate.valvesOpen[mystate.agentLocations[b]]) {
        State newstate = {0};
        memcpy(&newstate, &mystate, sizeof(State));
        newstate.valvesOpen[mystate.agentLocations[a]] = true;
        newstate.valvesOpen[mystate.agentLocations[b]] = true;
        newstate.numValvesOpen += 2;
        newstate.numPositivePressureValvesOpen += 2;
        newstate.runningScore += pressure;
        int64_t newPressure = pressure +
                              volcano->arr[mystate.agentLocations[a]].rate +
                              volcano->arr[mystate.agentLocations[b]].rate;
        int64_t result =
            pressure + solve(volcano, time - 1, newPressure, newstate);
        if (result > bestScore) {
          bestScore = result;
        }
      }
    }
  }

  if (bestScore > ALLTIMEBESTSCORE) {
    ALLTIMEBESTSCORE = bestScore;
    printf("%ld\n", ALLTIMEBESTSCORE);
  }

  idx = getIdx(&mystate, volcano);
  if (idx < MAXIDX) {
    volcano->cachedState[idx][time][mystate.agentLocations[0]]
                        [mystate.agentLocations[1]] = bestScore;
  }

  return bestScore;
}

int main(int argc, char **argv) {
  Valves *myvalves = calloc(sizeof(Valves), 1);

  FILE *fh = fopen(argv[1], "rb");
  char targets[100][100][2] = {0};
  for (;;) {
    size_t numTargets = 0;
    char source[100] = {0};
    int64_t rate;
    char dest[100] = {0};
    if (fscanf(fh,
               "Valve %[A-Z] has flow rate=%ld; %*[a-z] %*[a-z] to %*[a-z] "
               "%[^\r\n]",
               source, &rate, dest) != 3) {
      break;
    }
    fscanf(fh, "%*[\r\n]");
    // printf("SOURCE: %s, RATE: %ld, DEST:\n", source, rate);
    for (size_t i = 0; i < strlen(dest); i++) {
      if (!sscanf(dest + i, "%[A-Z]", targets[myvalves->num][numTargets])) {
        break;
      }
      i += 3;
      numTargets++;
    }
    for (size_t i = 0; i < numTargets; i++) {
      // printf("\t%.2s\n", targets[myvalves->num][i]);
    }

    Valve *myvalve = &myvalves->arr[myvalves->num];
    myvalve->index = myvalves->num;
    memcpy(myvalve->name, source, 2);
    myvalve->rate = rate;
    myvalve->numTunnels = numTargets;
    myvalves->num++;
  }
  fclose(fh);

  for (size_t i = 0; i < myvalves->num; i++) {
    for (size_t j = 0; j < myvalves->arr[i].numTunnels; j++) {
      // get index of tunnel:
      for (size_t k = 0; k < myvalves->num; k++) {
        if (memcmp(targets[i][j], myvalves->arr[k].name, 2) == 0) {
          myvalves->arr[i].tunnels[j] = k;
          break;
        }
      }
    }
  }

  printf("There are %ld valves: \n", myvalves->num);

  /*
    for (size_t i = 0; i < myvalves->num; i++) {
      Valve *tunnel = &myvalves->arr[i];
      printf("[%zd]: %.2s, rate: %ld\n", tunnel->index, tunnel->name,
             tunnel->rate);
      for (size_t j = 0; j < myvalves->arr[i].numTunnels; j++) {
        printf("\t[Tunnel ID %zd]: %.2s\n", tunnel->tunnels[j],
               myvalves->arr[tunnel->tunnels[j]].name);
      }
    }
  */

  // find index of starting valve:
  size_t startingIndex;
  for (startingIndex = 0; startingIndex < myvalves->num; startingIndex++) {
    if (memcmp("AA", myvalves->arr[startingIndex].name, 2) == 0) {
      break;
    }
  }

  size_t numValidValves = 0;
  // find number of valves with posivie pressure:
  for (size_t i = 0; i < myvalves->num; i++) {
    myvalves->maxPressure += myvalves->arr[i].rate;
    if (myvalves->arr[i].rate) {
      numValidValves++;
    }
  }
  myvalves->positivePressureValves = numValidValves;
  printf("There are %ld valves with positive pressure\n", numValidValves);

  for (int64_t i = 0; i < myvalves->num; i++) {
    myvalves->sortedValves[i] = i;
  }

  for (int64_t i = 0; i < myvalves->num; i++) {
    for (int64_t j = i + 1; j < myvalves->num; j++) {
      if (myvalves->arr[myvalves->sortedValves[i]].rate <
          myvalves->arr[myvalves->sortedValves[j]].rate) {
        int64_t tmp = myvalves->sortedValves[i];
        myvalves->sortedValves[i] = myvalves->sortedValves[j];
        myvalves->sortedValves[j] = tmp;
      }
    }
  }

  printf("Sorted rates: \n");
  for (size_t i = 0; i < myvalves->num; i++) {
    State mystate = {0};
    mystate.valvesOpen[myvalves->sortedValves[i]] = 1;
    uint32_t idx = getIdx(&mystate, myvalves);
    printf("%ld: %ld (idx: %d)\n", myvalves->sortedValves[i],
           myvalves->arr[myvalves->sortedValves[i]].rate, idx);
    for (size_t j = 0; j < myvalves->num; j++) {
      memset(&mystate, 0x0, sizeof(State));
      mystate.valvesOpen[myvalves->sortedValves[i]] = true;
      mystate.valvesOpen[myvalves->sortedValves[j]] = true;
      idx = getIdx(&mystate, myvalves);

      printf("Valve %ld + %ld = idx %d\n", myvalves->sortedValves[i],
             myvalves->sortedValves[j], idx);
    }
  }

  printf("Starting index: %ld\n", startingIndex);
  int64_t time = 26, pressure = 0;
  State mystate = {0};
  mystate.agentLocations[0] = startingIndex;
  mystate.agentLocations[1] = startingIndex;
  int64_t result = solve(myvalves, time, pressure, mystate);
  printf("Highest pressure: %ld\n", result);

  free(myvalves);
  return 0;
}