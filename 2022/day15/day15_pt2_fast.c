#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  int64_t sensors[1024][3] = {0};
  FILE *fh = fopen(argv[1], "rb");
  size_t sensor = 0;
  int64_t beaconx = 0, beacony = 0;
  while (fscanf(fh,
                "Sensor at x=%ld, y=%ld: closest beacon is at x=%ld, y=%ld\n",
                &sensors[sensor][0], &sensors[sensor][1], &beaconx,
                &beacony) == 4) {
    int64_t dist =
        labs(beaconx - sensors[sensor][0]) + labs(beacony - sensors[sensor][1]);
    sensors[sensor][2] = dist;
    sensor++;
  }
  fclose(fh);

  int64_t qx, qy, score;
  for (size_t i = 0; i < sensor; i++) {
    for (size_t j = i + 1; j < sensor; j++) {
      if (labs(sensors[i][0] - sensors[j][0]) +
              labs(sensors[i][1] - sensors[j][1]) ==
          sensors[i][2] + sensors[j][2] + 2) {

        int64_t x1 = sensors[i][0];
        if (sensors[j][0] < sensors[i][0]) {
          x1 -= (sensors[i][2] + 1);
        } else {
          x1 += (sensors[i][2] + 1);
        }
        int64_t y1 = sensors[i][1];
        int64_t x2 = sensors[i][0];
        int64_t y2 = sensors[i][1];
        if (sensors[j][1] > sensors[i][1]) {
          y2 += (sensors[i][2] + 1);
        } else {
          y2 -= (sensors[i][2] + 1);
        }

        // for every point between the two sensors:
        int64_t dx = 1, dy = 1;
        if (x2 < x1) {
          dx = -1;
        }
        if (y2 < y1) {
          dy = -1;
        }

        for (qx = x1, qy = y1; qx != x2 && qy != y2; qx += dx, qy += dy) {
          bool found = true;
          for (size_t k = 0; k < sensor; k++) {
            if (labs(sensors[k][0] - qx) + labs(sensors[k][1] - qy) <=
                sensors[k][2]) {
              found = false;
              break;
            }
          }
          if (found) {
            goto done;
          }
        }
      }
    }
  }
  return 0;
done:

  score = (4000000 * qx) + qy;
  printf("part 2: %ld\n", score);
  return 0;
}