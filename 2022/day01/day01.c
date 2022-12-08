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

int batchcmp(const void* a, const void* b) {
    return (*(int64_t*)a < *(int64_t*)b) ? 1 : ((*(int64_t*)a == *(int64_t*)b) ? 0 : -1);
}

#define BATCHSIZE(name) (sizeof(name)/sizeof(name[0]))

#define BATCH_TOTALS_CACHE_SIZE 9
void getmax(char* buf, size_t topn) {
    int64_t batchtotals[BATCH_TOTALS_CACHE_SIZE];
    memset(batchtotals,0,sizeof(batchtotals));

    char* bufpos = buf;
    int64_t batchnum = 0;
    int64_t batchtotal = 0;
    
    for(;;) {
        if(*bufpos == '\n') {
            bufpos++;

            if(batchtotal > batchtotals[BATCHSIZE(batchtotals)-1]) {
                batchtotals[BATCHSIZE(batchtotals)-1] = batchtotal;
                qsort(batchtotals,BATCHSIZE(batchtotals),sizeof(batchtotals[0]),batchcmp);
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

    for(size_t i = 0; i < BATCHSIZE(batchtotals); i++) {
        printf("%zd: %ld\n", i, batchtotals[i]);
    }
    printf("\n");

    int64_t topn_total = 0;
    for(size_t i = 0; i < topn; i++) {
        topn_total += batchtotals[i];
    }

    printf("The total of the top %zu entries is: %ld\n", topn, topn_total);
}

int main(int argc, char** argv) {
    char buf[1024*1024];
    memset(buf,0x0,sizeof(buf));
    readfile(argc,argv,buf);

    getmax(buf,1);
    getmax(buf,3);

    return 0;
}