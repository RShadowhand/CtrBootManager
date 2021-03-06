//
// Created by cpasjuste on 25/01/16.
//

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "gfx.h"
#include "anim.h"
#include "config.h"
#include "menu.h"

void drawBg() {
    u8* fbTop = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    if (readTopMovie(fbTop) < 0)
    {
        if (!config->imgError) {
            memcpy(fbTop, config->bgImgTopBuff, (size_t) config->bgImgTopSize);
        } else {
            gfxClearTop(anim->bgTop1, anim->bgTop2);
        }
    }
    u8* fbBot = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
    if (readBotMovie(fbBot) < 0)
    {
        if (!config->imgErrorBot) {
            memcpy(fbBot, config->bgImgBotBuff, (size_t) config->bgImgBotSize);
        } else {
            gfxClearBot(anim->bgBot);
        }
    }
    drawRectColor(GFX_TOP, GFX_LEFT, MENU_MIN_X, MENU_MIN_Y - 20, MENU_MAX_X, MENU_MAX_Y, anim->borders);
}

void drawTitle(const char *format, ...) {

    char msg[512];
    va_list argp;
    va_start(argp, format);
    vsnprintf(msg, 512, format, argp);
    va_end(argp);

    gfxDrawText(GFX_TOP, GFX_LEFT, &fontDefault, msg, 140, 25);
}

void drawItem(bool selected, int y, const char *format, ...) {

    char msg[512];
    va_list argp;
    va_start(argp, format);
    vsnprintf(msg, 512, format, argp);
    va_end(argp);

    if (selected) {
        gfxDrawRectangle(GFX_TOP, GFX_LEFT, anim->highlight, (s16) (MENU_MIN_X + 4), (s16) (y + MENU_MIN_Y), 361, 15);
    }
    memcpy(fontDefault.color, selected ? anim->fntSel : anim->fntDef, sizeof(u8[4]));
    gfxDrawText(GFX_TOP, GFX_LEFT, &fontDefault, msg, (s16) (MENU_MIN_X + 6),
                (s16) y + (s16) MENU_MIN_Y);

    memcpy(fontDefault.color, anim->fntDef, sizeof(u8[4]));
}

void drawItemN(bool selected, int maxChar, int y, const char *format, ...) {

    char msg[512];
    va_list argp;
    va_start(argp, format);
    vsnprintf(msg, 512, format, argp);
    va_end(argp);

    if (selected) {
        gfxDrawRectangle(GFX_TOP, GFX_LEFT, anim->highlight, (s16) (MENU_MIN_X + 4), (s16) (y + MENU_MIN_Y), 361, 15);
    }
    memcpy(fontDefault.color, selected ? anim->fntSel : anim->fntDef, sizeof(u8[4]));
    gfxDrawTextN(GFX_TOP, GFX_LEFT, &fontDefault, msg, maxChar, (s16) (MENU_MIN_X + 6),
                 (s16) y + (s16) MENU_MIN_Y);

    memcpy(fontDefault.color, anim->fntDef, sizeof(u8[4]));
}

void drawInfo(const char *format, ...) {

    char msg[512];
    va_list argp;
    va_start(argp, format);
    vsnprintf(msg, 512, format, argp);
    va_end(argp);

    gfxDrawText(GFX_BOTTOM, GFX_LEFT, &fontDefault, "Informations", (s16) (MENU_MIN_X + 6), 40);
    gfxDrawText(GFX_BOTTOM, GFX_LEFT, &fontDefault, msg, (s16) (MENU_MIN_X + 12), 80);
}
