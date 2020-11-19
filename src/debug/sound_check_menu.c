#if 0 == 0 //DEBUG
#include "global.h"
#include "sprite.h"
#include "graphics.h"
#include "palette.h"
#include "task.h"
#include "m4a.h"
#include "main.h"
#include "text.h"
#include "menu.h"
#include "gpu_regs.h"
#include "bg.h"
#include "text_window.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "title_screen.h"
#include "sound.h"
#include "window.h"
#include "pokedex_cry_screen.h"
#include "international_string_util.h"
#include "debug/sound_check_menu.h"

#define DTR(a,b) _(b)

// why does GF hate 2d arrays
#define MULTI_DIM_ARR(x, dim, y) ((x) * dim + (y))

#define FRAME_TILE_OFFSET 0x1D5

// dim access enums
enum
{
    B_8 = 1,
    B_16 = 2,
    B_32 = 4
};

// local task defines
#define tWindowSelected data[0]
#define tBgmIndex data[1]
#define tSeIndex data[2]

// window selections
enum
{
    MUS_WINDOW,
    SE_WINDOW
};

// window frame selections
enum
{
    WIN_A,
    WIN_B,
    WIN_C,
    CRY_A,
    CRY_B
};

// driver test cry enums
enum
{
    CRY_TEST_VOICE,
    CRY_TEST_VOLUME,
    CRY_TEST_PANPOT,
    CRY_TEST_PITCH,
    CRY_TEST_LENGTH,
    CRY_TEST_RELEASE,
    CRY_TEST_PROGRESS,
    CRY_TEST_CHORUS,
    CRY_TEST_PRIORITY
};

// minmax range enums
enum
{
    MIN,
    MAX
};

extern struct ToneData gCryTable[];
extern struct ToneData gCryTable2[];

static EWRAM_DATA u8 gUnknown_020387B0 = 0;
static EWRAM_DATA u8 gUnknown_020387B1 = 0;
static EWRAM_DATA u8 gUnknown_020387B2 = 0;
static EWRAM_DATA s8 sDriverTestSelection = 0;
static EWRAM_DATA int sSoundTestParams[9] = {0};
static EWRAM_DATA u8 gUnknown_020387D8 = 0;
static EWRAM_DATA u8 gUnknown_020387D9 = 0;

EWRAM_DATA u8 gSoundCheckWindowIds[6] = {0,0,0,0,0,0};
EWRAM_DATA u16 gSoundTestCryNum;

//extern u8 gUnknown_03005E98;
//IWRAM common
extern u8 gDexCryScreenState;

//struct MusicPlayerInfo *gUnknown_3005D30;
extern struct MusicPlayerInfo* gMPlay_PokemonCry;
extern struct MusicPlayerInfo gMPlayInfo_BGM;

static const u16 sSoundCheckBgPal[] = INCBIN_U16("graphics/misc/main_menu_bg.gbapal");
static const u16 sSoundCheckTextPal[] = INCBIN_U16("graphics/misc/main_menu_text.gbapal");

static const struct WindowTemplate sSoundCheckMUSFrame[] =
{
    {
        .bg = 0,
        .tilemapLeft = 3,
        .tilemapTop = 1,
        .width = 24,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    {
        .bg = 0,
        .tilemapLeft = 3,
        .tilemapTop = 6,
        .width = 24,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 49,
    },
    {
        .bg = 0,
        .tilemapLeft = 3,
        .tilemapTop = 13,
        .width = 24,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 145,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct WindowTemplate sSoundCheckSEFrame[] =
{
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 29,
        .height = 19,
        .paletteNum = 15,
        .baseBlock = 49,
    },
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 16,
        .width = 5,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 145,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sSoundCheckBgTemplates[] = {
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 7,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    }
};

#define MUS_FIRST MUS_LITTLEROOT_TEST
#define MUS_STOP (MUS_FIRST - 1)
#define MUS_LAST MUS_RG_TEACHY_TV_MENU

#define PIXEL_PER_UNIT 8

void Task_InitSoundCheckMenu(u8);
void sub_80BA384(u8);
void sub_80BA65C(u8);
void Task_ExitSoundCheck(u8);
void HighlightSelectedWindow(u8);
void PrintSoundNumber(u8, u16, u16, u16);
void sub_80BA79C(u8, const u8 *const, u16, u16);
void Task_DrawDriverTestMenu(u8);
void Task_ProcessDriverTestInput(u8);
void AdjustSelectedDriverParam(s8);
void PrintDriverTestMenuText(void);
void PrintDriverTestMenuCursor(u8, u8);
void PrintSignedNumber(int, u16, u16, u8);
void Task_InitSoundEffectTest(u8);
void Task_ProcessSoundEffectTestInput(u8);
void PrintSoundEffectTestText(void);
void Task_InitCryTest(u8);
void Task_ProcessCryTestInput(u8);
void PrintCryNumber(void);

static void InitSoundCheckScreenWindows(void);
static void LoadSoundCheckWindowFrameTiles(u8 bgId, u16 tileOffset);
static void DrawMainMenuWindowBorder(const struct WindowTemplate *template, u16 baseTileNum);

void CB2_SoundCheckMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

void VBlankCB_SoundCheckMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();

    if (gUnknown_020387B0 != 0)
    {
        m4aSoundMain();
        m4aSoundMain();
        m4aSoundMain();
    }
}

// unused
void CB2_StartSoundCheckMenu(void)
{
    u8 taskId;

    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);
    ResetPaletteFade();
    LoadPalette(sSoundCheckBgPal, 0, 32);
    LoadPalette(gUnknown_0860F074, 0xF0, 0x20);
    ResetTasks();
    ResetSpriteData();
    //Text_LoadWindowTemplate(&gWindowTemplate_81E6C3C);
    //InitMenuWindow(&gMenuTextWindowTemplate);
    //BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB(0, 0, 0));
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0x10, 0, RGB_BLACK);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sSoundCheckBgTemplates, ARRAY_COUNT(sSoundCheckBgTemplates));
    LoadSoundCheckWindowFrameTiles(0, FRAME_TILE_OFFSET);
    ChangeBgX(0, 0, 0);
    ChangeBgY(0, 0, 0);
    ChangeBgX(1, 0, 0);
    ChangeBgY(1, 0, 0);
    InitWindows(sSoundCheckMUSFrame);
    DeactivateAllTextPrinters();
    gSoundCheckWindowIds[WIN_A] = 0;
    gSoundCheckWindowIds[WIN_B] = 1;
    gSoundCheckWindowIds[WIN_C] = 2;
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(0, 0));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(0, 0));
    SetGpuReg(REG_OFFSET_WIN1H, WIN_RANGE(0, 0));
    SetGpuReg(REG_OFFSET_WIN1V, WIN_RANGE(0, 0));
    SetGpuReg(REG_OFFSET_WININ, 0x1111);
    SetGpuReg(REG_OFFSET_WINOUT, 0x31);
    SetGpuReg(REG_OFFSET_BLDCNT, 0xE1);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 7);
    REG_IME = 1; // could be a typo of REG_IME
    REG_IE |= 1;
    SetGpuRegBits(REG_OFFSET_DISPSTAT, 8);
    //REG_DISPSTAT |= 8;
    SetVBlankCallback(VBlankCB_SoundCheckMenu);
    SetMainCallback2(CB2_SoundCheckMenu);
    //REG_DISPCNT = 0x7140;
    SetGpuReg(REG_OFFSET_DISPCNT, 0x7140);
    taskId = CreateTask(Task_InitSoundCheckMenu, 0);
    gTasks[taskId].tWindowSelected = MUS_WINDOW;
    gTasks[taskId].tBgmIndex = 0;
    gTasks[taskId].tSeIndex = 0;
    gTasks[taskId].data[3] = 0;
    gUnknown_020387B0 = 0;
    gTasks[taskId].data[3] = 0; // why?
    m4aSoundInit();
}

void Task_InitSoundCheckMenu(u8 taskId)
{
    u8 soundcheckStr[] = DTR("サウンドチェック", "SOUND CHECK");
    u8 bgmStr[] = _("BGM");
    u8 seStr[] = _("SE ");
    u8 abDescStr[] = DTR("A.さいせい　B.おわり", "A PLAY  B STOP");
    u8 upDownStr[] = _("L..UP R..DOWN");
    u8 driverStr[] = _("R..DRIVER-TEST");
    s16 xx;
    u16 palette;
    struct WindowTemplate winTemp;

    if (!gPaletteFade.active)
    {
        palette = RGB_BLACK;
        LoadPalette(&palette, 254, 2);

        palette = RGB_WHITE;
        LoadPalette(&palette, 250, 2);

        palette = RGB(12, 12, 12);
        LoadPalette(&palette, 251, 2);

        palette = RGB(26, 26, 25);
        LoadPalette(&palette, 252, 2);

        ShowBg(0);
        HideBg(1);
        //Menu_DrawStdWindowFrame(2, 0, 27, 3);
        //winTemp = CreateWindowTemplate(0, 2, 0, 27, 3, 15, 1);
        //gSoundCheckWindowIds[WIN_A] = AddWindow(&winTemp);
        //FillWindowPixelBuffer(gSoundCheckWindowIds[WIN_A], PIXEL_FILL(1));
        //Menu_DrawStdWindowFrame(2, 5, 27, 10);
        //winTemp = CreateWindowTemplate(0, 2, 5, 27, 5, 15, 82);
        //gSoundCheckWindowIds[WIN_B] = AddWindow(&winTemp);
        //FillWindowPixelBuffer(gSoundCheckWindowIds[WIN_B], PIXEL_FILL(1));
        //Menu_DrawStdWindowFrame(2, 12, 27, 17);
        //winTemp = CreateWindowTemplate(0, 2, 5, 27, 5, 15, 217);
        //gSoundCheckWindowIds[WIN_C] = AddWindow(&winTemp);
        //FillWindowPixelBuffer(gSoundCheckWindowIds[WIN_C], PIXEL_FILL(1));
        //DrawStdWindowFrame(gSoundCheckWindowIds[WIN_A], FALSE);
        //DrawStdWindowFrame(gSoundCheckWindowIds[WIN_B], FALSE);
        //DrawStdWindowFrame(gSoundCheckWindowIds[WIN_C], FALSE);
        FillWindowPixelBuffer(0, PIXEL_FILL(0xA));
        FillWindowPixelBuffer(1, PIXEL_FILL(0xA));
        FillWindowPixelBuffer(2, PIXEL_FILL(0xA));

        //FillWindowPixelBuffer(gSoundCheckWindowIds[WIN_A], PIXEL_FILL(0xF));
        //FillWindowPixelBuffer(gSoundCheckWindowIds[WIN_B], PIXEL_FILL(0xF));
        //FillWindowPixelBuffer(gSoundCheckWindowIds[WIN_C], PIXEL_FILL(0xF));

        //Menu_PrintText(soundcheckStr, 4, 1);
        #define WINDOW_WIDTH 24*8

        xx = GetStringRightAlignXOffset(1, abDescStr, WINDOW_WIDTH);
        AddTextPrinterParameterized(gSoundCheckWindowIds[WIN_A], 1, soundcheckStr, 4, 1, TEXT_SPEED_FF, NULL);
        //Menu_PrintText(abDescStr, 14, 1);
        AddTextPrinterParameterized(gSoundCheckWindowIds[WIN_A], 1, abDescStr, xx, 1, TEXT_SPEED_FF, NULL);
        //Menu_PrintText(bgmStr, 4, 6);
        xx = GetStringRightAlignXOffset(1, upDownStr, WINDOW_WIDTH);
        AddTextPrinterParameterized(gSoundCheckWindowIds[WIN_B], 1, bgmStr, 4, 1, TEXT_SPEED_FF, NULL);
        //Menu_PrintText(upDownStr, 14, 6);
        AddTextPrinterParameterized(gSoundCheckWindowIds[WIN_B], 1, upDownStr, xx, 1, TEXT_SPEED_FF, NULL);
        //Menu_PrintText(seStr, 4, 13);
        AddTextPrinterParameterized(gSoundCheckWindowIds[WIN_C], 1, seStr, 4, 1, TEXT_SPEED_FF, NULL);
        //Menu_PrintText(upDownStr, 14, 13);
        AddTextPrinterParameterized(gSoundCheckWindowIds[WIN_C], 1, upDownStr, xx, 1, TEXT_SPEED_FF, NULL);
        //Menu_PrintText(driverStr, 14, 18);
        xx = GetStringRightAlignXOffset(1, driverStr, WINDOW_WIDTH);
        AddTextPrinterParameterized(gSoundCheckWindowIds[WIN_C], 1, driverStr, xx, 36, TEXT_SPEED_FF, NULL);

        #undef WINDOW_WIDTH

        PutWindowTilemap(gSoundCheckWindowIds[MUS_WINDOW]);
        CopyWindowToVram(gSoundCheckWindowIds[MUS_WINDOW], 2);

        PutWindowTilemap(gSoundCheckWindowIds[WIN_B]);
        CopyWindowToVram(gSoundCheckWindowIds[WIN_B], 2);

        PutWindowTilemap(gSoundCheckWindowIds[WIN_C]);
        CopyWindowToVram(gSoundCheckWindowIds[WIN_C], 2);

        gTasks[taskId].func = sub_80BA384;
        SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(17, 223));
        SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(1, 31));
    }
}

