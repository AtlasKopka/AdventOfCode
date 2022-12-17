#include <assert.h>
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
  int64_t cachedScores[31];
} Valve;

typedef struct Valves {
  size_t num;
  size_t positivePressureValves;
  int64_t maxPressure;
  int64_t sortedValves[100];
  Valve arr[1024];
} Valves;

typedef struct State {
  int64_t runningScore;
  size_t numValvesOpen;
  size_t numPositivePressureValvesOpen;
  bool valvesOpen[100];
} State;

int64_t ALLTIMEBESTSCORE = 0;

int64_t solve(Valve *v, Valves *volcano, int64_t time, int64_t pressure,
              State mystate) {
  if (time == 1) {
    return pressure;
  }

  if (mystate.numPositivePressureValvesOpen == 0 && v->cachedScores[time]) {
    return v->cachedScores[time];
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

  // what is the best-case scenario?
  State tmpstate;
  size_t tmpPressure = pressure;
  memcpy(&tmpstate, &mystate, sizeof(State));
  for (size_t j = 0; j < time; j++) {
    tmpstate.runningScore += tmpPressure;
    if (tmpstate.numPositivePressureValvesOpen <
        volcano->positivePressureValves) {
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
  if (tmpstate.runningScore < ALLTIMEBESTSCORE) {
    return 0;
  }

  int64_t bestScore = 0;

  if (!mystate.valvesOpen[v->index] && v->rate) {
    State newstate = {0};
    memcpy(&newstate, &mystate, sizeof(State));
    newstate.valvesOpen[v->index] = true;
    newstate.numValvesOpen++;
    newstate.runningScore += pressure;
    if (v->rate) {
      newstate.numPositivePressureValvesOpen++;
    }
    int64_t result =
        pressure + solve(v, volcano, time - 1, pressure + v->rate, newstate);
    if (result > bestScore) {
      bestScore = result;
    }
  }

  for (size_t i = 0; i < v->numTunnels; i++) {
    State newstate = {0};
    memcpy(&newstate, &mystate, sizeof(State));
    newstate.runningScore += pressure;
    int64_t result = pressure + solve(&volcano->arr[v->tunnels[i]], volcano,
                                      time - 1, pressure, newstate);
    if (result > bestScore) {
      bestScore = result;
    }
  }

  if (bestScore > ALLTIMEBESTSCORE) {
    ALLTIMEBESTSCORE = bestScore;
    printf("%ld\n", ALLTIMEBESTSCORE);
  }

  if (mystate.numPositivePressureValvesOpen == 0) {
    v->cachedScores[time] = bestScore;
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
    printf("%ld: %ld\n", myvalves->sortedValves[i],
           myvalves->arr[myvalves->sortedValves[i]].rate);
  }

  printf("Starting index: %ld\n", startingIndex);
  Valve *startingValve = &myvalves->arr[startingIndex];
  int64_t time = 30, pressure = 0;
  State mystate = {0};
  int64_t result = solve(startingValve, myvalves, time, pressure, mystate);
  printf("Highest pressure: %ld\n", result);

  free(myvalves);
  return 0;
}