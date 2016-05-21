#ifndef _loader_h_
#define _loader_h_

typedef struct {
    char memToSearch[256];
    char memOverwrite[256];
    int memToSearchSize;
    int memOverwriteSize;
    int occurence;
} binary_patch;

int load(char *path, long offset, binary_patch* patches, int patchesCount);

#endif // _loader_h_