// ideally this should be a multi Coords8 struct, but it wont match when its treated like a struct.
static const u8 gUnknown_083D0300[] = { 1, 1, 1, 3, 1, 5, 1, 7, 1, 9, 1, 11, 1, 13, 1, 15, 1, 17 };

extern const u8 *const gBGMNames[];
extern const u8 *const gSENames[];

void sub_80BA384(u8 taskId) // Task_HandleDrawingSoundCheckMenuText
{
    HighlightSelectedWindow(gTasks[taskId].tWindowSelected);
    // PrintSoundNumber(gSoundCheckWindowIds[WIN_B], gTasks[taskId].tBgmIndex + MUS_STOP, 7, 8); // print by BGM index
    // sub_80BA79C(gSoundCheckWindowIds[WIN_B], gBGMNames[gTasks[taskId].tBgmIndex], 11, 8);
    // PrintSoundNumber(gSoundCheckWindowIds[WIN_C], gTasks[taskId].tSeIndex, 7, 15);
    // sub_80BA79C(gSoundCheckWindowIds[WIN_C], gSENames[gTasks[taskId].tSeIndex], 11, 15);
    PrintSoundNumber(gSoundCheckWindowIds[WIN_B], gTasks[taskId].tBgmIndex + MUS_STOP, 14, 16); // print by BGM index
    sub_80BA79C(gSoundCheckWindowIds[WIN_B], gBGMNames[gTasks[taskId].tBgmIndex], 14+(3*8), 16);
    PrintSoundNumber(gSoundCheckWindowIds[WIN_C], gTasks[taskId].tSeIndex, 14, 16);
    sub_80BA79C(gSoundCheckWindowIds[WIN_C], gSENames[gTasks[taskId].tSeIndex], 14+(3*8), 16);
    //PutWindowTilemap(gSoundCheckWindowIds[MUS_WINDOW]);
    CopyWindowToVram(gSoundCheckWindowIds[MUS_WINDOW], 2);

    //PutWindowTilemap(gSoundCheckWindowIds[WIN_B]);
    CopyWindowToVram(gSoundCheckWindowIds[WIN_B], 2);

    //PutWindowTilemap(gSoundCheckWindowIds[WIN_C]);
    CopyWindowToVram(gSoundCheckWindowIds[WIN_C], 2);
    
    DrawMainMenuWindowBorder(&sSoundCheckMUSFrame[0], FRAME_TILE_OFFSET);
    DrawMainMenuWindowBorder(&sSoundCheckMUSFrame[1], FRAME_TILE_OFFSET);
    DrawMainMenuWindowBorder(&sSoundCheckMUSFrame[2], FRAME_TILE_OFFSET);
    gTasks[taskId].func = sub_80BA65C;
}

bool8 Task_ProcessSoundCheckMenuInput(u8 taskId)
{
    if (gMain.newKeys & R_BUTTON) // driver test
    {
        gTasks[taskId].func = Task_DrawDriverTestMenu;
    }
    else if (gMain.newKeys & L_BUTTON)
    {
        gTasks[taskId].func = Task_InitSoundEffectTest;
    }
    //else if (gMain.newKeys & START_BUTTON)
    //{
    //    gTasks[taskId].func = Task_InitCryTest;
    //}
    else if (gMain.newKeys & A_BUTTON)
    {
        if (gTasks[taskId].tWindowSelected != 0) // is playing?
        {
            if (gTasks[taskId].data[4] != 0)
            {
                if (gTasks[taskId].tSeIndex != 0)
                {
                    m4aSongNumStop(gTasks[taskId].data[4]);
                    m4aSongNumStart(gTasks[taskId].tSeIndex);
                    gTasks[taskId].data[4] = gTasks[taskId].tSeIndex;
                }
                else
                {
                    m4aSongNumStop(gTasks[taskId].data[4]);
                    gTasks[taskId].data[4] = 0;
                }
            }
            else if (gTasks[taskId].tSeIndex != 0)
            {
                m4aSongNumStart(gTasks[taskId].tSeIndex);
                gTasks[taskId].data[4] = gTasks[taskId].tSeIndex;
            }
        }
        else
        {
            if (gTasks[taskId].data[3] != 0)
            {
                if (gTasks[taskId].tBgmIndex != 0)
                {
                    m4aSongNumStop(gTasks[taskId].data[3] + MUS_STOP);
                    m4aSongNumStart(gTasks[taskId].tBgmIndex + MUS_STOP);
                    gTasks[taskId].data[3] = gTasks[taskId].tBgmIndex;
                }
                else
                {
                    m4aSongNumStop(gTasks[taskId].data[3] + MUS_STOP);
                    gTasks[taskId].data[3] = 0;
                }
            }
            else if (gTasks[taskId].tBgmIndex != 0)
            {
                m4aSongNumStart(gTasks[taskId].tBgmIndex + MUS_STOP);
                gTasks[taskId].data[3] = gTasks[taskId].tBgmIndex;
            }
        }
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        m4aSongNumStart(SE_SELECT);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB(0, 0, 0));
        gTasks[taskId].func = Task_ExitSoundCheck;
    }
    else if (gMain.newAndRepeatedKeys & DPAD_UP)
    {
        gTasks[taskId].tWindowSelected ^= 1;
        return TRUE;
    }
    else if (gMain.newAndRepeatedKeys & DPAD_DOWN)
    {
        gTasks[taskId].tWindowSelected ^= 1;
        return TRUE;
    }
    else if (gMain.newAndRepeatedKeys & DPAD_RIGHT)
    {
        if (gTasks[taskId].tWindowSelected != 0)
        {
            if (gTasks[taskId].tSeIndex > 0)
                gTasks[taskId].tSeIndex--;
            else
                gTasks[taskId].tSeIndex = 247;
        }
        else
        {
            if (gTasks[taskId].tBgmIndex > 0)
                gTasks[taskId].tBgmIndex--;
            else
                gTasks[taskId].tBgmIndex = (MUS_LAST - MUS_STOP);
        }
        return TRUE;
    }
    else if (gMain.newAndRepeatedKeys & DPAD_LEFT)
    {
        if (gTasks[taskId].tWindowSelected != 0)
        {
            if (gTasks[taskId].tSeIndex < 247)
                gTasks[taskId].tSeIndex++;
            else
                gTasks[taskId].tSeIndex = 0;
        }
        else
        {
            if (gTasks[taskId].tBgmIndex < MUS_LAST - MUS_STOP)
                gTasks[taskId].tBgmIndex++;
            else
                gTasks[taskId].tBgmIndex = 0;
        }
        return TRUE;
    }
    else if (gMain.heldKeys & SELECT_BUTTON)
    {
        gUnknown_020387B0 = 1;
    }
    else
    {
        gUnknown_020387B0 = 0;
    }
    return FALSE;
}

void sub_80BA65C(u8 taskId)
{
    if (Task_ProcessSoundCheckMenuInput(taskId) != FALSE)
        gTasks[taskId].func = sub_80BA384;
}

//sub_80BA68C
void Task_ExitSoundCheck(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        SetMainCallback2(CB2_InitTitleScreen);
    }
}

void HighlightSelectedWindow(u8 windowType)
{
    switch (windowType)
    {
    case MUS_WINDOW:
    default:
        SetGpuReg(REG_OFFSET_WIN1H, WIN_RANGE(17, 223));
        SetGpuReg(REG_OFFSET_WIN1V, WIN_RANGE(41, 87));
        break;
    case SE_WINDOW:
        SetGpuReg(REG_OFFSET_WIN1H, WIN_RANGE(17, 223));
        SetGpuReg(REG_OFFSET_WIN1V, WIN_RANGE(97, 143));
        break;
    }
}

void PrintSoundNumber(u8 windowId, u16 soundIndex, u16 x, u16 y) // PrintSoundNumber ?
{
    u8 i;
    u8 str[5];
    bool8 someBool;
    u8 divisorValue;

    for (i = 0; i < 3; i++)
        str[i] = 0; // initialize array

    str[3] = CHAR_ELLIPSIS;
    str[4] = EOS;
    someBool = FALSE;

    divisorValue = soundIndex / 100;
    if (divisorValue)
    {
        str[0] = divisorValue + CHAR_0;
        someBool = TRUE;
    }

    divisorValue = (soundIndex % 100) / 10;
    if (divisorValue || someBool)
        str[1] = divisorValue + CHAR_0;

    str[2] = ((soundIndex % 100) % 10) + CHAR_0;
    //Menu_PrintText(str, x, y);
    AddTextPrinterParameterized(windowId, 1, str, x, y, TEXT_SPEED_FF, NULL);
}

