#include "anim.h"

#ifdef ARM9
#include "memory.h"
#endif

void animInit()
{
#ifdef ARM9
    anim = (anim_state_s*)PTR_ANIM;
#else
    anim = malloc(sizeof(anim_state_s));
#endif
    
    anim->timer = 0;
    anim->active = 0;
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
