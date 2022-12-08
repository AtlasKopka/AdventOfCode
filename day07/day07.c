#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LENGTH 32

typedef struct Directory Directory;

typedef struct CurrentDirectory {
  size_t stackPos;
  char directoryStack[1024 * 1024];
  bool isDirectoryListing;
  Directory *currentDirectory;
} CurrentDirectory;
CurrentDirectory *currdir;

typedef struct File {
  char name[MAX_FILENAME_LENGTH];
  uint64_t size;
  bool exists;
} File;

typedef struct Directory {
  char name[MAX_FILENAME_LENGTH];
  struct Directory *directories[1024];
  struct Directory *parent;
  File files[1024];
  uint64_t directorySize;
  bool directorySizeSet;
} Directory;
Directory TLD;

Directory *getSubdirectory(char *name, Directory *parent) {
  for (size_t i = 0;
       i < sizeof(parent->directories) / sizeof(parent->directories[0]); i++) {
    if (!parent->directories[i]) {
      return NULL;
    }
    if (memcmp(name, ((Directory *)parent->directories[i])->name,
               MAX_FILENAME_LENGTH) == 0) {
      return (Directory *)parent->directories[i];
    }
  }
  return NULL;
}

Directory *addSubdirectory(char *name, Directory *parent) {
  Directory *child = getSubdirectory(name, parent);
  if (!child) {
    for (size_t i = 0;
         i < sizeof(parent->directories) / sizeof(parent->directories[0]);
         i++) {
      if (!parent->directories[i]) {
        parent->directories[i] = calloc(1, sizeof(Directory));
        memcpy(((Directory *)parent->directories[i])->name, name,
               MAX_FILENAME_LENGTH);
        parent->directories[i]->parent = parent;
        // printf("(Added directory '%s; at index %zd) ", name, i);
        child = parent->directories[i];
        break;
      }
    }
  }
  // printf("(Directory %s already existed. not adding.) ", name);
  return child;
}

CurrentDirectory *initCurrentDirectory() {
  CurrentDirectory *dir = calloc(1, sizeof(CurrentDirectory));
  assert(dir);
  dir->stackPos = 0;
  memset(dir->directoryStack, 0x0, sizeof(dir->directoryStack));
  dir->isDirectoryListing = false;
  return dir;
}

char *getCurrentDirectoryName() {
  return currdir->directoryStack + currdir->stackPos * MAX_FILENAME_LENGTH;
}

void printCurrentDirectory(CurrentDirectory *dir) {
  for (size_t i = 0; i <= dir->stackPos; i++) {
    for (size_t j = 0; j < MAX_FILENAME_LENGTH; j++) {
      if (dir->directoryStack[i * MAX_FILENAME_LENGTH + j] > 0) {
        printf("%c", dir->directoryStack[i * MAX_FILENAME_LENGTH + j]);
      }
    }
    if (i < dir->stackPos) {
      printf(" ");
    }
  }
}

void mapFileToDirectory(char *line, Directory *parent) {
  uint64_t filesize = 0;
  char filename[MAX_FILENAME_LENGTH];
  memset(filename, 0x0, sizeof(filename));
  sscanf(line, "%zd %[^\r\n ]", &filesize, filename);

  for (size_t i = 0; i < sizeof(parent->files) / sizeof(parent->files[0]);
       i++) {
    if (!parent->files[i].exists) {
      parent->files[i].exists = true;
      parent->files[i].size = filesize;
      memcpy(parent->files[i].name, filename, MAX_FILENAME_LENGTH);
      return;
    }
    // file previously mapped to subdirectory:
    if (memcmp(filename, parent->files[i].name, MAX_FILENAME_LENGTH) == 0) {
      return;
    }
  }
}

void parseDirectoryListing(CurrentDirectory *currdir, char *line) {
  if (line[0] == '$') {
    currdir->isDirectoryListing = false;
  }

  if (currdir->isDirectoryListing) {
    if (line[0] == 'd') {
      char dirName[MAX_FILENAME_LENGTH + 1];
      memset(dirName, 0x0, MAX_FILENAME_LENGTH);
      sscanf(line + 4, "%[^\r\n ]", dirName);
      addSubdirectory(dirName, currdir->currentDirectory);
    } else {
      mapFileToDirectory(line, currdir->currentDirectory);
    }
  }

  if (line[0] == '$' && line[2] == 'l') {
    currdir->isDirectoryListing = true;
  }
}

