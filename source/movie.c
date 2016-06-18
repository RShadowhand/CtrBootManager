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

        if (readBytes != (comp_size - 9) || readCompressedMovieFrameFromBuffer(iCompMgr, oFrameBuffer, iFrameBufferSize)) // Check if EOF
            return -1; // Disable the animation

        *ioFileOffset += readBytes;
        return 0;
    }
}

int readCompressedMovieFrameFromBuffer(comp_manager_s* iCompMgr, char* oFrameBuffer, int iFrameBufferSize)
{
    if ( qlz_size_decompressed(iCompMgr->frame_comp) != iFrameBufferSize )
        return -1;
    
    qlz_decompress(iCompMgr->frame_comp, oFrameBuffer, &iCompMgr->state); // Decompress the frame

    if ( iCompMgr->reverse )
    {
        for (size_t i = 0; i < iFrameBufferSize; i++) // Perform delta decoding
            oFrameBuffer[i] = iCompMgr->frame_prev_read[i] - oFrameBuffer[i];
    }
    else
    {
        for (size_t i = 0; i < iFrameBufferSize; i++) // Perform delta decoding
            oFrameBuffer[i] += iCompMgr->frame_prev_read[i];
    }

    memcpy(iCompMgr->frame_prev_write, oFrameBuffer, iFrameBufferSize); // Keep a copy of the fb
    return 0;
}

int getMovieFramesCount(void* iFileHandlePtr, int iFrameBufferSize, int iCompressed)
{
    size_t fileSize = fileHandleSize(iFileHandlePtr);
    if ( !iCompressed )
        return fileSize / iFrameBufferSize;
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
            fileOffset += comp_size;
            framesCount++;
        }

        return framesCount;
    }
}

void getMovieDataOffsetForFrames(void* iFileHandlePtr, int iFrameBufferSize, int iCompressed, int iStartFrame, int iEndFrame,
                                 int* oStartFrameOffset, int* oDataSizeToEndFrame)
{
    size_t fileSize = fileHandleSize(iFileHandlePtr);
    if ( !iCompressed )
    {
        int framesCount = fileSize / iFrameBufferSize;
        if ( NULL != oStartFrameOffset )
            *oStartFrameOffset = iFrameBufferSize * ( ( iStartFrame < framesCount ) ? iStartFrame : (framesCount-1) );
        if ( NULL != oDataSizeToEndFrame )
            *oDataSizeToEndFrame = iFrameBufferSize * ( ( iEndFrame < framesCount ) ? (iEndFrame+1) : framesCount );
    }
    else
    {
        char compHeader[9];
        int curFrame = 0;
        int fileOffset = 0;
        int firstFrameOffset = -1;

        while ( curFrame <= iEndFrame && 9 == fileHandleRead(iFileHandlePtr, compHeader, 9, fileOffset) )
        {
            size_t comp_size = qlz_size_compressed(compHeader); // Get compressed size
            if ( fileOffset + comp_size > fileSize )
                break;
            if ( curFrame == iStartFrame )
                firstFrameOffset = fileOffset;
            fileOffset += comp_size;
            curFrame++;
        }

        if ( NULL != oStartFrameOffset )
            *oStartFrameOffset = firstFrameOffset;
        if ( NULL != oDataSizeToEndFrame )
            *oDataSizeToEndFrame = fileOffset;
    }
}
