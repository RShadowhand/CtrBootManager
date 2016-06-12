#include "anim.h"

#define TOP_SCREEN_SIZE (400*240*3)
#define BOT_SCREEN_SIZE (320*240*3)

#ifdef ARM9
#include "memory.h"

// Top compression required memory: 864000+28809 = 892809
#define TOP_COMP_PTR (char*)PTR_MOVIE_COMP
#define TOP_COMP_PREV_FRAME_PTR (TOP_COMP_PTR+sizeof(comp_manager_s))
#define TOP_COMP_CURR_FRAME_PTR (TOP_COMP_PREV_FRAME_PTR+TOP_SCREEN_SIZE)
#define TOP_COMP_COMP_FRAME_PTR (TOP_COMP_CURR_FRAME_PTR+TOP_SCREEN_SIZE)

// Bottom compression required memory: 691200+23049 = 714249
#define BOT_COMP_PTR (TOP_COMP_COMP_FRAME_PTR+TOP_SCREEN_SIZE+(TOP_SCREEN_SIZE/10)+9)
#define BOT_COMP_PREV_FRAME_PTR (BOT_COMP_PTR+sizeof(comp_manager_s))
#define BOT_COMP_CURR_FRAME_PTR (BOT_COMP_PREV_FRAME_PTR+BOT_SCREEN_SIZE)
#define BOT_COMP_COMP_FRAME_PTR (BOT_COMP_CURR_FRAME_PTR+BOT_SCREEN_SIZE)

// Total compression required memory: 1607058
#endif

#include "utility.h"

void movieInit(movie_state_s* movie)
{
    movie->file = NULL;
    movie->fileOffset = 0;
    movie->comp = NULL;
    movie->loopStream = NULL;
    movie->curLoop = 0;
    movie->loopsToDo = 0;
    movie->loopStartFrameStream = NULL;
    movie->loopEndFrameStream = NULL;
}

void animInit()
{
#ifdef ARM9
    anim = (anim_state_s*)PTR_ANIM;
#else
    anim = malloc(sizeof(anim_state_s));
#endif
    
    anim->timer = 0;
    anim->active = 0;
    
    anim->topMovie.frameSize = TOP_SCREEN_SIZE;
    movieInit(&anim->topMovie);

    anim->botMovie.frameSize = BOT_SCREEN_SIZE;
    movieInit(&anim->botMovie);
}

void movieLoopSetup(movie_state_s* movie)
{
    movie->loopFramesCount = movie->loopEndFrame - movie->loopStartFrame
                           + movie->loopTimeOnStartFrame + movie->loopTimeOnEndFrame;

    if ( movie->loopTimeOnStartFrame > 0 )
        movie->loopStartFrameStream = movie->loopStream;

    if ( movie->loopTimeOnEndFrame > 0 )
        movie->loopEndFrameStream = movie->loopStream + movie->frameSize*(movie->loopEndFrame-movie->loopStartFrame);
}

#ifdef ARM9
void movieSetup(movie_config_s* mvConf, movie_state_s* movie, void* filePtr, comp_manager_s* compPtr, void* streamPtr)
#else
void movieSetup(movie_config_s* mvConf, movie_state_s* movie)
#endif
{
    if ( mvConf->path[0] == '\0' )
        return;

#ifdef ARM9
    movie->file = (void*)(filePtr);
#else
    int fileHandleSize = getFileHandleSize();
    movie->file = malloc(fileHandleSize);
#endif
    if ( fileHandleOpen(movie->file, mvConf->path) < 0 )
    {
    #ifndef ARM9
        free(movie->file);
    #endif
        movie->file = NULL;
        return;
    }

    if ( mvConf->compressed )
    {
    #ifdef ARM9
        movie->comp = compPtr;
    #else
        movie->comp = (comp_manager_s*)malloc(sizeof(comp_manager_s));
        movie->comp->frame_prev = malloc(movie->frameSize);
        movie->comp->frame_curr = malloc(movie->frameSize);
        movie->comp->frame_comp = malloc(movie->frameSize + (movie->frameSize/10) + 9);
    #endif

        memset(&movie->comp->state, 0, sizeof(qlz_state_decompress));
        memset(movie->comp->frame_prev, 0, movie->frameSize);
        memset(movie->comp->frame_curr, 0, movie->frameSize);
        memset(movie->comp->frame_comp, 0, movie->frameSize + (movie->frameSize/10) + 9);
    }
    
    movie->framesCount = getMovieFramesCount(movie->file, movie->frameSize, (NULL != movie->comp), NULL);
    if ( movie->framesCount <= 0 )
    {
        fileHandleClose(movie->file);
    #ifndef ARM9
        free(movie->file);
    #endif
        movie->file = NULL;
        return;
    }

    movie->curFrame = 0;
    movie->curLoop = 0;
    movie->loopsToDo = mvConf->loopCount;
    if ( 0 == movie->loopsToDo )
        return;

    movie->loopReverse = mvConf->loopReverse;
    
    movie->loopTimeOnStartFrame = ( mvConf->loopTimeOnStartFrame > 0 ) ? mvConf->loopTimeOnStartFrame : 0;
    movie->loopTimeOnEndFrame = ( mvConf->loopTimeOnEndFrame > 0 ) ? mvConf->loopTimeOnEndFrame : 0;

    movie->loopStartFrame = (mvConf->loopStartFrame < movie->framesCount) ? mvConf->loopStartFrame : (movie->framesCount-1);
    if ( movie->loopStartFrame < 0 )
        movie->loopStartFrame = 0;
    
    movie->loopEndFrame = (mvConf->loopEndFrame < 0 || mvConf->loopEndFrame > movie->loopStartFrame)
                        ? mvConf->loopEndFrame : movie->loopStartFrame;
    if ( movie->loopEndFrame < 0 || movie->loopEndFrame >= movie->framesCount )
        movie->loopEndFrame = movie->framesCount-1;

    movie->loopStreamSize = movie->frameSize * (movie->loopEndFrame-movie->loopStartFrame+1);
#ifdef ARM9
    movie->loopStream = streamPtr;
#else
    movie->loopStream = malloc(movie->loopStreamSize);
#endif

    movieLoopSetup(movie);
}