void sub_80BA79C(u8 windowId, const u8 *const string, u16 x, u16 y)
{
    u8 i;
    u8 str[11];

    for (i = 0; i <= 10; i++)
        str[i] = 0; // format string.

    str[10] = EOS; // the above for loop formats the last element of the array unnecessarily.

    for (i = 0; string[i] != EOS && i < 10; i++)
        str[i] = string[i];

    //Menu_PrintText(str, x, y);
    AddTextPrinterParameterized(windowId, 1, str, x, y, TEXT_SPEED_FF, NULL);
}

void Task_DrawDriverTestMenu(u8 taskId) // Task_DrawDriverTestMenu
{
    u8 bbackStr[] = DTR("Bぼたんで　もどる", "B BUTTON: BACK");
    u8 aplayStr[] = DTR("Aぼたんで　さいせい", "A BUTTON: PLAY");
    u8 voiceStr[] = _("VOICE....");
    u8 volumeStr[] = _("VOLUME...");
    u8 panpotStr[] = _("PANPOT...");
    u8 pitchStr[] = _("PITCH....");
    u8 lengthStr[] = _("LENGTH...");
    u8 releaseStr[] = _("RELEASE..");
    u8 progressStr[] = _("PROGRESS.");
    u8 chorusStr[] = _("CHORUS...");
    u8 priorityStr[] = _("PRIORITY.");
    u8 playingStr[] = DTR("さいせいちゆう.", "PLAYING"); // 再生中 (playing)
    u8 reverseStr[] = DTR("はんてん....", "REVERSE"); // 反転 (reverse)
    u8 stereoStr[] = DTR("すてれお....", "STEREO"); // stereo

    SetGpuReg(REG_OFFSET_DISPCNT, 0x3140);
    //Menu_DrawStdWindowFrame(0, 0, 29, 19);
    LoadSoundCheckWindowFrameTiles(0, FRAME_TILE_OFFSET);

    gSoundCheckWindowIds[CRY_A] = AddWindow(sSoundCheckSEFrame);
    FillWindowPixelBuffer(gSoundCheckWindowIds[CRY_A], PIXEL_FILL(0xA));
    //DrawStdWindowFrame(gSoundCheckWindowIds[CRY_A], TRUE);
    //Menu_PrintText(bbackStr, 19, 4);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, bbackStr, 19*PIXEL_PER_UNIT, 4*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(aplayStr, 19, 2);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, aplayStr, 19*PIXEL_PER_UNIT, 2*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(voiceStr, 2, 1);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, voiceStr, 2*PIXEL_PER_UNIT, 1*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(volumeStr, 2, 3);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, volumeStr, 2*PIXEL_PER_UNIT, 3*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(panpotStr, 2, 5);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, panpotStr, 2*PIXEL_PER_UNIT, 5*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(pitchStr, 2, 7);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, pitchStr, 2*PIXEL_PER_UNIT, 7*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(lengthStr, 2, 9);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, lengthStr, 2*PIXEL_PER_UNIT, 9*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(releaseStr, 2, 11);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, releaseStr, 2*PIXEL_PER_UNIT, 11*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(progressStr, 2, 13);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, progressStr, 2*PIXEL_PER_UNIT, 13*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(chorusStr, 2, 15);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, chorusStr, 2*PIXEL_PER_UNIT, 15*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(priorityStr, 2, 17);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, priorityStr, 2*PIXEL_PER_UNIT, 17*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(playingStr, 19, 16);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, playingStr, 19*PIXEL_PER_UNIT, 16*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(reverseStr, 19, 14);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, reverseStr, 19*PIXEL_PER_UNIT, 14*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(stereoStr, 19, 12);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, stereoStr, 19*PIXEL_PER_UNIT, 12*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(0, DISPLAY_WIDTH));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(0, DISPLAY_HEIGHT));
    sDriverTestSelection = 0;
    gUnknown_020387B1 = 0;
    gUnknown_020387B2 = 0;
    gMPlay_PokemonCry = NULL;
    gUnknown_020387D8 = 0;
    gUnknown_020387D9 = 1;
    sSoundTestParams[CRY_TEST_VOICE] = 0;
    sSoundTestParams[CRY_TEST_VOLUME] = 120;
    sSoundTestParams[CRY_TEST_PANPOT] = 0;
    sSoundTestParams[CRY_TEST_PITCH] = 15360;
    sSoundTestParams[CRY_TEST_LENGTH] = 180;
    sSoundTestParams[CRY_TEST_PROGRESS] = 0;
    sSoundTestParams[CRY_TEST_RELEASE] = 0;
    sSoundTestParams[CRY_TEST_CHORUS] = 0;
    sSoundTestParams[CRY_TEST_PRIORITY] = 2;
    PrintDriverTestMenuText();
    PrintDriverTestMenuCursor(0, 0);
    // CopyWindowToVram(gSoundCheckWindowIds[CRY_A], 3);

    gTasks[taskId].func = Task_ProcessDriverTestInput;
}

void Task_ProcessDriverTestInput(u8 taskId)
{
    if (gMain.newKeys & B_BUTTON)
    {
        SetGpuReg(REG_OFFSET_DISPCNT, 0x7140);
        SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(17, 223));
        SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(1, 31));
        ClearStdWindowAndFrame(gSoundCheckWindowIds[CRY_A], TRUE);
        RemoveWindow(gSoundCheckWindowIds[CRY_A]);
        //Menu_EraseWindowRect(0, 0, 29, 19);
        gTasks[taskId].func = Task_InitSoundCheckMenu;
        return;
    }
    if (gMain.newAndRepeatedKeys & DPAD_UP) // _080BAAA8
    {
        u8 old = sDriverTestSelection;

        if(--sDriverTestSelection < 0)
            sDriverTestSelection = 8;
        PrintDriverTestMenuCursor(old, sDriverTestSelection);
        return;
    }
    if (gMain.newAndRepeatedKeys & DPAD_DOWN) // _080BAAD0
    {
        u8 old = sDriverTestSelection;

        if(++sDriverTestSelection > 8)
            sDriverTestSelection = 0;
        PrintDriverTestMenuCursor(old, sDriverTestSelection);
        return;
    }
    if (gMain.newKeys & START_BUTTON) // _080BAAF8
    {
        gUnknown_020387D8 ^= 1;
        PrintDriverTestMenuText();
        return;
    }
    if (gMain.newKeys & SELECT_BUTTON) // _080BAB14
    {
        gUnknown_020387D9 ^= 1;
        PrintDriverTestMenuText();
        SetPokemonCryStereo(gUnknown_020387D9);
        return;
    }
    if (gMain.newAndRepeatedKeys & R_BUTTON) // _080BAB38
    {
        AdjustSelectedDriverParam(10);
        PrintDriverTestMenuText();
        return;
    }
    if (gMain.newAndRepeatedKeys & L_BUTTON) // _080BAB46
    {
        AdjustSelectedDriverParam(-10);
        PrintDriverTestMenuText();
        return;
    }
    if (gMain.newAndRepeatedKeys & DPAD_LEFT) // _080BAB56
    {
        AdjustSelectedDriverParam(-1);
        PrintDriverTestMenuText();
        return;
    }
    if (gMain.newAndRepeatedKeys & DPAD_RIGHT) // _080BAB64
    {
        AdjustSelectedDriverParam(1);
        PrintDriverTestMenuText();
        return;
    }
    if (gMain.newKeys & A_BUTTON) // _080BAB78
    {
        u8 divide, remaining;

        SetPokemonCryVolume(sSoundTestParams[CRY_TEST_VOLUME]);
        SetPokemonCryPanpot(sSoundTestParams[CRY_TEST_PANPOT]);
        SetPokemonCryPitch(sSoundTestParams[CRY_TEST_PITCH]);
        SetPokemonCryLength(sSoundTestParams[CRY_TEST_LENGTH]);
        SetPokemonCryProgress(sSoundTestParams[CRY_TEST_PROGRESS]);
        SetPokemonCryRelease(sSoundTestParams[CRY_TEST_RELEASE]);
        SetPokemonCryChorus(sSoundTestParams[CRY_TEST_CHORUS]);
        SetPokemonCryPriority(sSoundTestParams[CRY_TEST_PRIORITY]);

        remaining = sSoundTestParams[CRY_TEST_VOICE] % 128;
        divide = sSoundTestParams[CRY_TEST_VOICE] / 128;

        switch (divide)
        {
        case 0:
            if (gUnknown_020387D8)
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable2[(128 * 0) + remaining]);
            else
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable[(128 * 0) + remaining]);
            break;
        case 1:
            if (gUnknown_020387D8)
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable2[(128 * 1) + remaining]);
            else
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable[(128 * 1) + remaining]);
            break;
        case 2:
            if (gUnknown_020387D8)
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable2[(128 * 2) + remaining]);
            else
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable[(128 * 2) + remaining]);
            break;
        case 3:
            if (gUnknown_020387D8)
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable2[(128 * 3) + remaining]);
            else
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable[(128 * 3) + remaining]);
            break;
        case 4:
            if (gUnknown_020387D8)
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable2[(128 * 4) + remaining]);
            else
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable[(128 * 4) + remaining]);
            break;
        case 5:
            if (gUnknown_020387D8)
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable2[(128 * 5) + remaining]);
            else
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable[(128 * 5) + remaining]);
            break;
        case 6:
            if (gUnknown_020387D8)
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable2[(128 * 6) + remaining]);
            else
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable[(128 * 6) + remaining]);
            break;
        case 7:
            if (gUnknown_020387D8)
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable2[(128 * 7) + remaining]);
            else
                gMPlay_PokemonCry = SetPokemonCryTone(&gCryTable[(128 * 7) + remaining]);
            break;
        }
    }

    // _080BACA2
    if (gMPlay_PokemonCry != NULL)
    {
        gUnknown_020387B1 = IsPokemonCryPlaying(gMPlay_PokemonCry);

        if (gUnknown_020387B1 != gUnknown_020387B2)
            PrintDriverTestMenuText();

        gUnknown_020387B2 = gUnknown_020387B1;
    }
}

