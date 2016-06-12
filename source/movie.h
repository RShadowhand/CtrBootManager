#ifndef _movie_h_
#define _movie_h_

#include "quicklz.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    qlz_state_decompress state;
    char* frame_prev;
    char* frame_curr;
    char* frame_comp;
} comp_manager_s;

int readMovieFrame(void* iFileHandlePtr, int* ioFileOffset, comp_manager_s* iCompMgr, char* oFrameBuffer, int iFrameBufferSize);
int getMovieFramesCount(void* iFileHandlePtr, int iFrameBufferSize, int iCompressed, int* ioFrameOffset);

#ifdef __cplusplus
}
#endif

#endif
