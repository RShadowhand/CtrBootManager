#include "movie.h"

#include "config.h"
#include "utility.h"

int readMovieFrame(void* iFileHandlePtr, int* ioFileOffset, comp_manager_s* iCompMgr, char* oFrameBuffer, int iFrameBufferSize)
{
    if ( NULL == iCompMgr )
    {
        int readBytes = fileHandleRead(iFileHandlePtr, oFrameBuffer, iFrameBufferSize, *ioFileOffset);
        if ( readBytes < 0 )
            return -1;
        *ioFileOffset += readBytes;
        return 0;
    }
    else
    {
        int readBytes = fileHandleRead(iFileHandlePtr, iCompMgr->frame_comp, 9, *ioFileOffset); // Read top animations QuickLZ header
        if (readBytes != 9) // If it couldn't read the header
            return -1; // Disable the animation
        *ioFileOffset += readBytes;

        size_t comp_size = qlz_size_compressed(iCompMgr->frame_comp); // Get compressed size

        readBytes = fileHandleRead(iFileHandlePtr, iCompMgr->frame_comp + 9, comp_size - 9, *ioFileOffset); // Read compressed buffer

        if (readBytes != (comp_size - 9) || qlz_size_decompressed(iCompMgr->frame_comp) != iFrameBufferSize) // Check if EOF
            return -1; // Disable the animation
        *ioFileOffset += readBytes;

        qlz_decompress(iCompMgr->frame_comp, iCompMgr->frame_curr, &iCompMgr->state); // Decompress the frame

        for (size_t i = 0; i < iFrameBufferSize; i++) // Perform delta decoding
            iCompMgr->frame_curr[i] += iCompMgr->frame_prev[i];

        memcpy(iCompMgr->frame_prev, iCompMgr->frame_curr, iFrameBufferSize); // Keep a copy of the fb
        memcpy(oFrameBuffer, iCompMgr->frame_curr, iFrameBufferSize); // Display it on the screen
        return 0;
    }
}

int getMovieFramesCount(void* iFileHandlePtr, int iFrameBufferSize, int iCompressed, int* ioFrameOffset)
{
    size_t fileSize = fileHandleSize(iFileHandlePtr);
    if ( !iCompressed )
    {
        int framesCount = fileSize / iFrameBufferSize;
        if ( NULL != ioFrameOffset )
            *ioFrameOffset = ( *ioFrameOffset < framesCount && *ioFrameOffset >=0 ) ? (*ioFrameOffset * iFrameBufferSize) : -1;
        return framesCount;
    }
    else
    {
        char compHeader[9];
        int framesCount = 0;
        int fileOffset = 0;

        while ( 9 == fileHandleRead(iFileHandlePtr, compHeader, 9, fileOffset) )
        {
            size_t comp_size = qlz_size_compressed(compHeader); // Get compressed size
            if ( fileOffset + comp_size > fileSize )
                break;
            if ( NULL != ioFrameOffset && *ioFrameOffset == framesCount )
            {
                *ioFrameOffset = fileOffset;
                ioFrameOffset = NULL;
            }
            fileOffset += comp_size;
            framesCount++;
        }
        
        if ( NULL != ioFrameOffset )
            *ioFrameOffset = -1; // We didn't find any valid frame
        
        return framesCount;
    }
}
