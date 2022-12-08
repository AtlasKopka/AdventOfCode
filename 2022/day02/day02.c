#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void part1(int argc, char** argv) {
        assert(argc == 2);
    FILE *fh = fopen(argv[1], "rb");
    assert(fh != NULL);
    int64_t score = 0;
    for(;;) {
        char opponent[2], self[2];
        if(fscanf(fh,"%[A-C] %[X-Z] ", opponent, self) <= 0) {
            break;
        }
        switch(self[0]) {
            case 'X':
            score += 1;
            break;
            case 'Y':
            score += 2;
            break;
            case 'Z':
            score += 3;
            break;
        }
        // detect a draw:
        if(opponent[0] + 23 == self[0]) {
            score += 3;
        } else {
            if(
                (opponent[0] == 'A' && self[0] == 'Y')
                ||
                (opponent[0] == 'B' && self[0] == 'Z')
                ||
                (opponent[0] == 'C' && self[0] == 'X') 
            ){
                // win:
                score += 6;
            } else{
                // loss:
                score += 0;
            }
        }
    }
    printf("[part 1] Total score: %ld\n", score);
}

#define ROCK 1
#define PAPER 2
#define SCISSORS 3

#define LOSE 0
#define DRAW 3
#define WIN 6

void part2(int argc, char** argv) {
        assert(argc == 2);
    FILE *fh = fopen(argv[1], "rb");
    assert(fh != NULL);
    int64_t score = 0;
    for(;;) {
        char opponent[2], self[2];
        if(fscanf(fh,"%[A-C] %[X-Z] ", opponent, self) <= 0) {
            break;
        }
        int objective = (self[0] - 'X')*3; // LOSE, or DRAW, or WIN
        int shape = opponent[0] - 'A' + 1; // ROCK, or PAPER, or SCISSORS
        score += objective;
        switch(objective) {
            case LOSE:
            {
                score += ((shape + 1) % 3)+1;
                break;
            }
            case DRAW:
            {
                score += shape;
                break;
            }
            case WIN:
            {
                score += (shape % 3) + 1;
                break;
            }
        }
    }
    printf("[part 2] Total score: %ld\n", score);
}

int main(int argc, char** argv) {
    part1(argc,argv);
    part2(argc,argv);
    return 0;
}