void animSetup()
{
    memcpy(anim->bgTop1, config->bgTop1, sizeof(u8[3]));
    memcpy(anim->bgTop2, config->bgTop2, sizeof(u8[3]));
    memcpy(anim->bgBot, config->bgBot, sizeof(u8[3]));
    memcpy(anim->highlight, config->highlight, sizeof(u8[4]));
    memcpy(anim->borders, config->borders, sizeof(u8[4]));
    memcpy(anim->fntDef, config->fntDef, sizeof(u8[4]));
    memcpy(anim->fntSel, config->fntSel, sizeof(u8[4]));
    
#ifdef ARM9
    comp_manager_s* topComp = (comp_manager_s*)PTR_MOVIE_COMP;
    topComp->frame_prev = TOP_COMP_PREV_FRAME_PTR;
    topComp->frame_curr = TOP_COMP_CURR_FRAME_PTR;
    topComp->frame_comp = TOP_COMP_COMP_FRAME_PTR;

    comp_manager_s* botComp = (comp_manager_s*)BOT_COMP_PTR;
    botComp->frame_prev = BOT_COMP_PREV_FRAME_PTR;
    botComp->frame_curr = BOT_COMP_CURR_FRAME_PTR;
    botComp->frame_comp = BOT_COMP_COMP_FRAME_PTR;

    int fileHandleSize = getFileHandleSize();
    movieSetup(&config->movieTop, &anim->topMovie, PTR_ANIM+sizeof(anim_state_s), topComp, PTR_MOVIE_LOOP);
    movieSetup(&config->movieBot, &anim->botMovie, PTR_ANIM+sizeof(anim_state_s)+fileHandleSize,
               botComp, PTR_MOVIE_LOOP+anim->topMovie.loopStreamSize);
#else
    movieSetup(&config->movieTop, &anim->topMovie);
    movieSetup(&config->movieBot, &anim->botMovie);
#endif

    if ( config->bgTop1AnimTime > 0 || config->bgTop2AnimTime > 0 || config->bgBotAnimTime > 0
      || config->highlightAnimTime > 0 || config->bordersAnimTime > 0
      || config->fntDefAnimTime > 0 || config->fntSelAnimTime > 0
      || config->menuFadeInTime > 0 || config->menuFadeInTimeStart != 0 )
    {
        anim->active = 1;
        incrementAnimTime();
    }
}

float computeMerge(int elapsedTime, int animTime, int animStart)
{
    int curTime = (elapsedTime+animStart) % (2*animTime);
    int animStep = (curTime/animTime == 0) ? (curTime%animTime) : (animTime-curTime%animTime-1);
    return (float)animStep / (float)animTime;
}

void setMergedColor(float mergeVal, u8 color1[3], u8 color2[3], u8 finalColor[3])
{
    float oneMinusMerge = 1.0f - mergeVal;
    finalColor[0] = (u8)(oneMinusMerge*color1[0]+mergeVal*color2[0]);
    finalColor[1] = (u8)(oneMinusMerge*color1[1]+mergeVal*color2[1]);
    finalColor[2] = (u8)(oneMinusMerge*color1[2]+mergeVal*color2[2]);
}

