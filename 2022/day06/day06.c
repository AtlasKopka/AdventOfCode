#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

int findEndOfFrame(char *buf, size_t frameLength) {
  for (size_t i = frameLength-1; i < strlen(buf); i++) {
    char matcher[1024];
    memset(matcher, 0x0, sizeof(matcher));
    for (size_t j = 0; j < frameLength; j++) {
      matcher[(int32_t)buf[i - j]]++;
    }
    bool found = true;
    for (size_t l = 0; l < sizeof(matcher); l++) {
      if (matcher[l] > 1) {
        found = false;
        break;
      }
    }
    if(found) {
      return i + 1;
    }
  }
  assert(false);
  return 0;
}

int main(int argc, char **argv) {
  assert(argc == 2);
  FILE *fh = fopen(argv[1],"r");
  assert(fh);
  for (;;) {
    char buf[1024 * 1024];
    memset(buf, 0x0, sizeof(buf));
    if(EOF == fscanf(fh, "%[a-zA-Z]", buf)) {
      break;
    }
    fscanf(fh, "%*[\r\n ]");
    printf("start-of-frame: %d, start-of-message: %d\n", findEndOfFrame(buf,4), findEndOfFrame(buf,14));
  }
  fclose(fh);
  return 0;
}