void AdjustSelectedDriverParam(s8 delta)
{
    // also ideally should be a MinMax struct, but any attempt to make this into a struct causes it to not match due to the weird multi dim access.
    int paramRanges[] =
    {
        0,NUM_SPECIES-2,// Voice
        0, 127,         // Volume
        -127, 127,      // Panpot
        -128, 32639,    // Pitch
        0, 65535,       // Length
        0, 255,         // Release
        0, 65535,       // Progress
        -64, 63         // Chorus
                        // Priority??? Why is it missing?
    };

    sSoundTestParams[sDriverTestSelection] += delta;

    if (sSoundTestParams[sDriverTestSelection] > paramRanges[MULTI_DIM_ARR(sDriverTestSelection, B_16, MAX)])
        sSoundTestParams[sDriverTestSelection] = paramRanges[MULTI_DIM_ARR(sDriverTestSelection, B_16, MIN)];

    if (sSoundTestParams[sDriverTestSelection] < paramRanges[MULTI_DIM_ARR(sDriverTestSelection, B_16, MIN)])
        sSoundTestParams[sDriverTestSelection] = paramRanges[MULTI_DIM_ARR(sDriverTestSelection, B_16, MAX)];
}

void PrintDriverTestMenuText(void)
{
    PrintSignedNumber(sSoundTestParams[CRY_TEST_VOICE] + 1, 11*PIXEL_PER_UNIT, 1*PIXEL_PER_UNIT, 5);
    PrintSignedNumber(sSoundTestParams[CRY_TEST_VOLUME], 11*PIXEL_PER_UNIT, 3*PIXEL_PER_UNIT, 5);
    PrintSignedNumber(sSoundTestParams[CRY_TEST_PANPOT], 11*PIXEL_PER_UNIT, 5*PIXEL_PER_UNIT, 5);
    PrintSignedNumber(sSoundTestParams[CRY_TEST_PITCH], 11*PIXEL_PER_UNIT, 7*PIXEL_PER_UNIT, 5);
    PrintSignedNumber(sSoundTestParams[CRY_TEST_LENGTH], 11*PIXEL_PER_UNIT, 9*PIXEL_PER_UNIT, 5);
    PrintSignedNumber(sSoundTestParams[CRY_TEST_RELEASE], 11*PIXEL_PER_UNIT, 11*PIXEL_PER_UNIT, 5);
    PrintSignedNumber(sSoundTestParams[CRY_TEST_PROGRESS], 11*PIXEL_PER_UNIT, 13*PIXEL_PER_UNIT, 5);
    PrintSignedNumber(sSoundTestParams[CRY_TEST_CHORUS], 11*PIXEL_PER_UNIT, 15*PIXEL_PER_UNIT, 5);
    PrintSignedNumber(sSoundTestParams[CRY_TEST_PRIORITY], 11*PIXEL_PER_UNIT, 17*PIXEL_PER_UNIT, 5);
    PrintSignedNumber(gUnknown_020387B1, 27*PIXEL_PER_UNIT, 16*PIXEL_PER_UNIT, 1);
    PrintSignedNumber(gUnknown_020387D8, 27*PIXEL_PER_UNIT, 14*PIXEL_PER_UNIT, 1);
    PrintSignedNumber(gUnknown_020387D9, 27*PIXEL_PER_UNIT, 12*PIXEL_PER_UNIT, 1);

    PutWindowTilemap(gSoundCheckWindowIds[CRY_A]);
    CopyWindowToVram(gSoundCheckWindowIds[CRY_A], 3);

    DrawMainMenuWindowBorder(sSoundCheckSEFrame, FRAME_TILE_OFFSET);
}

//sub_80BAE10 = PrintDriverTestMenuCursor
void PrintDriverTestMenuCursor(u8 var1, u8 var2)
{
    u8 str1[] = _("▶");
    u8 str2[] = _("  ");

    //Menu_PrintText(str2, gUnknown_083D0300[MULTI_DIM_ARR(var1, B_16, 0)], gUnknown_083D0300[MULTI_DIM_ARR(var1, B_16, 1)]);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, str2, PIXEL_PER_UNIT*gUnknown_083D0300[MULTI_DIM_ARR(var1, B_16, 0)], PIXEL_PER_UNIT*gUnknown_083D0300[MULTI_DIM_ARR(var1, B_16, 1)], TEXT_SPEED_FF, NULL);
    //Menu_PrintText(str1, gUnknown_083D0300[MULTI_DIM_ARR(var2, B_16, 0)], gUnknown_083D0300[MULTI_DIM_ARR(var2, B_16, 1)]);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, str1, PIXEL_PER_UNIT*gUnknown_083D0300[MULTI_DIM_ARR(var2, B_16, 0)], PIXEL_PER_UNIT*gUnknown_083D0300[MULTI_DIM_ARR(var2, B_16, 1)], TEXT_SPEED_FF, NULL);
    
    PutWindowTilemap(gSoundCheckWindowIds[CRY_A]);
    CopyWindowToVram(gSoundCheckWindowIds[CRY_A], 3);

    DrawMainMenuWindowBorder(sSoundCheckSEFrame, FRAME_TILE_OFFSET);
}

void PrintSignedNumber(int n, u16 x, u16 y, u8 digits)
{
    int powersOfTen[6] =
    {
              1,
             10,
            100,
           1000,
          10000,
         100000
    };
    u8 str[8];
    s8 i;
    s8 negative;
    s8 someVar2;

    for (i = 0; i <= digits; i++)
        str[i] = CHAR_SPACE;
    str[digits + 1] = EOS;

    negative = FALSE;
    if (n < 0)
    {
        n = -n;
        negative = TRUE;
    }

    if (digits == 1)
        someVar2 = TRUE;
    else
        someVar2 = FALSE;

    for (i = digits - 1; i >= 0; i--)
    {
        s8 d = n / powersOfTen[i];

        if (d != 0 || someVar2 || i == 0)
        {
            if (negative && !someVar2)
                str[digits - i - 1] = CHAR_HYPHEN;
            str[digits - i] = CHAR_0 + d;
            someVar2 = TRUE;
        }
        n %= powersOfTen[i];
    }

    //Menu_PrintText(str, x, y);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, str, x, y, TEXT_SPEED_FF, NULL);
}

static const s8 gUnknown_083D03F8[5] = { 0x3F, 0x00, 0xC0, 0x7F, 0x80 };

// sub_80BAF84 = Task_InitSoundEffectTest
void Task_InitSoundEffectTest(u8 taskId)
{
    u8 seStr[] = _("SE");
    u8 panStr[] = _("PAN");
    u8 playingStr[] = DTR("さいせいちゆう.", "PLAYING");

    SetGpuReg(REG_OFFSET_DISPCNT, 0x3140);
    //Menu_DrawStdWindowFrame(0, 0, 29, 19);
    //InitWindows(sSoundCheckSEFrame);
    //gSoundCheckWindowIds[SE_WINDOW] = 1;
    gSoundCheckWindowIds[CRY_A] = AddWindow(sSoundCheckSEFrame);
    DrawStdWindowFrame(gSoundCheckWindowIds[CRY_A], FALSE);
    //Menu_PrintText(seStr, 3, 2);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, seStr, 3*PIXEL_PER_UNIT, 2*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(panStr, 3, 4);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, panStr, 3*PIXEL_PER_UNIT, 4*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    //Menu_PrintText(playingStr, 3, 8);
    AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, playingStr, 3*PIXEL_PER_UNIT, 8*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(0, DISPLAY_WIDTH));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(0, DISPLAY_HEIGHT));
    sSoundTestParams[CRY_TEST_VOICE] = 1;
    sSoundTestParams[CRY_TEST_PANPOT] = 0;
    sSoundTestParams[CRY_TEST_CHORUS] = 0;
    sSoundTestParams[CRY_TEST_PROGRESS] = 0;
    sSoundTestParams[CRY_TEST_RELEASE] = 0;
    PrintSoundEffectTestText();
    gTasks[taskId].func = Task_ProcessSoundEffectTestInput;
}

//sub_80BB038 = Task_ProcessSoundEffectTestInput
void Task_ProcessSoundEffectTestInput(u8 taskId)
{
    PrintSoundEffectTestText();
    if (sSoundTestParams[CRY_TEST_PROGRESS])
    {
        if (sSoundTestParams[CRY_TEST_RELEASE])
        {
            sSoundTestParams[CRY_TEST_RELEASE]--;
        }
        else // _080BB05C
        {
            s8 panpot = gUnknown_083D03F8[sSoundTestParams[CRY_TEST_PANPOT]];
            if (panpot != -128)
            {
                if (panpot == 127)
                {
                    sSoundTestParams[CRY_TEST_CHORUS] += 2;
                    if (sSoundTestParams[CRY_TEST_CHORUS] < 63)
                        SE12PanpotControl(sSoundTestParams[CRY_TEST_CHORUS]);
                }
            }
            else // _080BB08C
            {
                sSoundTestParams[CRY_TEST_CHORUS] -= 2;
                if (sSoundTestParams[CRY_TEST_CHORUS] > -64)
                    SE12PanpotControl(sSoundTestParams[CRY_TEST_CHORUS]);
            }
        }
    }
     // _080BB0A2
    if (gMain.newKeys & B_BUTTON)
    {
        SetGpuReg(REG_OFFSET_DISPCNT, 0x7140);
        SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(17, 223));
        SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(1, 31));
        ClearStdWindowAndFrame(gSoundCheckWindowIds[CRY_A], TRUE);
        RemoveWindow(gSoundCheckWindowIds[CRY_A]);
        //Menu_EraseWindowRect(0, 0, 29, 19);
        gTasks[taskId].func = Task_InitSoundCheckMenu;
        return;
    }
    if (gMain.newKeys & A_BUTTON) // _080BB104
    {
        s8 panpot = gUnknown_083D03F8[sSoundTestParams[CRY_TEST_PANPOT]];
        if (panpot != -128)
        {
            if (panpot == 127)
            {
                PlaySE12WithPanning(sSoundTestParams[CRY_TEST_VOICE], -64);
                sSoundTestParams[CRY_TEST_CHORUS] = -64;
                sSoundTestParams[CRY_TEST_PROGRESS] = 1;
                sSoundTestParams[CRY_TEST_RELEASE] = 30;
                return;
            }
        }
        else // _080BB140
        {
            PlaySE12WithPanning(sSoundTestParams[CRY_TEST_VOICE], 63);
            sSoundTestParams[CRY_TEST_CHORUS] = 63;
            sSoundTestParams[CRY_TEST_PROGRESS] = 1;
            sSoundTestParams[CRY_TEST_RELEASE] = 30;
            return;
        }
        // _080BB154
        PlaySE12WithPanning(sSoundTestParams[CRY_TEST_VOICE], panpot);
        sSoundTestParams[CRY_TEST_PROGRESS] = 0;
        return;
    }
    if (gMain.newKeys & L_BUTTON) // _080BB15E
    {
        sSoundTestParams[CRY_TEST_PANPOT]++;
        if (sSoundTestParams[CRY_TEST_PANPOT] > 4)
            sSoundTestParams[CRY_TEST_PANPOT] = 0;
    }
    if (gMain.newKeys & R_BUTTON) // _080BB176
    {
        sSoundTestParams[CRY_TEST_PANPOT]--;
        if (sSoundTestParams[CRY_TEST_PANPOT] < 0)
            sSoundTestParams[CRY_TEST_PANPOT] = 4;
    }
    if (gMain.newAndRepeatedKeys & DPAD_RIGHT) // _080BB192
    {
        sSoundTestParams[CRY_TEST_VOICE]++;
        if (sSoundTestParams[CRY_TEST_VOICE] > 247)
            sSoundTestParams[CRY_TEST_VOICE] = 0;
    }
    else if (gMain.newAndRepeatedKeys & DPAD_LEFT) // _080BB1B0
    {
        sSoundTestParams[CRY_TEST_VOICE]--;
        if (sSoundTestParams[CRY_TEST_VOICE] < 0)
            sSoundTestParams[CRY_TEST_VOICE] = 247;
    }
}