void setMergedColorAlpha(float mergeVal, u8 color1[4], u8 color2[4], u8 finalColor[4])
{
    float oneMinusMerge = 1.0f - mergeVal;
    finalColor[0] = (u8)(oneMinusMerge*color1[0]+mergeVal*color2[0]);
    finalColor[1] = (u8)(oneMinusMerge*color1[1]+mergeVal*color2[1]);
    finalColor[2] = (u8)(oneMinusMerge*color1[2]+mergeVal*color2[2]);
    finalColor[3] = (u8)(oneMinusMerge*color1[3]+mergeVal*color2[3]);
}

void incrementAnimTime()
{
    if (!anim->active)
        return;
    
    if ( config->bgTop1AnimTime > 0 )
    {
        float merge = computeMerge(anim->timer, config->bgTop1AnimTime, config->bgTop1AnimTimeStart);
        setMergedColor(merge, config->bgTop1, config->bgTop1AnimColor, anim->bgTop1);
    }
    
    if ( config->bgTop2AnimTime > 0 )
    {
        float merge = computeMerge(anim->timer, config->bgTop2AnimTime, config->bgTop2AnimTimeStart);
        setMergedColor(merge, config->bgTop2, config->bgTop2AnimColor, anim->bgTop2);
    }

    if ( config->bgBotAnimTime > 0 )
    {
        float merge = computeMerge(anim->timer, config->bgBotAnimTime, config->bgBotAnimTimeStart);
        setMergedColor(merge, config->bgBot, config->bgBotAnimColor, anim->bgBot);
    }

    if ( config->highlightAnimTime > 0 )
    {
        float merge = computeMerge(anim->timer, config->highlightAnimTime, config->highlightAnimTimeStart);
        setMergedColorAlpha(merge, config->highlight, config->highlightAnimColor, anim->highlight);
    }
    else if ( anim->timer > config->menuFadeInTimeStart )
        memcpy(anim->highlight, config->highlight, sizeof(u8[4]));

    if ( config->bordersAnimTime > 0 )
    {
        float merge = computeMerge(anim->timer, config->bordersAnimTime, config->bordersAnimTimeStart);
        setMergedColorAlpha(merge, config->borders, config->bordersAnimColor, anim->borders);
    }
    else if ( anim->timer > config->menuFadeInTimeStart )
        memcpy(anim->borders, config->borders, sizeof(u8[4]));
    
    if ( config->fntDefAnimTime > 0 )
    {
        float merge = computeMerge(anim->timer, config->fntDefAnimTime, config->fntDefAnimTimeStart);
        setMergedColorAlpha(merge, config->fntDef, config->fntDefAnimColor, anim->fntDef);
    }
    else if ( anim->timer > config->menuFadeInTimeStart )
        memcpy(anim->fntDef, config->fntDef, sizeof(u8[4]));

    if ( config->fntSelAnimTime > 0 )
    {
        float merge = computeMerge(anim->timer, config->fntSelAnimTime, config->fntSelAnimTimeStart);
        setMergedColorAlpha(merge, config->fntSel, config->fntSelAnimColor, anim->fntSel);
    }
    else if ( anim->timer > config->menuFadeInTimeStart )
        memcpy(anim->fntSel, config->fntSel, sizeof(u8[4]));
    
    if ( anim->timer >= config->menuFadeInTimeStart )
    {
        int fadeInTime = anim->timer - config->menuFadeInTimeStart;
        if ( fadeInTime <= config->menuFadeInTime )
        {
            float fadeIn = (config->menuFadeInTime > 0) ? (float)fadeInTime / (float)config->menuFadeInTime : 1.f;
            
            anim->highlight[3] = (u8)(fadeIn*(float)anim->highlight[3]);
            anim->borders[3] = (u8)(fadeIn*(float)anim->borders[3]);
            anim->fntDef[3] = (u8)(fadeIn*(float)anim->fntDef[3]);
            anim->fntSel[3] = (u8)(fadeIn*(float)anim->fntSel[3]);
            
            if ( fadeInTime == config->menuFadeInTime && config->bgTop1AnimTime == 0
              && config->bgTop2AnimTime == 0 && config->bgBotAnimTime == 0
              && config->highlightAnimTime == 0 && config->bordersAnimTime == 0
              && config->fntDefAnimTime == 0 && config->fntSelAnimTime == 0 )
                anim->active = 0;
        }
    }
    else
    {
        anim->highlight[3] = 0x00;
        anim->borders[3] = 0x00;
        anim->fntDef[3] = 0x00;
        anim->fntSel[3] = 0x00;
    }

    anim->timer++;
}

