#ifndef _anim_h_
#define _anim_h_

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int timer;
    int active;

    u8 bgTop1[3];
    u8 bgTop2[3];
    u8 bgBot[3];
    u8 highlight[4];
    u8 borders[4];
    u8 fntDef[4];
    u8 fntSel[4];    
} anim_state_s;

anim_state_s* anim;

void animInit();
void animSetup();
void incrementAnimTime();

#ifdef __cplusplus
}
#endif

#endif