//sub_80BB1D4 = PrintSoundEffectTestText
void PrintSoundEffectTestText(void)
{
    u8 lrStr[] = _("  LR");
    u8 rlStr[] = _("  RL");

    PrintSignedNumber(sSoundTestParams[CRY_TEST_VOICE], 7*PIXEL_PER_UNIT, 2*PIXEL_PER_UNIT, 3);

    switch (gUnknown_083D03F8[sSoundTestParams[CRY_TEST_PANPOT]])
    {
    case 127:
        //Menu_PrintText(lrStr, 7, 4);
        AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, lrStr, 7*PIXEL_PER_UNIT, 4*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
        break;
    case -128:
        //Menu_PrintText(rlStr, 7, 4);
        AddTextPrinterParameterized(gSoundCheckWindowIds[CRY_A], 1, rlStr, 7*PIXEL_PER_UNIT, 4*PIXEL_PER_UNIT, TEXT_SPEED_FF, NULL);
        break;
    default:
        PrintSignedNumber(gUnknown_083D03F8[sSoundTestParams[CRY_TEST_PANPOT]], 7*PIXEL_PER_UNIT, 4*PIXEL_PER_UNIT, 3);
        break;
    }
    PrintSignedNumber(IsSEPlaying(), 12*PIXEL_PER_UNIT, 8*PIXEL_PER_UNIT, 1);

    PutWindowTilemap(gSoundCheckWindowIds[CRY_A]);
    CopyWindowToVram(gSoundCheckWindowIds[CRY_A], 3);
}

#define SOUND_LIST_BGM \
	X(MUS_STOP, "STOP") \
	X(MUS_LITTLEROOT_TEST, "LITTLEROOT-TEST") \
	X(MUS_GSC_ROUTE38, "GSC-ROUTE38") \
	X(MUS_CAUGHT, "CAUGHT") \
	X(MUS_VICTORY_WILD, "VICTORY-WILD") \
	X(MUS_VICTORY_GYM_LEADER, "VICTORY-GYM") \
	X(MUS_VICTORY_LEAGUE, "VICTORY-LEAGUE") \
	X(MUS_C_COMM_CENTER, "C-COMM-CENTER") \
	X(MUS_GSC_PEWTER, "GSC-PEWTER") \
	X(MUS_C_VS_LEGEND_BEAST, "C-VS-LEGEND-BEAST") \
	X(MUS_ROUTE101, "ROUTE101") \
	X(MUS_ROUTE110, "ROUTE110") \
	X(MUS_ROUTE120, "ROUTE120") \
	X(MUS_PETALBURG, "PETALBURG") \
	X(MUS_OLDALE, "OLDALE") \
	X(MUS_GYM, "GYM") \
	X(MUS_SURF, "SURF") \
	X(MUS_PETALBURG_WOODS, "PETALBURG-WOODS") \
	X(MUS_LEVEL_UP, "LEVEL-UP") \
	X(MUS_HEAL, "HEAL") \
	X(MUS_OBTAIN_BADGE, "OBTAIN-BADGE") \
	X(MUS_OBTAIN_ITEM, "OBTAIN-ITEM") \
	X(MUS_EVOLVED, "EVOLVED") \
	X(MUS_OBTAIN_TMHM, "OBTAIN-TMHM") \
	X(MUS_LILYCOVE_MUSEUM, "LILYCOVE-MUSEUM") \
	X(MUS_ROUTE122, "ROUTE122") \
	X(MUS_OCEANIC_MUSEUM, "OCEANIC-MUSEUM") \
	X(MUS_EVOLUTION_INTRO, "EVOLUTION-INTRO") \
	X(MUS_EVOLUTION, "EVOLUTION") \
	X(MUS_MOVE_DELETED, "MOVE-DELETED") \
	X(MUS_ENCOUNTER_GIRL, "ENCOUNTER-GIRL") \
	X(MUS_ENCOUNTER_MALE, "ENCOUNTER-MALE") \
	X(MUS_ABANDONED_SHIP, "ABANDONED-SHIP") \
	X(MUS_FORTREE, "FORTREE") \
	X(MUS_BIRCH_LAB, "BIRCH-LAB") \
	X(MUS_B_TOWER_RS, "B-TOWER-RS") \
	X(MUS_ENCOUNTER_SWIMMER, "ENCOUNTER-SWIMER") \
	X(MUS_CAVE_OF_ORIGIN, "CAVE-OF-ORIGIN") \
	X(MUS_OBTAIN_BERRY, "OBTAIN-BERRY") \
	X(MUS_AWAKEN_LEGEND, "AWAKEN-LEGEND") \
	X(MUS_SLOTS_JACKPOT, "SLOTS-JACKPOT") \
	X(MUS_SLOTS_WIN, "SLOTS-WIN") \
	X(MUS_TOO_BAD, "TOO-BAD") \
	X(MUS_ROULETTE, "ROULETTE") \
	X(MUS_LINK_CONTEST_P1, "LINK-CONTEST-P1") \
	X(MUS_LINK_CONTEST_P2, "LINK-CONTEST-P2") \
	X(MUS_LINK_CONTEST_P3, "LINK-CONTEST-P3") \
	X(MUS_LINK_CONTEST_P4, "LINK-CONTEST-P4") \
	X(MUS_ENCOUNTER_RICH, "ENCOUNTER-RICH") \
	X(MUS_VERDANTURF, "VERDANTURF") \
	X(MUS_RUSTBORO, "RUSTBORO") \
	X(MUS_POKE_CENTER, "POKE-CENTER") \
	X(MUS_ROUTE104, "ROUTE104") \
	X(MUS_ROUTE119, "ROUTE119") \
	X(MUS_CYCLING, "CYCLING") \
	X(MUS_POKE_MART, "POKE-MART") \
	X(MUS_LITTLEROOT, "LITTLEROOT") \
	X(MUS_MT_CHIMNEY, "MT-CHIMNEY") \
	X(MUS_ENCOUNTER_FEMALE, "ENCOUNTER-FEMALE") \
	X(MUS_LILYCOVE, "LILYCOVE") \
	X(MUS_ROUTE111, "ROUTE111") \
	X(MUS_HELP, "HELP!") \
	X(MUS_UNDERWATER, "UNDERWATER") \
	X(MUS_VICTORY_TRAINER, "VICTORY-TRAINER") \
	X(MUS_TITLE, "TITLE") \
	X(MUS_INTRO, "INTRO") \
	X(MUS_ENCOUNTER_MAY, "ENCOUNTER-MAY") \
	X(MUS_ENCOUNTER_INTENSE, "ENCOUNTER-INTENSE") \
	X(MUS_ENCOUNTER_COOL, "ENCOUNTER-COOL") \
	X(MUS_ROUTE113, "ROUTE-113") \
	X(MUS_ENCOUNTER_AQUA, "ENCOUNTER-AQUA") \
	X(MUS_FOLLOW_ME, "FOLLOW-ME") \
	X(MUS_ENCOUNTER_BRENDAN, "ENCOUNTER-BRENDAN") \
	X(MUS_EVER_GRANDE, "EVER-GRANDE") \
	X(MUS_ENCOUNTER_SUSPICIOUS, "ENCOUNTER-SUSPICIOUS") \
	X(MUS_VICTORY_AQUA_MAGMA, "VICTORY-AQUA-MAGMA") \
	X(MUS_CABLE_CAR, "CABLE-CAR") \
	X(MUS_GAME_CORNER, "GAME-CORNER") \
	X(MUS_DEWFORD, "DEWFORD") \
	X(MUS_SAFARI_ZONE, "SAFARI-ZONE") \
	X(MUS_VICTORY_ROAD, "VICTORY-ROAD") \
	X(MUS_AQUA_MAGMA_HIDEOUT, "AQUA-MAGMA-HIDEOUT") \
	X(MUS_SAILING, "SAILING") \
	X(MUS_MT_PYRE, "MT-PYRE") \
	X(MUS_SLATEPORT, "SLATEPORT") \
	X(MUS_MT_PYRE_EXTERIOR, "MT-PYRE-EXTERIOR") \
	X(MUS_SCHOOL, "SCHOOL") \
	X(MUS_HALL_OF_FAME, "HALL-OF-FAME") \
	X(MUS_FALLARBOR, "FALLARBOR") \
	X(MUS_SEALED_CHAMBER, "SEALED-CHAMBER") \
	X(MUS_CONTEST_WINNER, "CONTEST-WINNER") \
	X(MUS_CONTEST, "CONTEST") \
	X(MUS_ENCOUNTER_MAGMA, "ENCOUNTER-MAGMA") \
	X(MUS_INTRO_BATTLE, "INTRO-BATTLE") \
	X(MUS_ABNORMAL_WEATHER, "ABNORMAL-WEATHER") \
	X(MUS_WEATHER_GROUDON, "WEATHER-GROUDON") \
	X(MUS_SOOTOPOLIS, "SOOTOPOLIS") \
	X(MUS_CONTEST_RESULTS, "CONTEST-RESULTS") \
	X(MUS_HALL_OF_FAME_ROOM, "EIKOU-R") \
	X(MUS_TRICK_HOUSE, "TRICK-HOUSE") \
	X(MUS_ENCOUNTER_TWINS, "ENCOUNTER-TWINS") \
	X(MUS_ENCOUNTER_ELITE_FOUR, "ENCOUNTER-ELITE-FOUR") \
	X(MUS_ENCOUNTER_HIKER, "ENCOUNTER-HIKER") \
	X(MUS_CONTEST_LOBBY, "CONTEST-LOBBY") \
	X(MUS_ENCOUNTER_INTERVIEWER, "ENCOUNTER-INTERVIEWER") \
	X(MUS_ENCOUNTER_CHAMPION, "ENCOUNTER-CHAMPION") \
	X(MUS_CREDITS, "CREDITS") \
	X(MUS_END, "END") \