int readMovie(u8* fb, movie_state_s* movie)
{
    if ( movie->curLoop == 0 )
    {
        if ( NULL == movie->file )
            return -1;
        int readMovieRes = readMovieFrame(movie->file, &movie->fileOffset, movie->comp, (char*)fb, movie->frameSize);
        if ( readMovieRes < 0 || movie->curFrame >= movie->framesCount-1
          || ( movie->loopReverse && movie->curFrame >= movie->loopEndFrame ) )
        {
            fileHandleClose(movie->file);
        #ifndef ARM9
            free(movie->file);
        #endif
            movie->file = NULL;
            movie->curLoop = 1;
            
            if ( 0 != movie->loopsToDo && movie->curFrame < movie->framesCount-1 )
            {
                // Correction for corrupted compressed movies
                movie->framesCount = movie->curFrame;
                if ( movie->loopStartFrame >= movie->framesCount )
                    movie->loopStartFrame = movie->framesCount-1;
                if ( movie->loopEndFrame >= movie->framesCount )
                    movie->loopEndFrame = movie->framesCount-1;
                movieLoopSetup(movie);
            }
        }

        if ( 0 == readMovieRes && NULL != movie->loopStream && movie->curFrame >= movie->loopStartFrame && movie->curFrame <= movie->loopEndFrame )
            memcpy(movie->loopStream+(movie->curFrame-movie->loopStartFrame)*movie->frameSize, fb, movie->frameSize);

        movie->curFrame++;
        if ( movie->curLoop > 0 )
            movie->curFrame = 0;
    }
    else if ( movie->loopsToDo < 0 || movie->curLoop <= movie->loopsToDo )
    {
        if ( movie->loopReverse && movie->curLoop%2 == 1 )
        {
            if ( movie->curFrame < movie->loopTimeOnEndFrame )
                memcpy(fb, movie->loopEndFrameStream, movie->frameSize);
            else if ( movie->curFrame > movie->loopTimeOnEndFrame+movie->loopEndFrame-movie->loopStartFrame )
                memcpy(fb, movie->loopStartFrameStream, movie->frameSize);            
            else
            {
                memcpy(fb, movie->loopStream + movie->frameSize*((movie->loopEndFrame-movie->loopStartFrame)
                       -(movie->curFrame-movie->loopTimeOnEndFrame)), movie->frameSize);
            }
        }
        else
        {
            if ( movie->curFrame < movie->loopTimeOnStartFrame )
                memcpy(fb, movie->loopStartFrameStream, movie->frameSize);
            else if ( movie->curFrame > movie->loopTimeOnStartFrame+movie->loopEndFrame-movie->loopStartFrame )
                memcpy(fb, movie->loopEndFrameStream, movie->frameSize);            
            else
                memcpy(fb, movie->loopStream+movie->frameSize*(movie->curFrame-movie->loopTimeOnStartFrame), movie->frameSize);
        }

        movie->curFrame++;
        if ( movie->curFrame > movie->loopFramesCount )
        {
            movie->curFrame = 0;
            movie->curLoop++;
        }
    }
    else
        return -1;

    return 0;
}

int readTopMovie(u8* fb)
{
    return readMovie(fb, &anim->topMovie);
}

int readBotMovie(u8* fb)
{
    return readMovie(fb, &anim->botMovie);
}

void animExit()
{
    if (anim) {
        if (anim->topMovie.file)
            fileHandleClose(anim->topMovie.file);
        if (anim->botMovie.file)
            fileHandleClose(anim->botMovie.file);
#ifndef ARM9
        if (anim->topMovie.file)
            free(anim->topMovie.file);
        if (anim->topMovie.comp)
        {
            free(anim->topMovie.comp->frame_prev);
            free(anim->topMovie.comp->frame_curr);
            free(anim->topMovie.comp->frame_comp);
            free(anim->topMovie.comp);
        }
        if (anim->topMovie.loopStream)
            free(anim->topMovie.loopStream);

        if (anim->botMovie.file)
            free(anim->botMovie.file);
        if (anim->botMovie.comp)
        {
            free(anim->botMovie.comp->frame_prev);
            free(anim->botMovie.comp->frame_curr);
            free(anim->botMovie.comp->frame_comp);
            free(anim->botMovie.comp);
        }
        if (anim->botMovie.loopStream)
            free(anim->botMovie.loopStream);
        free(anim);
#endif
        anim = NULL;
    }
}
