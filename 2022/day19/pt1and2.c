#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1
#undef DEBUG

typedef struct Blueprint {
  int blueprintIdx;
  int oreRobotOreCost;
  int clayRobotOreCost;
  int obsidianRobotOreCost;
  int obsidianRobotClayCost;
  int geodeRobotOreCost;
  int geodeRobotObsidianCost;
  int maxtime;
} Blueprint;

enum robot { orebot = 1, claybot, obsidianbot, geodebot };

typedef struct State {
  size_t time;
  struct {
    size_t ore, clay, obsidian, geode;
  } minerals;
  struct {
    size_t ore, clay, obsidian, geode;
  } robots;
  enum robot builds[33];
  bool isMax;
  bool pruneme;
} State;

State incrRobots(State s) {
  s.minerals.clay += s.robots.clay;
  s.minerals.geode += s.robots.geode;
  s.minerals.obsidian += s.robots.obsidian;
  s.minerals.ore += s.robots.ore;
  return s;
}

State buildOreRobot(State s, Blueprint b) {
  s.minerals.ore -= b.oreRobotOreCost;
  s.robots.ore++;
  s.builds[s.time] = orebot;
  return s;
}

State buildClayRobot(State s, Blueprint b) {
  s.minerals.ore -= b.clayRobotOreCost;
  s.robots.clay++;
  s.builds[s.time] = claybot;
  return s;
}

State buildObsidianRobot(State s, Blueprint b) {
  s.minerals.ore -= b.obsidianRobotOreCost;
  s.minerals.clay -= b.obsidianRobotClayCost;
  s.robots.obsidian++;
  s.builds[s.time] = obsidianbot;
  return s;
}

State buildGeodeRobot(State s, Blueprint b) {
  s.minerals.ore -= b.geodeRobotOreCost;
  s.minerals.obsidian -= b.geodeRobotObsidianCost;
  s.robots.geode++;
  s.builds[s.time] = geodebot;
  return s;
}

size_t maxGeodes[33] = {0};

void printMinerals(State s) {
  printf(" [(t: %zd) %zd ore, %zd clay, %zd obsidian, %zd geode]", s.time,
         s.minerals.ore, s.minerals.clay, s.minerals.obsidian,
         s.minerals.geode);
}

void printRobots(State s) {
  printf(" [(t: %zd) %zd orebot, %zd claybot, %zd obsidianbot, %zd geodebot]",
         s.time, s.robots.ore, s.robots.clay, s.robots.obsidian,
         s.robots.geode);
}

void printState(State s, Blueprint b) {
  printf("%zd geodes total: \n", s.minerals.geode);
  for (size_t i = 1; i <= b.maxtime; i++) {
    printf("\t%zd", i);
    if (s.builds[i]) {
      switch (s.builds[i]) {
      case (orebot): {
        printf(":ore\t");
        break;
      }
      case (claybot): {
        printf(":clay\t\t");
        break;
      }
      case (obsidianbot): {
        printf(":obsidian\t");
        break;
      }
      case (geodebot): {
        printf(":geode\t");
        break;
      }
      }
    } else {
      printf("\t\t");
    }
    printf("\n");
  }
  printf("\n");
}

size_t getBestPossibleOutcome(State s, Blueprint b) {
  for (; s.time <= b.maxtime; s.time++) {
    s.minerals.geode += s.robots.geode;

    if (s.minerals.obsidian >= b.geodeRobotObsidianCost &&
        s.minerals.ore >= b.geodeRobotOreCost) {
      s.minerals.obsidian -= b.geodeRobotObsidianCost;
      s.minerals.ore -= b.geodeRobotOreCost;
      s.robots.geode++;
    }

    s.minerals.clay += s.robots.clay;
    s.minerals.obsidian += s.robots.obsidian;
    s.minerals.ore += s.robots.ore;

    s.robots.clay++;
    s.robots.obsidian++;
    s.robots.ore++;
  }

  return s.minerals.geode;
}