X(MUS_B_FRONTIER, "B-FRONTIER") \
X(MUS_B_ARENA, "B-ARENA") \
X(MUS_OBTAIN_B_POINTS, "OBTAIN-B-POINTS") \
X(MUS_REGISTER_MATCH_CALL, "REGISTER-MATCH-CALL") \
X(MUS_B_PYRAMID, "B-PYRAMID") \
X(MUS_B_PYRAMID_TOP, "B-PYRAMID-TOP") \
X(MUS_B_PALACE, "B-PALACE") \
X(MUS_RAYQUAZA_APPEARS, "RAYQUAZA-APPEARS") \
X(MUS_B_TOWER, "B-TOWER") \
X(MUS_OBTAIN_SYMBOL, "OBTAIN-SYMBOL") \
X(MUS_B_DOME, "B-DOME") \
X(MUS_B_PIKE, "B-PIKE") \
X(MUS_B_FACTORY, "B-FACTORY") \
X(MUS_VS_RAYQUAZA, "VS-RAYQUAZA") \
X(MUS_VS_FRONTIER_BRAIN, "VS-FRONTIER-BRAIN") \
X(MUS_VS_MEW, "VS-MEW") \
    X(MUS_B_DOME_LOBBY, "B-DOME-LOBBY") \
	X(MUS_VS_WILD, "VS-WILD") \
	X(MUS_VS_AQUA_MAGMA, "VS-AQUA-MAGMA") \
	X(MUS_VS_TRAINER, "VS-TRAINER") \
	X(MUS_VS_GYM_LEADER, "VS-GYM-LEADER") \
	X(MUS_VS_CHAMPION, "VS-CHAMPION") \
	X(MUS_VS_REGI, "VS-REGI") \
	X(MUS_VS_KYOGRE_GROUDON, "VS-KYOGRE-GROUDON") \
	X(MUS_VS_RIVAL, "VS-RIVAL") \
	X(MUS_VS_ELITE_FOUR, "VS-ELITE-FOUR") \
	X(MUS_VS_AQUA_MAGMA_LEADER, "VS-AQUA-MAGMA-LEADER") \
X(MUS_RG_FOLLOW_ME, "RG-FOLLOW-ME") \
X(MUS_RG_GAME_CORNER, "RG-GAME-CORNER") \
X(MUS_RG_ROCKET_HIDEOUT, "RG-AJITO") \
X(MUS_RG_GYM, "RG-GYM") \
X(MUS_RG_JIGGLYPUFF, "RG-JIGGLYPUFF") \
X(MUS_RG_INTRO_FIGHT, "RG-INTRO-FIGHT") \
X(MUS_RG_TITLE, "RG-TITLE") \
X(MUS_RG_CINNABAR, "RG-CINNABAR") \
X(MUS_RG_LAVENDER, "RG-LAVENDER") \
X(MUS_RG_HEAL, "RG-HEAL") \
X(MUS_RG_CYCLING, "RG-CYCLING") \
X(MUS_RG_ENCOUNTER_ROCKET, "RG-ENCOUNTER-ROCKET") \
X(MUS_RG_ENCOUNTER_GIRL, "RG-ENCOUNTER-GIRL") \
X(MUS_RG_ENCOUNTER_BOY, "RG-ENCOUNTER-BOY") \
X(MUS_RG_HALL_OF_FAME, "RG-HALL-OF-FAME") \
X(MUS_RG_VIRIDIAN_FOREST, "RG-VIRIDIAN-FOREST") \
X(MUS_RG_MT_MOON, "RG-MT-MOON") \
X(MUS_RG_POKE_MANSION, "RG-POKE-MANSION") \
X(MUS_RG_CREDITS, "RG-CREDITS") \
X(MUS_RG_ROUTE1, "RG-ROUTE1") \
X(MUS_RG_ROUTE24, "RG-ROUTE24") \
X(MUS_RG_ROUTE3, "RG-ROUTE3") \
X(MUS_RG_ROUTE11, "RG-ROUTE11") \
X(MUS_RG_VICTORY_ROAD, "RG-VICTORY-ROAD") \
X(MUS_RG_VS_GYM_LEADER, "RG-VS-GYM-LEADER") \
X(MUS_RG_VS_TRAINER, "RG-VS-TRAINER") \
X(MUS_RG_VS_WILD, "RG-VS-WILD") \
X(MUS_RG_VS_CHAMPION, "RG-VS-CHAMPION") \
X(MUS_RG_PALLET, "RG-PALLET") \
X(MUS_RG_OAK_LAB, "RG-OAK-LAB") \
X(MUS_RG_OAK, "RG-OAK") \
X(MUS_RG_POKE_CENTER, "RG-POKE-CENTER") \
X(MUS_RG_SS_ANNE, "RG-SS-ANNE") \
X(MUS_RG_SURF, "RG-SURF") \
X(MUS_RG_POKE_TOWER, "RG-POKE-TOWER") \
X(MUS_RG_SILPH, "RG-SILPH") \
X(MUS_RG_FUCHSIA, "RG-FUCHSIA") \
X(MUS_RG_CELADON, "RG-CELADON") \
X(MUS_RG_VICTORY_TRAINER, "RG-VICTORY-TRAINER") \
X(MUS_RG_VICTORY_WILD, "RG-VICTORY-WILD") \
X(MUS_RG_VICTORY_GYM_LEADER, "RG-VICTORY-GYM-LEADER") \
X(MUS_RG_VERMILLION, "RG-VERMILLION") \
X(MUS_RG_PEWTER, "RG-PEWTER") \
X(MUS_RG_ENCOUNTER_RIVAL, "RG-ENCOUNTER-RIVAL") \
X(MUS_RG_RIVAL_EXIT, "RG-RIVAL-EXIT") \
X(MUS_RG_DEX_RATING, "RG-DEX-RATING") \
X(MUS_RG_OBTAIN_KEY_ITEM, "RG-OBTAIN-KEY-ITEM") \
X(MUS_RG_CAUGHT_INTRO, "RG-CAUGHT-INTRO") \
X(MUS_RG_PHOTO, "RG-PHOTO") \
X(MUS_RG_GAME_FREAK, "RG-GAME-FREAK") \
X(MUS_RG_CAUGHT, "RG-CAUGHT") \
X(MUS_RG_NEW_GAME_INSTRUCT, "RG-NEW-GAME-INSTRUCT") \
X(MUS_RG_NEW_GAME_INTRO, "RG-NEW-GAME-INTRO") \
X(MUS_RG_NEW_GAME_EXIT, "RG-NEW-GAME-EXIT") \
X(MUS_RG_POKE_JUMP, "RG-JUMP") \
X(MUS_RG_UNION_ROOM, "RG-UNION-ROOM") \
X(MUS_RG_NET_CENTER, "RG-NET-CENTER") \
X(MUS_RG_MYSTERY_GIFT, "RG-MYSTERY-GIFT") \
X(MUS_RG_BERRY_PICK, "RG-BERRY-PICK") \
X(MUS_RG_SEVII_CAVE, "RG-SEVII-CAVE") \
X(MUS_RG_TEACHY_TV_SHOW, "RG-TEACHY-TV-SHOW") \
X(MUS_RG_SEVII_ROUTE, "NANASHIMA") \
X(MUS_RG_SEVII_DUNGEON, "NANAISEKI") \
X(MUS_RG_SEVII_123, "RG-SEVII-123") \
X(MUS_RG_SEVII_45, "RG-SEVII-45") \
X(MUS_RG_SEVII_67, "RG-SEVII-67") \
X(MUS_RG_POKE_FLUTE, "RG-POKE-FLUTE") \
X(MUS_RG_VS_DEOXYS, "RG-VS-DEOXYS") \
X(MUS_RG_VS_MEWTWO, "RG-VS-MEWTWO") \
X(MUS_RG_VS_LEGEND, "RG-VS-LEGEND") \
X(MUS_RG_ENCOUNTER_GYM_LEADER, "RG-ENCOUNTER-GYM-LEADER") \
X(MUS_RG_ENCOUNTER_DEOXYS, "RG-ENCOUNTER-DEOXYS") \
X(MUS_RG_TRAINER_TOWER, "RG-TRAINER-TOWER") \
X(MUS_RG_SLOW_PALLET, "RG-SLOW-PALLET") \
X(MUS_RG_TEACHY_TV_MENU, "RG-TEACHY-TV-MENU")