void updateCurrentDirectory(CurrentDirectory *dir, char *line) {
  if (line[0] != '$') {
    return;
  }

  if (line[2] != 'c') {
    return;
  }

  if (line[5] == '/') {
    dir->stackPos = 0;
    memset(dir->directoryStack + MAX_FILENAME_LENGTH * dir->stackPos, 0x0,
           MAX_FILENAME_LENGTH);
    dir->directoryStack[0] = '/';
    dir->currentDirectory = &TLD;
  } else if (line[5] == '.') {
    memset(dir->directoryStack + MAX_FILENAME_LENGTH * dir->stackPos, 0x0,
           MAX_FILENAME_LENGTH);
    dir->stackPos--;
    dir->currentDirectory = dir->currentDirectory->parent;
  } else {
    dir->stackPos++;
    sscanf(line + 5, "%[^\r\n ]",
           dir->directoryStack + MAX_FILENAME_LENGTH * dir->stackPos);
    dir->currentDirectory =
        addSubdirectory(getCurrentDirectoryName(), dir->currentDirectory);
  }
}

uint64_t updateDirectorySizesRecursive(Directory *dir) {
  uint64_t size = 0;

  for (size_t i = 0; i < sizeof(dir->directories) / sizeof(dir->directories[0]);
       i++) {
    if (!dir->directories[i]) {
      break;
    }
    Directory *subdir = (Directory *)dir->directories[i];
    size += updateDirectorySizesRecursive(subdir);
  }

  for (size_t j = 0; j < sizeof(dir->files) / sizeof(dir->files[0]); j++) {
    if (!dir->files[j].exists) {
      break;
    }
    /*
    printf("DIR %s: FILE %s: SIZE %zd\n", dir->name, dir->files[j].name,
           dir->files[j].size);
    */
    size += dir->files[j].size;
  }

  printf("[[TOTAL SIZE FOR DIR %s: %zd]]\n", dir->name, size);
  dir->directorySize = size;

  return size;
}

size_t part1(Directory *dir) {
  size_t total = 0;
  for (size_t i = 0; i < sizeof(dir->directories) / sizeof(dir->directories[0]);
       i++) {
    if (!dir->directories[i]) {
      break;
    }
    total += part1(dir->directories[i]);
  }
  if (dir->directorySize <= 100000) {
    total += dir->directorySize;
  }
  return total;
}

typedef struct FreeSpace {
  uint64_t spaceToFreeUp;
  Directory *best;
} FreeSpace;
FreeSpace *dirToFree = NULL;

size_t part2(Directory *dir) {
  if (!dirToFree) {
    dirToFree = calloc(1, sizeof(FreeSpace));
    dirToFree->spaceToFreeUp = 30000000 - (70000000 - dir->directorySize);
    printf("Need to free up %zd bytes\n", dirToFree->spaceToFreeUp);
  }

  for (size_t i = 0; i < sizeof(dir->directories) / sizeof(dir->directories[0]);
       i++) {
    if (!dir->directories[i]) {
      break;
    }
    part2(dir->directories[i]);
  }

  if (dir->directorySize >= dirToFree->spaceToFreeUp &&
      (!dirToFree->best ||
       dirToFree->best->directorySize > dir->directorySize)) {
    dirToFree->best = dir;
  }
  return dirToFree->best ? dirToFree->best->directorySize : 0;
}

int main(int argc, char **argv) {
  currdir = initCurrentDirectory();
  memset(&TLD, 0x0, sizeof(Directory));

  FILE *fh = fopen(argv[1], "rb");
  assert(fh);
  for (;;) {
    char line[1024];
    memset(line, 0x0, sizeof(line));
    if (fscanf(fh, "%[^\r\n]", line) == EOF) {
      break;
    }
    fscanf(fh, "%*[\r\n ]");
    updateCurrentDirectory(currdir, line);
    parseDirectoryListing(currdir, line);

    printf("LINE: %s [", line);
    printCurrentDirectory(currdir);
    printf("]\n");
  }

  printf("Updating directory sizes...\n");
  updateDirectorySizesRecursive(&TLD);
  printf("[part1]: %zd\n", part1(&TLD));
  printf("[part2]: %zd\n", part2(&TLD));

  fclose(fh);
  free(currdir);
  return 0;
}