#ifndef _movie_h_
#define _movie_h_

#include "quicklz.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    qlz_state_decompress state;
    const char* frame_prev_read;
    char* frame_prev_write;
    char* frame_comp;
    int reverse;
} comp_manager_s;

int readMovieFrame(void* iFileHandlePtr, int* ioFileOffset, comp_manager_s* iCompMgr, char* oFrameBuffer, int iFrameBufferSize);
int readCompressedMovieFrameFromBuffer(comp_manager_s* iCompMgr, char* oFrameBuffer, int iFrameBufferSize);

int getMovieFramesCount(void* iFileHandlePtr, int iFrameBufferSize, int iCompressed);
void getMovieDataOffsetForFrames(void* iFileHandlePtr, int iFrameBufferSize, int iCompressed, int iStartFrame, int iEndFrame,
                                 int* oStartFrameOffset, int* oDataSizeToEndFrame);

#ifdef __cplusplus
}
#endif

#endif
