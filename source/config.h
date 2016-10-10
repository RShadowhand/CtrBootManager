#ifndef _config_h_
#define _config_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM9

#include "arm9/source/common.h"
#define CONFIG_PATH "/shadownand/bootmgr.cfg\0Free space for binary patch to chainload another instance of CtrBootManager9.....End"

#else
    
#include <3ds.h>
#include "utility.h"
#define CONFIG_PATH "/shadownand/bootmgr.cfg"

#endif

#include "loader.h" // Binary patches define

#define BIT(n) (1U<<(n))
#define CONFIG_MAX_ENTRIES 11
#define PATCHES_MAX_PER_ENTRY 4

#define FILE_STREAM 0
#define MEMORY_STREAM 1
#define MEMORY_COMPRESSED_STREAM 2


typedef struct {
    char title[64];
    char path[128];
    int key;
    long offset;
    binary_patch patches[PATCHES_MAX_PER_ENTRY];
    int patchesCount;
} boot_entry_s;

typedef struct {
    char path[128];
    int compressed;
    int loopCount;
    int loopReverse;
    int loopStreamType;
    int loopStartFrame;
    int loopTimeOnStartFrame;
    int loopEndFrame;
    int loopTimeOnEndFrame;
} movie_config_s;

typedef struct {
    int timeout;
    int autobootfix;
    int index;
    int recovery;
    int count;
    boot_entry_s entries[CONFIG_MAX_ENTRIES];
    
    u8 bgTop1[3];
    u8 bgTop2[3];
    u8 bgBot[3];
    u8 highlight[4];
    u8 borders[4];
    u8 fntDef[4];
    u8 fntSel[4];
    
    u8 bgTop1AnimColor[3];
    u8 bgTop2AnimColor[3];
    u8 bgBotAnimColor[3];
    u8 highlightAnimColor[4];
    u8 bordersAnimColor[4];
    u8 fntDefAnimColor[4];
    u8 fntSelAnimColor[4];
    
    int bgTop1AnimTime;
    int bgTop2AnimTime;
    int bgBotAnimTime;
    int highlightAnimTime;
    int bordersAnimTime;
    int fntDefAnimTime;
    int fntSelAnimTime;

    int bgTop1AnimTimeStart;
    int bgTop2AnimTimeStart;
    int bgBotAnimTimeStart;
    int highlightAnimTimeStart;
    int bordersAnimTimeStart;
    int fntDefAnimTimeStart;
    int fntSelAnimTimeStart;
    
    int menuFadeInTime;
    int menuFadeInTimeStart;

    char bgImgTop[128];
    char bgImgBot[128];
    bool imgError;
    bool imgErrorBot;
    u8 *bgImgTopBuff;
    u8 *bgImgBotBuff;
    off_t bgImgTopSize;
    off_t bgImgBotSize;

    movie_config_s movieTop;
    movie_config_s movieBot;

} boot_config_s;

boot_config_s *config;

int configInit();

int configAddEntry(char *title, char *path, long offset);

int configRemoveEntry(int index);

void configSave();

void configExit();

void loadBg(gfxScreen_t screen);

#ifdef __cplusplus
}
#endif
#endif // _config_h_
