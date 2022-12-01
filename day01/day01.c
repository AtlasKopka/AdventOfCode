#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

void readfile(int argc, char** argv, char* buf) {
    if(argc!=2) {
        printf("Filename needed.");
        exit(1);
    }
    FILE *fh = fopen(argv[1],"rb");
    if(!fh) {
        printf("Could not open file");
        exit(1);
    }
    size_t fpos = 0;
    for(;;) {
        size_t bytesread = fread(buf+fpos,1024,1024,fh);
        if(bytesread > 0) {
            fpos+= bytesread;
        } else {
            break;
        }
    }
    fclose(fh);
}

int main(int argc, char** argv) {
    char buf[1024*1024];
    memset(buf,0x0,sizeof(buf));
    readfile(argc,argv,buf);

    char* bufpos = buf;
    int64_t batchnum = 0;
    int64_t batchtotal = 0;
    
    int64_t maxbatch = 0;
    int64_t maxbatchtotal = 0;
    for(;;) {
        if(*bufpos == '\n') {
            bufpos++;

            if(batchtotal >= maxbatchtotal) {
                maxbatch = batchnum;
                maxbatchtotal = batchtotal;
            }
            batchnum++;
            batchtotal = 0;
        }

        int num = 0;
        int result = 0;
        result = sscanf(bufpos,"%d",&num);
        if(result == 1) {
            batchtotal += num;
        } else {
            printf("Finished processing input.\n");
        }

        bufpos = strchr(bufpos,'\n');
        if(!bufpos) {
            break;
        }
        bufpos++;
    }

    printf("Batch %ld has the maximum of: %ld\n", maxbatch, maxbatchtotal);

    return 0;
}