#define SOUND_LIST_SE \
	X(SE_STOP, "STOP") \
	X(SE_USE_ITEM, "USE-ITEM") \
	X(SE_PC_LOGIN, "PC-LOGIN") \
	X(SE_PC_OFF, "PC-OFF") \
	X(SE_PC_ON, "PC-ON") \
	X(SE_SELECT, "SELECT") \
	X(SE_WIN_OPEN, "WIN-OPEN") \
	X(SE_WALL_HIT, "WALL-HIT") \
	X(SE_DOOR, "DOOR") \
	X(SE_EXIT, "EXIT") \
	X(SE_LEDGE, "LEDGE") \
	X(SE_BIKE_BELL, "BIKE-BELL") \
	X(SE_NOT_EFFECTIVE, "NOT-EFFECTIVE") \
	X(SE_EFFECTIVE, "EFFECTIVE") \
	X(SE_SUPER_EFFECTIVE, "SUPER-EFFECTIVE") \
	X(SE_BALL_OPEN, "BALL-OPEN") \
	X(SE_FAINT, "FAINT") \
	X(SE_FLEE, "FLEE") \
	X(SE_SLIDING_DOOR, "SLIDING-DOOR") \
	X(SE_SHIP, "SHIP") \
	X(SE_BANG, "BANG") \
	X(SE_PIN, "PIN") \
	X(SE_BOO, "BOO") \
	X(SE_BALL, "BALL") \
	X(SE_CONTEST_PLACE, "CONTEST-PLACE") \
	X(SE_A, "A") \
	X(SE_I, "I") \
	X(SE_U, "U") \
	X(SE_E, "E") \
	X(SE_O, "O") \
	X(SE_N, "N") \
	X(SE_SUCCESS, "SUCCESS") \
	X(SE_FAILURE, "FAILURE") \
	X(SE_EXP, "EXP") \
	X(SE_BIKE_HOP, "BIKE-HOP") \
	X(SE_SWITCH, "SWITCH") \
	X(SE_CLICK, "CLICK") \
	X(SE_FU_ZAKU, "FU-ZAKU") \
	X(SE_CONTEST_CONDITION_LOSE, "CONTEST-CONDITION-LOSE") \
	X(SE_LAVARIDGE_FALL_WARP, "LAVARIDGE-FALL-WARP") \
	X(SE_ICE_STAIRS, "ICE-STAIRS") \
	X(SE_ICE_BREAK, "ICE-BREAK") \
	X(SE_ICE_CRACK, "ICE-CRACK") \
	X(SE_FALL, "FALL") \
	X(SE_UNLOCK, "UNLOCK") \
	X(SE_WARP_IN, "WARP-IN") \
	X(SE_WARP_OUT, "WARP-OUT") \
	X(SE_REPEL, "TU-SAA") \
	X(SE_ROTATING_GATE, "ROTATING-GATE") \
	X(SE_TRUCK_MOVE, "TRUCK-MOVE") \
	X(SE_TRUCK_STOP, "TRUCK-STOP") \
	X(SE_TRUCK_UNLOAD, "TRUCK-UNLOAD") \
	X(SE_TRUCK_DOOR, "TRUCK-DOOR") \
	X(SE_BERRY_BLENDER, "BERRY-BLENDER") \
	X(SE_CARD, "CARD") \
	X(SE_SAVE, "SAVE") \
	X(SE_BALL_BOUNCE_1, "BALL-BOUNCE-1") \
	X(SE_BALL_BOUNCE_2, "BALL-BOUNCE-2") \
	X(SE_BALL_BOUNCE_3, "BALL-BOUNCE-3") \
	X(SE_BALL_BOUNCE_4, "BALL-BOUNCE-4") \
	X(SE_BALL_TRADE, "BALL-TRADE") \
	X(SE_BALL_THROW, "BALL-THROW") \
	X(SE_NOTE_C, "NOTE-C") \
	X(SE_NOTE_D, "NOTE-D") \
	X(SE_NOTE_E, "NOTE-E") \
	X(SE_NOTE_F, "NOTE-F") \
	X(SE_NOTE_G, "NOTE-G") \
	X(SE_NOTE_A, "NOTE-A") \
	X(SE_NOTE_B, "NOTE-B") \
	X(SE_NOTE_C_HIGH, "NOTE-C-HIGH") \
	X(SE_PUDDLE, "PUDDLE") \
	X(SE_BRIDGE_WALK, "BRIDGE-WALK") \
	X(SE_ITEMFINDER, "ITEMFINDER") \
	X(SE_DING_DONG, "DING-DONG") \
	X(SE_BALLOON_RED, "BALLOON-RED") \
	X(SE_BALLOON_BLUE, "BALLOON-BLUE") \
	X(SE_BALLOON_YELLOW, "BALLOON-YELLOW") \
	X(SE_BREAKABLE_DOOR, "BREAKABLE-DOOR") \
	X(SE_MUD_BALL, "MUD-BALL") \
	X(SE_FIELD_POISON, "FIELD-POISON") \
	X(SE_ESCALATOR, "ESCALATOR") \
	X(SE_THUNDERSTORM, "THUNDERSTORM") \
	X(SE_THUNDERSTORM_STOP, "THUNDERSTORM-STOP") \
	X(SE_DOWNPOUR, "DOWNPOUR") \
	X(SE_DOWNPOUR_STOP, "DOWNPOUR-STOP") \
	X(SE_RAIN, "RAIN") \
	X(SE_RAIN_STOP, "RAIN-STOP") \
	X(SE_THUNDER, "THUNDER") \
	X(SE_THUNDER2, "THUNDER2") \
	X(SE_ELEVATOR, "ELEVATOR") \
	X(SE_LOW_HEALTH, "LOW-HEALTH") \
	X(SE_EXP_MAX, "EXP-MAX") \
	X(SE_ROULETTE_BALL, "ROULETTE-BALL") \
	X(SE_ROULETTE_BALL2, "ROULETTE-BALL2") \
	X(SE_TAILLOW_WING_FLAP, "TAILLOW-WING-FLAP") \
	X(SE_SHOP, "SHOP") \
	X(SE_CONTEST_HEART, "CONTEST-HEART") \
	X(SE_CONTEST_CURTAIN_RISE, "CONTEST-CURTAIN-RISE") \
	X(SE_CONTEST_CURTAIN_FALL, "CONTEST-CURTAIN-FALL") \
	X(SE_CONTEST_ICON_CHANGE, "CONTEST-ICON-CHANGE") \
	X(SE_CONTEST_ICON_CLEAR, "CONTEST-ICON-CLEAR") \
	X(SE_CONTEST_MONS_TURN, "CONTEST-MONS-TURN") \
	X(SE_SHINY, "SHINY") \
	X(SE_INTRO_BLAST, "INTRO-BLAST") \
	X(SE_MUGSHOT, "MUGSHOT") \
	X(SE_APPLAUSE, "APPLAUSE") \
	X(SE_VEND, "VEND") \
	X(SE_ORB, "ORB") \
	X(SE_DEX_SCROLL, "DEX-SCROLL") \
	X(SE_DEX_PAGE, "DEX-PAGE") \
	X(SE_POKENAV_ON, "POKENAV-ON") \
	X(SE_POKENAV_OFF, "POKENAV-OFF") \
	X(SE_DEX_SEARCH, "DEX-SEARCH") \
	X(SE_EGG_HATCH, "TAMAGO") \
	X(SE_BALL_TRAY_ENTER, "BALL-TRAY-START") \
	X(SE_BALL_TRAY_BALL, "BALL-TRAY-BALL") \
	X(SE_BALL_TRAY_EXIT, "BALL-TRAY-END") \
	X(SE_GLASS_FLUTE, "GLASS-FLUTE") \
	X(SE_M_THUNDERBOLT, "W085") \
	X(SE_M_THUNDERBOLT2, "W085B") \
	X(SE_M_HARDEN, "W231") \
	X(SE_M_NIGHTMARE, "W171") \
	X(SE_M_VITAL_THROW, "W233") \
	X(SE_M_VITAL_THROW2, "W233B") \
	X(SE_M_BUBBLE, "W145") \
	X(SE_M_BUBBLE2, "W145B") \
	X(SE_M_BUBBLE3, "W145C") \
	X(SE_M_RAIN_DANCE, "W240") \
	X(SE_M_CUT, "W015") \
	X(SE_M_STRING_SHOT, "W081") \
	X(SE_M_STRING_SHOT2, "W081B") \
	X(SE_M_ROCK_THROW, "W088") \
	X(SE_M_GUST, "W016") \
	X(SE_M_GUST2, "W016B") \
	X(SE_M_DOUBLE_SLAP, "W003") \
	X(SE_M_DOUBLE_TEAM, "W104") \
	X(SE_M_RAZOR_WIND, "W013") \
	X(SE_M_ICY_WIND, "W196") \
	X(SE_M_THUNDER_WAVE, "W086") \
	X(SE_M_COMET_PUNCH, "W004") \
	X(SE_M_MEGA_KICK, "W025") \
	X(SE_M_MEGA_KICK2, "W025B") \
	X(SE_M_CRABHAMMER, "W152") \
	X(SE_M_JUMP_KICK, "W026") \
	X(SE_M_FLAME_WHEEL, "W172") \
	X(SE_M_FLAME_WHEEL2, "W172B") \
	X(SE_M_FLAMETHROWER, "W053") \
	X(SE_M_FIRE_PUNCH, "W007") \
	X(SE_M_TOXIC, "W092") \
	X(SE_M_SACRED_FIRE, "W221") \
	X(SE_M_SACRED_FIRE2, "W221B") \
	X(SE_M_EMBER, "W052") \
	X(SE_M_TAKE_DOWN, "W036") \
	X(SE_M_BLIZZARD, "W059") \
	X(SE_M_BLIZZARD2, "W059B") \
	X(SE_M_SCRATCH, "W010") \
	X(SE_M_VICEGRIP, "W011") \
	X(SE_M_WING_ATTACK, "W017") \
	X(SE_M_FLY, "W019") \
	X(SE_M_SAND_ATTACK, "W028") \
	X(SE_M_RAZOR_WIND2, "W013B") \
	X(SE_M_BITE, "W044") \
	X(SE_M_HEADBUTT, "W029") \
	X(SE_M_SURF, "W057") \
	X(SE_M_HYDRO_PUMP, "W056") \
	X(SE_M_WHIRLPOOL, "W250") \
	X(SE_M_HORN_ATTACK, "W030") \
	X(SE_M_TAIL_WHIP, "W039") \
	X(SE_M_MIST, "W054") \
	X(SE_M_POISON_POWDER, "W077") \
	X(SE_M_BIND, "W020") \
	X(SE_M_DRAGON_RAGE, "W082") \
	X(SE_M_SING, "W047") \
	X(SE_M_PERISH_SONG, "W195") \
	X(SE_M_PAY_DAY, "W006") \
	X(SE_M_DIG, "W091") \
	X(SE_M_DIZZY_PUNCH, "W146") \
	X(SE_M_SELF_DESTRUCT, "W120") \
	X(SE_M_EXPLOSION, "W153") \
	X(SE_M_ABSORB_2, "W071B") \
	X(SE_M_ABSORB, "W071") \
	X(SE_M_SCREECH, "W103") \
	X(SE_M_BUBBLE_BEAM, "W062") \
	X(SE_M_BUBBLE_BEAM2, "W062B") \
	X(SE_M_SUPERSONIC, "W048") \
	X(SE_M_BELLY_DRUM, "W187") \
	X(SE_M_METRONOME, "W118") \
	X(SE_M_BONEMERANG, "W155") \
	X(SE_M_LICK, "W122") \
	X(SE_M_PSYBEAM, "W060") \
	X(SE_M_FAINT_ATTACK, "W185") \
	X(SE_M_SWORDS_DANCE, "W014") \
	X(SE_M_LEER, "W043") \
	X(SE_M_SWAGGER, "W207") \
	X(SE_M_SWAGGER2, "W207B") \
	X(SE_M_HEAL_BELL, "W215") \
	X(SE_M_CONFUSE_RAY, "W109") \
	X(SE_M_SNORE, "W173") \
	X(SE_M_BRICK_BREAK, "W280") \
	X(SE_M_GIGA_DRAIN, "W202") \
	X(SE_M_PSYBEAM2, "W060B") \
	X(SE_M_SOLAR_BEAM, "W076") \
	X(SE_M_PETAL_DANCE, "W080") \
	X(SE_M_TELEPORT, "W100") \
	X(SE_M_MINIMIZE, "W107") \
	X(SE_M_SKETCH, "W166") \
	X(SE_M_SWIFT, "W129") \
	X(SE_M_REFLECT, "W115") \
	X(SE_M_BARRIER, "W112") \
	X(SE_M_DETECT, "W197") \
	X(SE_M_LOCK_ON, "W199") \
	X(SE_M_MOONLIGHT, "W236") \
	X(SE_M_CHARM, "W204") \
	X(SE_M_CHARGE, "W268") \
	X(SE_M_STRENGTH, "W070") \
	X(SE_M_HYPER_BEAM, "W063") \
	X(SE_M_WATERFALL, "W127") \
	X(SE_M_REVERSAL, "W179") \
	X(SE_M_ACID_ARMOR, "W151") \
	X(SE_M_SANDSTORM, "W201") \
	X(SE_M_TRI_ATTACK, "W161") \
	X(SE_M_TRI_ATTACK2, "W161B") \
	X(SE_M_ENCORE, "W227") \
	X(SE_M_ENCORE2, "W227B") \
	X(SE_M_BATON_PASS, "W226") \
	X(SE_M_MILK_DRINK, "W208") \
	X(SE_M_ATTRACT, "W213") \
	X(SE_M_ATTRACT2, "W213B") \
	X(SE_M_MORNING_SUN, "W234") \
	X(SE_M_FLATTER, "W260") \
	X(SE_M_SAND_TOMB, "W328") \
	X(SE_M_GRASSWHISTLE, "W320") \
	X(SE_M_SPIT_UP, "W255") \
	X(SE_M_DIVE, "W291") \
	X(SE_M_EARTHQUAKE, "W089") \
	X(SE_M_TWISTER, "W239") \
	X(SE_M_SWEET_SCENT, "W230") \
	X(SE_M_YAWN, "W281") \
	X(SE_M_SKY_UPPERCUT, "W327") \
	X(SE_M_STAT_INCREASE, "W287") \
	X(SE_M_HEAT_WAVE, "W257") \
	X(SE_M_UPROAR, "W253") \
	X(SE_M_HAIL, "W258") \
	X(SE_M_COSMIC_POWER, "W322") \
	X(SE_M_TEETER_DANCE, "W298") \
	X(SE_M_STAT_DECREASE, "W287B") \
	X(SE_M_HAZE, "M-HAZE") \
	X(SE_M_HYPER_BEAM2, "M-HYPER-BEAM2")

