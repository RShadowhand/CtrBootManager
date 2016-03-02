#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define s16 int16_t

#define vu8 volatile u8
#define vu16 volatile u16
#define vu32 volatile u32
#define vu64 volatile u64

#define max(a,b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a > _b ? _a : _b; })
#define min(a,b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a < _b ? _a : _b; })
#define getbe16(d) \
    (((d)[0]<<8) | (d)[1])
#define getbe32(d) \
    ((((u32) getbe16(d))<<16) | ((u32) getbe16(d+2)))
#define getbe64(d) \
    ((((u64) getbe32(d))<<32) | ((u64) getbe32(d+4)))
#define getle16(d) (*((u16*) (d)))
#define getle32(d) (*((u32*) (d)))
#define getle64(d) (*((u64*) (d)))
#define align(v,a) \
    (((v) % (a)) ? ((v) + (a) - ((v) % (a))) : (v))
    
#define PAYLOAD_ADDRESS		0x23F00000
#define PAYLOAD_DATA		0x24F00000
#define PAYLOAD_STAGE2		0x24000000
#define PAYLOAD_SIZE		0x00100000


typedef enum
{
    GFX_TOP = 0,   ///< Top screen
    GFX_BOTTOM = 1 ///< Bottom screen
}gfxScreen_t;

typedef enum
{
    GFX_LEFT = 0, ///< Left eye framebuffer
    GFX_RIGHT = 1,///< Right eye framebuffer
}gfx3dSide_t;