State calc(Blueprint b, State s) {
  if (s.time == 0) {
    memset(maxGeodes, 0x0, sizeof(maxGeodes));
  }
#ifdef DEBUG
  if (s.time == 0) {
    printf("Blueprint %d:\n", b.blueprintIdx);
    printf("\t%d\n", b.oreRobotOreCost);
    printf("\t%d\n", b.clayRobotOreCost);
    printf("\t%d %d\n", b.obsidianRobotOreCost, b.obsidianRobotClayCost);
    printf("\t%d %d\n", b.geodeRobotOreCost, b.geodeRobotObsidianCost);
  }
#endif
  s.time++;

  if (s.time == b.maxtime) {
    s = incrRobots(s);
    if (s.minerals.geode > maxGeodes[s.time]) {
#ifdef DEBUG
      printf("%zd geodes...\n", s.minerals.geode);
      printState(s, b);
#endif
      maxGeodes[s.time] = s.minerals.geode;
      s.isMax = true;
    }
    return s;
  }

  State incrstate = incrRobots(s);
  if (incrstate.minerals.geode < maxGeodes[incrstate.time]) {
    State blank = {0};
    // blank.pruneme = true;
    // return blank;
  } else {
    maxGeodes[incrstate.time] = incrstate.minerals.geode;
  }

  size_t bestPossibleWorld = getBestPossibleOutcome(incrstate, b);
  if (bestPossibleWorld <= maxGeodes[b.maxtime]) {
    State blank = {0};
    // blank.pruneme = true;
    return blank;
  }

  State best = {0};
  State stateCausingBest = {0};

  if (s.minerals.ore >= b.geodeRobotOreCost &&
      s.minerals.obsidian >= b.geodeRobotObsidianCost) {
    State newstate = buildGeodeRobot(incrstate, b);
    State g = calc(b, newstate);
    if (g.pruneme) {
      return g;
    }
    if (g.minerals.geode > best.minerals.geode) {
      best = g;
      stateCausingBest = newstate;
    }
  }
  if (s.minerals.ore >= b.obsidianRobotOreCost &&
      s.minerals.clay >= b.obsidianRobotClayCost) {
    State newstate = buildObsidianRobot(incrstate, b);
    State g = calc(b, newstate);
    if (g.pruneme) {
      return g;
    }
    if (g.minerals.geode > best.minerals.geode) {
      best = g;
      stateCausingBest = newstate;
    }
  }
  if (s.minerals.ore >= b.clayRobotOreCost) {
    State newstate = buildClayRobot(incrstate, b);
    State g = calc(b, newstate);
    if (g.pruneme) {
      return g;
    }
    if (g.minerals.geode > best.minerals.geode) {
      best = g;
      stateCausingBest = newstate;
    }
  }
  if (s.minerals.ore >= b.oreRobotOreCost) {
    State newstate = buildOreRobot(incrstate, b);
    State g = calc(b, newstate);
    if (g.pruneme) {
      return g;
    }
    if (g.minerals.geode > best.minerals.geode) {
      best = g;
      stateCausingBest = newstate;
    }
  }
  if (1) { // noop
    State g = calc(b, incrstate);
    if (g.pruneme) {
      return g;
    }
    if (g.minerals.geode > best.minerals.geode) {
      best = g;
      stateCausingBest = incrstate;
    }
  }

#ifdef DEBUG
  if (best.isMax) {
    printMinerals(stateCausingBest);
    printRobots(stateCausingBest);
    printf("\n");
  }
#endif

  return best;
}

int main(int argc, char **argv) {
  size_t totalScore = 0;
  uint64_t part2score = 1;
  for (size_t partnum = 1; partnum <= 2; partnum++) {
    FILE *fh = fopen(argv[1], "rb");
    for (;;) {
      Blueprint b = {0};
      if (fscanf(fh, "Blueprint %d:", &b.blueprintIdx) == EOF) {
        break;
      }
      assert(fscanf(fh, " Each ore robot costs %d ore.", &b.oreRobotOreCost));
      assert(fscanf(fh, " Each clay robot costs %d ore.", &b.clayRobotOreCost));
      assert(fscanf(fh, " Each obsidian robot costs %d ore and %d clay.",
                    &b.obsidianRobotOreCost, &b.obsidianRobotClayCost));
      assert(fscanf(fh, " Each geode robot costs %d ore and %d obsidian.",
                    &b.geodeRobotOreCost, &b.geodeRobotObsidianCost));
      fscanf(fh, "%*[ \r\n]");
      State s = {0};
      s.robots.ore = 1;

      if (partnum == 1) {
        b.maxtime = 24;
        State numGeodes = calc(b, s);
        size_t qualityScore = b.blueprintIdx * numGeodes.minerals.geode;
        printf("[part1] blueprint #%d: ", b.blueprintIdx);
        printf(" max geodes: %zd, quality score: %zd\n",
               numGeodes.minerals.geode, qualityScore);
        totalScore += qualityScore;
      } else {
        if (b.blueprintIdx <= 3) {
          b.maxtime = 32;
          State numGeodes = calc(b, s);
          printf("[part2] Max # geodes for blueprint # %d is: %zd\n",
                 b.blueprintIdx, numGeodes.minerals.geode);
          part2score *= numGeodes.minerals.geode;
        }
      }
    }
    fclose(fh);
  }
  printf("[part1]: Total score: %zd\n", totalScore);
  printf("[part2]: Total score: %zd\n", part2score);

  return (0);
}