#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

bool isContained(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1) {
    return (a0 >= b0 && a0 <= b1 && a1 >= b0 && a1 <= b1);
}
bool isPartiallyContained(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1) {
    return ((a0 >= b0 && a0 <= b1) || (a1 >= b0 && a1 <= b1));
}


int main(int argc, char** argv) {
    assert(argc == 2);
    FILE *fh = fopen(argv[1],"rb");
    assert(fh);
    char buf[1024 * 1024];
    memset(buf, 0x0, sizeof(buf));
    fread(buf, 1024, 1024, fh);
    fclose(fh);
    size_t pos = 0;
    uint64_t part0score = 0;
    uint64_t part1score = 0;
    for (;;)
    {
        char line[1024];
        memset(line, 0x0, sizeof(line));
        if (sscanf(buf + pos, "%s ", line) <= 0)
        {
            break;
        }
        uint64_t a0, a1, b0, b1;
        sscanf(line, "%zd-%zd,%zd-%zd", &a0, &a1, &b0, &b1);
        pos += strlen(line) + 1;
        if(isContained(a0,a1,b0,b1) || isContained(b0,b1,a0,a1)) {
            part0score++;
        }
        if(isPartiallyContained(a0,a1,b0,b1) || isPartiallyContained(b0,b1,a0,a1)) {
            part1score++;
        }
    }
    printf("[part 0]: %zd\n", part0score);
    printf("[part 1]: %zd\n", part1score);
    return 0;
}