// Create BGM list
#define X(songId, name) static const u8 sBGMName_##songId[] = _(name);
SOUND_LIST_BGM
#undef X

#define X(songId, name) sBGMName_##songId,
static const u8 *const gBGMNames[] =
{
SOUND_LIST_BGM
};
#undef X

// Create SE list
#define X(songId, name) static const u8 sSEName_##songId[] = _(name);
SOUND_LIST_SE
#undef X

#define X(songId, name) sSEName_##songId,
static const u8 *const gSENames[] =
{
SOUND_LIST_SE
};
#undef X

void Task_InitCryTest(u8 taskId)
{
    struct CryScreenWindow cryWindow1, cryWindow2;
    u8 zero;

    //Text_LoadWindowTemplate(&gWindowTemplate_81E6C3C);
    //InitMenuWindow(&gMenuTextWindowTemplate);
    InitSoundCheckScreenWindows();
    gSoundTestCryNum = 1;
    ResetSpriteData();
    FreeAllSpritePalettes();

    cryWindow1.unk0 = 0x2000;
    cryWindow1.unk2 = 29;
    cryWindow1.paletteNo = 12;
    cryWindow1.yPos = 30;
    cryWindow1.xPos = 4;

    zero = 0; // wtf?
    //gUnknown_03005E98 = 0;
    gDexCryScreenState = 0;

    //while (sub_8119E3C(&cryWindow1, 3) == FALSE)
    //    ;
    //while (sub_8145354(&cryWindow1, 3) == FALSE)
    //    ;
    while (LoadCryMeter(&cryWindow1, gSoundCheckWindowIds[CRY_A]) == FALSE)
        ;

    cryWindow2.unk0 = 0;
    cryWindow2.unk2 = 15;
    cryWindow2.paletteNo = 13;
    cryWindow2.xPos = 12;
    cryWindow2.yPos = 12;

    zero = 0; // wtf?
    //gUnknown_03005E98 = 0;
    gDexCryScreenState = 0;

    //while (ShowPokedexCryScreen(&cryWindow2, 2) == FALSE)
    //    ;
    //while (sub_8145850(&cryWindow2, 2) == FALSE)
    //    ;
    while (LoadCryWaveformWindow(&cryWindow2, gSoundCheckWindowIds[CRY_B]) == FALSE)
        ;

    //Menu_DrawStdWindowFrame(0, 16, 5, 19);
    //ClearStdWindowAndFrameToTransparent(gSoundCheckWindowIds[WIN_A], TRUE);
    //ClearStdWindowAndFrameToTransparent(gSoundCheckWindowIds[WIN_B], TRUE);
    //ClearStdWindowAndFrameToTransparent(gSoundCheckWindowIds[WIN_C], TRUE);
    HideBg(0);
    ShowBg(1);
    //gSoundCheckWindowIds[CRY_A] = AddWindow(&sSoundCheckSEFrame[1]);
    //DrawStdFrameWithCustomTileAndPalette(gSoundCheckWindowIds[CRY_A], FALSE, 1, 1);
    PutWindowTilemap(gSoundCheckWindowIds[CRY_A]);
    PrintCryNumber();
    CopyWindowToVram(gSoundCheckWindowIds[CRY_A], 2);
    DrawMainMenuWindowBorder(&sSoundCheckSEFrame[1], FRAME_TILE_OFFSET);
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB(0, 0, 0));
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0xF01);
    SetGpuReg(REG_OFFSET_BG3CNT, 0x1D03);
    SetGpuReg(REG_OFFSET_DISPCNT, 0x1d40);
    m4aMPlayFadeOutTemporarily(&gMPlayInfo_BGM, 2);
    gTasks[taskId].func = Task_ProcessCryTestInput;
}

void Task_ProcessCryTestInput(u8 taskId)
{
    //sub_8119F88(3);
    //sub_814545C(3);
    UpdateCryWaveformWindow(3);

    if (gMain.newKeys & A_BUTTON)
    {
        //sub_811A050(gSoundTestCryNum);
        //sub_8145534(gSoundTestCryNum);
        CryScreenPlayButton(gSoundTestCryNum);
    }
    if (gMain.newKeys & R_BUTTON)
    {
        StopCryAndClearCrySongs();
    }
    if (gMain.newAndRepeatedKeys & DPAD_UP)
    {
        if(--gSoundTestCryNum == 0)
            gSoundTestCryNum = 384; // total species
        PrintCryNumber();
    }
    if (gMain.newAndRepeatedKeys & DPAD_DOWN)
    {
        if(++gSoundTestCryNum > 384)
            gSoundTestCryNum = 1;
        PrintCryNumber();
    }
    if (gMain.newKeys & B_BUTTON)
    {
        SetGpuReg(REG_OFFSET_DISPCNT, 0x7140);
        SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(17, 223));
        SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(1, 31));
        ClearStdWindowAndFrame(gSoundCheckWindowIds[CRY_A], TRUE);
        RemoveWindow(gSoundCheckWindowIds[CRY_A]);
        ClearStdWindowAndFrame(gSoundCheckWindowIds[CRY_B], TRUE);
        RemoveWindow(gSoundCheckWindowIds[CRY_B]);
        //Menu_EraseWindowRect(0, 0, 29, 19);
        gTasks[taskId].func = Task_InitSoundCheckMenu;
        //DestroyCryMeterNeedleSprite();
        //sub_8145914();
        FreeCryScreen();
    }
}

void PrintCryNumber(void)
{
    PrintSignedNumber(gSoundTestCryNum, 1, 17, 3);
}

static void InitSoundCheckScreenWindows(void)
{
    // InitWindows(sSoundCheckMUSFrame);
    // gSoundCheckWindowIds[MUS_WINDOW] = AddWindow(&sSoundCheckMUSFrame[0]);
    // FillWindowPixelBuffer(gSoundCheckWindowIds[MUS_WINDOW], PIXEL_FILL(0xF));
    // LoadWindowGfx(gSoundCheckWindowIds[MUS_WINDOW], 0, 1, 224);
    // gSoundCheckWindowIds[WIN_B] = AddWindow(&sSoundCheckMUSFrame[1]);
    // FillWindowPixelBuffer(gSoundCheckWindowIds[WIN_B], PIXEL_FILL(0xF));
    // LoadWindowGfx(gSoundCheckWindowIds[WIN_B], 0, 2, 224);
    // gSoundCheckWindowIds[WIN_C] = AddWindow(&sSoundCheckMUSFrame[2]);
    // LoadWindowGfx(gSoundCheckWindowIds[WIN_C], 0, 2, 224);
    // FillWindowPixelBuffer(gSoundCheckWindowIds[WIN_C], PIXEL_FILL(0xF));
    // LoadPalette(gUnknown_0860F074, 0xF0, 0x20);
    // gSoundCheckWindowIds[MUS_WINDOW] = 0;
    // gSoundCheckWindowIds[WIN_B] = 1;
    // gSoundCheckWindowIds[WIN_C] = 2;
    gSoundCheckWindowIds[CRY_A] = AddWindow(&sSoundCheckSEFrame[0]);
    gSoundCheckWindowIds[CRY_B] = AddWindow(&sSoundCheckSEFrame[1]);
}

static void LoadSoundCheckWindowFrameTiles(u8 bgId, u16 tileOffset)
{
    LoadBgTiles(bgId, GetWindowFrameTilesPal(0)->tiles, 0x120, tileOffset);
    LoadPalette(GetWindowFrameTilesPal(0)->pal, 32, 32);
}

static void DrawMainMenuWindowBorder(const struct WindowTemplate *template, u16 baseTileNum)
{
    u16 r9 = 1 + baseTileNum;
    u16 r10 = 2 + baseTileNum;
    u16 sp18 = 3 + baseTileNum;
    u16 spC = 5 + baseTileNum;
    u16 sp10 = 6 + baseTileNum;
    u16 sp14 = 7 + baseTileNum;
    u16 r6 = 8 + baseTileNum;

    FillBgTilemapBufferRect(template->bg, baseTileNum, template->tilemapLeft - 1, template->tilemapTop - 1, 1, 1, 2);
    FillBgTilemapBufferRect(template->bg, r9, template->tilemapLeft, template->tilemapTop - 1, template->width, 1, 2);
    FillBgTilemapBufferRect(template->bg, r10, template->tilemapLeft + template->width, template->tilemapTop - 1, 1, 1, 2);
    FillBgTilemapBufferRect(template->bg, sp18, template->tilemapLeft - 1, template->tilemapTop, 1, template->height, 2);
    FillBgTilemapBufferRect(template->bg, spC, template->tilemapLeft + template->width, template->tilemapTop, 1, template->height, 2);
    FillBgTilemapBufferRect(template->bg, sp10, template->tilemapLeft - 1, template->tilemapTop + template->height, 1, 1, 2);
    FillBgTilemapBufferRect(template->bg, sp14, template->tilemapLeft, template->tilemapTop + template->height, template->width, 1, 2);
    FillBgTilemapBufferRect(template->bg, r6, template->tilemapLeft + template->width, template->tilemapTop + template->height, 1, 1, 2);
    CopyBgTilemapBufferToVram(template->bg);
}

#endif