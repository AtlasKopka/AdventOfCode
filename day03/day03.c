#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

uint64_t getPriority(char item) {
    return item < 'a' ? item - 'A' + 27 : item - 'a' + 1;
}

void part1(int argc, char** argv) {
    assert(argc == 2);
    FILE *fh = fopen(argv[1],"rb");
    assert(fh);
    char buf[1024*1024];
    size_t pos = 0;
    for (;;)
    {
        size_t bytesRead = fread(buf+pos, 1, 1024, fh);
        if(bytesRead <= 0) {
            break;
        }
        pos += bytesRead;
    }
    fclose(fh);

    unsigned long score = 0;
    char line[1024];
    pos = 0;
    for (;;)
    {
        if(sscanf(buf+pos,"%[a-zA-Z]+",line) <= 0 ) {
            break;
        }
        pos += strlen(line) + 1;
        size_t numItems = strlen(line);
        unsigned long linescore = 0;
        for (size_t i = 0; i < numItems / 2; i++)
        {
            for (size_t j = numItems / 2; j < numItems; j++) {
                if(line[i] == line[j]) {
                    linescore = getPriority(line[i]);
                    break;
                }
            }
            if(linescore) {
                break;
            }
        }
        score += linescore;
    }
    printf("[Part 1] TOTAL SCORE: %zd\n", score);
}

void part2(int argc, char** argv) {
    assert(argc == 2);
    FILE *fh = fopen(argv[1],"rb");
    assert(fh);
    char buf[1024*1024];
    memset(buf, 0x0, sizeof(buf));
    size_t pos = 0;
    for (;;)
    {
        size_t bytesRead = fread(buf+pos, 1, 1024, fh);
        if(bytesRead <= 0) {
            break;
        }
        pos += bytesRead;
    }
    fclose(fh);

    uint64_t score = 0;
    bool done = false;
    pos = 0;
    for (; !done;)
    {
        char matches[1024];
        memset(matches, 0x0, sizeof(matches));
        for (size_t linenum = 0; linenum < 3; linenum++)
        {
            char duplicates[1024];
            memset(duplicates, 0x0, sizeof(duplicates));

            for (; buf[pos] >= 'A' && buf[pos] <= 'z'; pos++) {
                if(!duplicates[(int)buf[pos]]++) {
                    matches[(int)buf[pos]]++;
                    if(matches[(int)buf[pos]] == 3) {
                        score += getPriority(buf[pos]);
                    }
                }
            }
            if (buf[++pos] == '\0')
            {
                done = true;
            }
        }
    }
    printf("[Part 2] Score: %zd\n", score);
}

int main(int argc, char** argv) {
    part1(argc, argv);
    part2(argc, argv);
    return 0;
}