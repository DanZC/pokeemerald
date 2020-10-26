#include "global.h"
#include "fieldmap.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "field_weather.h"
#include "field_screen_effect.h"
#include "international_string_util.h"
#include "list_menu.h"
#include "main.h"
#include "menu.h"
#include "pokedex.h"
#include "script.h"
#include "strings.h"
#include "string_util.h"
#include "sound.h"
#include "strings.h"
#include "task.h"
#include "constants/battle_frontier.h"
#include "constants/songs.h"
#include "constants/flags.h"
//#include "constants/game_modes.h"
#include "constants/species.h"
#include "region_map.h"
#include "event_data.h"
#include "debug.h"

//Palette fix
#include "map_name_popup.h"
//void HideMapNamePopUpWindow(void);
//void sub_81973A4(void);

#if 0 == 0 //DEBUG

//extern const u8 EventScript_DebugToggleMode[];
extern const u8 EventScript_DebugWarp[];
extern const u8 EventScript_DebugTestTrainer[];
extern const u8 EventScript_DebugGiveSwampert[];
extern const u8 EventScript_DebugGiveNatDex[];
extern const u8 EventScript_DebugGiveBattlePoints[];

#define DEBUG_MAIN_MENU_HEIGHT 7
#define DEBUG_MAIN_MENU_WIDTH 14

#define DEBUG_FLAG_EDIT_MENU_HEIGHT 4
#define DEBUG_FLAG_EDIT_MENU_WIDTH 7

EWRAM_DATA void (*gDebug_exitCallback)(void) = NULL;
EWRAM_DATA s16 sCurrentFlag = TEMP_FLAGS_START + 0x1;

void Debug_ShowMainMenu(void);
void Debug_ShowFlagEditMenu(void);

static void Debug_DrawFlagEditMenu(u8);

static void Debug_DestroyMainMenu(u8);
static void Debug_DestroyMainMenuThenRunScript(u8,const u8*);
static void Debug_DestroyFlagEditMenu(u8);

//Menu Actions
static void DebugAction_Flags(u8);
static void DebugAction_Fly(u8);
static void DebugAction_EditFlags(u8);
static void DebugAction_HardMode(u8);
static void DebugAction_SeeEveryMon(u8);
static void DebugAction_CatchEveryMon(u8);
static void DebugAction_Warp(u8);
static void DebugAction_TestTrainer(u8);
static void DebugAction_GiveNatDex(u8);
static void DebugAction_GiveSwampert(u8);
static void DebugAction_GiveBattlePoints(u8);
static void DebugAction_Cancel(u8);

//Tasks
static void DebugTask_HandleMainMenuInput(u8);
static void DebugTask_HandleFlagEditInput(u8);

//Text
static const u8 gDebugText_Flags[] = _("FLAGS");
static const u8 gDebugText_Fly[] = _("FLY");
static const u8 gDebugText_EditFlags[] = _("FLY FLAGS");
static const u8 gDebugText_HardMode[] = _("TOGGLE HARD MODE");
static const u8 gDebugText_SeeEveryHMon[] = _("SEE EVERY MON");
static const u8 gDebugText_CatchEveryMon[] = _("CATCH EVERY MON");
static const u8 gDebugText_Warp[] = _("WARP");
static const u8 gDebugText_TestTrainer[] = _("TEST TRAINER");
static const u8 gDebugText_GiveSwampert[] = _("GIVE SWAMPERT");
static const u8 gDebugText_GiveNatDex[] = _("GIVE NATIONAL DEX");
static const u8 gDebugText_GiveBP[] = _("GIVE BP");
static const u8 gDebugText_Cancel[] = _("CANCEL");

static const u8 gDebugText_FlagHex[] = _("FLAG 0X");

static bool8 FieldCB_ReturnToFieldDebugMenu(void);

enum {
    DEBUG_MENU_ITEM_FLAGS,
    DEBUG_MENU_ITEM_FLY,
    DEBUG_MENU_ITEM_WARP,
    DEBUG_MENU_ITEM_TESTTRAINER,
    DEBUG_MENU_ITEM_EDITFLAGS,
    DEBUG_MENU_ITEM_HARDMODE,
    DEBUG_MENU_ITEM_SEEEVERYMON,
    DEBUG_MENU_ITEM_CATCHEVERYMON,
    DEBUG_MENU_ITEM_GIVENATDEX,
    DEBUG_MENU_ITEM_GIVESWAMPERT,
    DEBUG_MENU_ITEM_GIVEBP,
    DEBUG_MENU_ITEM_CANCEL,
};

static const struct ListMenuItem sDebugMenuItems[] =
{
    [DEBUG_MENU_ITEM_FLAGS] = {gDebugText_Flags, DEBUG_MENU_ITEM_FLAGS},
    [DEBUG_MENU_ITEM_FLY] = {gDebugText_Fly, DEBUG_MENU_ITEM_FLY},
    [DEBUG_MENU_ITEM_EDITFLAGS] = {gDebugText_EditFlags, DEBUG_MENU_ITEM_EDITFLAGS},
    [DEBUG_MENU_ITEM_HARDMODE] = {gDebugText_HardMode, DEBUG_MENU_ITEM_HARDMODE},
    [DEBUG_MENU_ITEM_SEEEVERYMON] = {gDebugText_SeeEveryHMon, DEBUG_MENU_ITEM_SEEEVERYMON},
    [DEBUG_MENU_ITEM_CATCHEVERYMON] = {gDebugText_CatchEveryMon, DEBUG_MENU_ITEM_CATCHEVERYMON},
    [DEBUG_MENU_ITEM_WARP] = {gDebugText_Warp, DEBUG_MENU_ITEM_WARP},
    [DEBUG_MENU_ITEM_TESTTRAINER] = {gDebugText_TestTrainer, DEBUG_MENU_ITEM_TESTTRAINER},
    [DEBUG_MENU_ITEM_GIVENATDEX] = {gDebugText_GiveNatDex, DEBUG_MENU_ITEM_GIVENATDEX},
    [DEBUG_MENU_ITEM_GIVESWAMPERT] = {gDebugText_GiveSwampert, DEBUG_MENU_ITEM_GIVESWAMPERT},
    [DEBUG_MENU_ITEM_GIVEBP] = {gDebugText_GiveBP, DEBUG_MENU_ITEM_GIVEBP},
    [DEBUG_MENU_ITEM_CANCEL] = {gDebugText_Cancel, DEBUG_MENU_ITEM_CANCEL},
};

static const struct ListMenuItem sFlagItems[] =
{
    {gText_EmptyString2, 0}
};

static void (*const sDebugMenuActions[])(u8) = 
{
    [DEBUG_MENU_ITEM_FLAGS] = DebugAction_Flags,
    [DEBUG_MENU_ITEM_FLY] = DebugAction_Fly,
    [DEBUG_MENU_ITEM_EDITFLAGS] = DebugAction_EditFlags,
    [DEBUG_MENU_ITEM_HARDMODE] = DebugAction_HardMode,
    [DEBUG_MENU_ITEM_SEEEVERYMON] = DebugAction_SeeEveryMon,
    [DEBUG_MENU_ITEM_CATCHEVERYMON] = DebugAction_CatchEveryMon,
    [DEBUG_MENU_ITEM_WARP] = DebugAction_Warp,
    [DEBUG_MENU_ITEM_TESTTRAINER] = DebugAction_TestTrainer,
    [DEBUG_MENU_ITEM_GIVENATDEX] = DebugAction_GiveNatDex,
    [DEBUG_MENU_ITEM_GIVESWAMPERT] = DebugAction_GiveSwampert,
    [DEBUG_MENU_ITEM_GIVEBP] = DebugAction_GiveBattlePoints,
    [DEBUG_MENU_ITEM_CANCEL] = DebugAction_Cancel,
};

static const struct WindowTemplate sDebugMenuWindowTemplate = 
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = DEBUG_MAIN_MENU_WIDTH,
    .height = 2 * DEBUG_MAIN_MENU_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 1,
};

static const struct WindowTemplate sDebugFlagEditWindowTemplate = 
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = DEBUG_FLAG_EDIT_MENU_WIDTH,
    .height = DEBUG_FLAG_EDIT_MENU_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 1,
};

static const struct ListMenuTemplate sDebugMenuListTemplate =
{
    .items = sDebugMenuItems,
    .moveCursorFunc = ListMenuDefaultCursorMoveFunc,
    .totalItems = ARRAY_COUNT(sDebugMenuItems),
    .maxShowed = DEBUG_MAIN_MENU_HEIGHT,
    .windowId = 0,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = 1,
    .cursorKind = 0
};

static const struct ListMenuTemplate sFlagListTemplate =
{
    .items = sFlagItems,
    .moveCursorFunc = NULL,
    .itemPrintFunc = NULL,
    .totalItems = ARRAY_COUNT(sFlagItems),
    .maxShowed = 1,
    .windowId = 0,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = 1,
    .cursorKind = 0
};

/**
 * Main Menu
 * */

void Debug_ShowMainMenu(void)
{
    struct ListMenuTemplate menuTemplate;
    u8 windowId;
    u8 menuTaskId;
    u8 inputTaskId;

    //create window
    windowId = AddWindow(&sDebugMenuWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);

    //create list menu
    menuTemplate = sDebugMenuListTemplate;
    menuTemplate.windowId = windowId;
    menuTaskId = ListMenuInit(&menuTemplate, 0, 0);

    //draw
    CopyWindowToVram(windowId, 3);

    //create input handler task
    inputTaskId = CreateTask(DebugTask_HandleMainMenuInput, 3);
    gTasks[inputTaskId].data[0] = menuTaskId;
    gTasks[inputTaskId].data[1] = windowId;
}

void DebugTask_HandleMainMenuInput(u8 taskId)
{
    void (*func)(u8);
    u32 input = ListMenu_ProcessInput(gTasks[taskId].data[0]);

    if(gMain.newKeys & A_BUTTON)
    {
        PlaySE(SE_SELECT);
        if((func = sDebugMenuActions[input]) != NULL)
            func(taskId);
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        PlaySE(SE_SELECT);
        Debug_DestroyMainMenu(taskId);
    }
}

static void Debug_DestroyMainMenu(u8 taskId)
{
    if(gDebug_exitCallback != NULL) 
    {
        SetMainCallback2(gDebug_exitCallback);
        gDebug_exitCallback = NULL;
        DestroyListMenuTask(gTasks[taskId].data[0], NULL, NULL);
        ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
        RemoveWindow(gTasks[taskId].data[1]);
        DestroyTask(taskId);
        ScriptContext2_Enable();
    } 
    else 
    {
        DestroyListMenuTask(gTasks[taskId].data[0], NULL, NULL);
        ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
        RemoveWindow(gTasks[taskId].data[1]);
        DestroyTask(taskId);
        EnableBothScriptContexts();
    }
}

static void Debug_DestroyMainMenuThenRunScript(u8 taskId, const u8 *script) 
{
    DestroyListMenuTask(gTasks[taskId].data[0], NULL, NULL);
    ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);
    DestroyTask(taskId);
    ScriptContext1_SetupScript(script);
    EnableBothScriptContexts();
}

static void DebugAction_Flags(u8 taskId)
{
    Debug_DestroyMainMenu(taskId);
    sCurrentFlag = TEMP_FLAGS_START + 0x1;
    Debug_ShowFlagEditMenu();
}

static void DebugAction_Fly(u8 taskId)
{
    gDebug_exitCallback = CB2_OpenFlyMap;
    Debug_DestroyMainMenu(taskId);
}

static void DebugAction_HardMode(u8 taskId)
{
    Debug_DestroyMainMenu(taskId);
    //Debug_DestroyMainMenuThenRunScript(taskId, EventScript_DebugToggleMode);
}

static void DebugAction_Warp(u8 taskId)
{   
    Debug_DestroyMainMenuThenRunScript(taskId, EventScript_DebugWarp);
}

static void DebugAction_TestTrainer(u8 taskId)
{   
    Debug_DestroyMainMenuThenRunScript(taskId, EventScript_DebugTestTrainer);
}

static void DebugAction_GiveNatDex(u8 taskId)
{
    FlagSet(FLAG_SYS_NATIONAL_DEX);
    Debug_DestroyMainMenuThenRunScript(taskId, EventScript_DebugGiveNatDex);
}

static void DebugAction_GiveSwampert(u8 taskId)
{
    Debug_DestroyMainMenuThenRunScript(taskId, EventScript_DebugGiveSwampert);
}

static void DebugAction_EditFlags(u8 taskId)
{
    FlagSet(FLAG_VISITED_LITTLEROOT_TOWN);
    FlagSet(FLAG_VISITED_OLDALE_TOWN);
    FlagSet(FLAG_VISITED_PETALBURG_CITY);
    FlagSet(FLAG_VISITED_RUSTBORO_CITY);
    FlagSet(FLAG_VISITED_DEWFORD_TOWN);
    FlagSet(FLAG_VISITED_SLATEPORT_CITY);
    FlagSet(FLAG_VISITED_MAUVILLE_CITY);
    FlagSet(FLAG_VISITED_VERDANTURF_TOWN);
    FlagSet(FLAG_VISITED_FALLARBOR_TOWN);
    FlagSet(FLAG_VISITED_LAVARIDGE_TOWN);
    FlagSet(FLAG_VISITED_FORTREE_CITY);
    FlagSet(FLAG_VISITED_LILYCOVE_CITY);
    FlagSet(FLAG_VISITED_PACIFIDLOG_TOWN);
    FlagSet(FLAG_VISITED_MOSSDEEP_CITY);
    FlagSet(FLAG_VISITED_SOOTOPOLIS_CITY);
    FlagSet(FLAG_VISITED_EVER_GRANDE_CITY);
    FlagSet(FLAG_LANDMARK_POKEMON_LEAGUE);
    FlagSet(FLAG_LANDMARK_BATTLE_FRONTIER);
    Debug_DestroyMainMenu(taskId);
}

static void DebugAction_SeeEveryMon(u8 taskId)
{
    int i;
    for(i = 1; i < NATIONAL_DEX_COUNT; i++)
        GetSetPokedexFlag(i, FLAG_SET_SEEN);
    Debug_DestroyMainMenu(taskId);
}

static void DebugAction_CatchEveryMon(u8 taskId)
{
    int i;
    for(i = 1; i < NATIONAL_DEX_COUNT; i++)
        GetSetPokedexFlag(i, FLAG_SET_CAUGHT);
    Debug_DestroyMainMenu(taskId);
}

void DebugAction_GiveBattlePoints(u8 taskId)
{
    VarSet(VAR_0x8004, 300);
    if (gSaveBlock2Ptr->frontier.battlePoints + gSpecialVar_0x8004 > MAX_BATTLE_FRONTIER_POINTS)
    {
        gSaveBlock2Ptr->frontier.battlePoints = MAX_BATTLE_FRONTIER_POINTS;
    }
    else
    {
        gSaveBlock2Ptr->frontier.battlePoints = gSaveBlock2Ptr->frontier.battlePoints + gSpecialVar_0x8004;
    }
    Debug_DestroyMainMenuThenRunScript(taskId, EventScript_DebugGiveBattlePoints);
}

static void DebugAction_Cancel(u8 taskId)
{
    Debug_DestroyMainMenu(taskId);
}

/**
 * Flag Edit Menu
 * */

void Debug_ShowFlagEditMenu(void)
{
    struct ListMenuTemplate menuTemplate;
    u8 windowId;
    u8 menuTaskId;
    u8 inputTaskId;

    //create window
    windowId = AddWindow(&sDebugFlagEditWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);

    //create list menu
    menuTemplate = sFlagListTemplate;
    menuTemplate.windowId = windowId;
    menuTaskId = ListMenuInit(&menuTemplate, 0, 0);

    //draw
    Debug_DrawFlagEditMenu(windowId);

    //create input handler task
    inputTaskId = CreateTask(DebugTask_HandleMainMenuInput, 3);
    //gTasks[taskId].func = Task_DebugMenuProcessInput;
    gTasks[inputTaskId].data[0] = menuTaskId;
    gTasks[inputTaskId].data[1] = windowId;
}

static const u8 sDefaultTextColors[] = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GREY };
static const u8 sDebugOnColor[] = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_GREEN, TEXT_COLOR_GREEN };
static const u8 sDebugOffColor[] = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_RED, TEXT_COLOR_RED };

static const u8 gDebugText_FlagFmt[] = _("{STR_VAR_1}{STR_VAR_2}");

static void Debug_DrawFlagEditMenu(u8 windowId)
{
    StringCopy(gStringVar1, gDebugText_FlagHex);
    ConvertIntToHexStringN(gStringVar2, (s32)sCurrentFlag, STR_CONV_MODE_LEADING_ZEROS, 3);
    StringExpandPlaceholders(gStringVar3, gDebugText_FlagFmt);

    AddTextPrinterParameterized3(windowId, 0, 2, 2, sDefaultTextColors, 0, gStringVar3);
    if(FlagGet(sCurrentFlag))
    {
        AddTextPrinterParameterized3(windowId, 0, 8, 12, sDebugOnColor, 0, gText_On);
    }
    else
    {
        AddTextPrinterParameterized3(windowId, 0, 8, 12, sDebugOffColor, 0, gText_Off);
    }
    CopyWindowToVram(windowId, 3);
}

static void DebugTask_HandleFlagEditInput(u8 taskId)
{
    //void (*func)(u8);
    u8 windowId = gTasks[taskId].data[1];

    if(gMain.newKeys & R_BUTTON)
    {
        sCurrentFlag++;
        PlaySE(SE_RG_BAG_POCKET);
        Debug_DrawFlagEditMenu(windowId);
    }

    if(gMain.newKeys & L_BUTTON)
    {
        if(sCurrentFlag > TEMP_FLAGS_START + 0x1) 
        {
            sCurrentFlag--;
            PlaySE(SE_RG_BAG_POCKET);
            Debug_DrawFlagEditMenu(windowId);
        }
    }

    if(gMain.newKeys & A_BUTTON)
    {
        if(FlagGet(sCurrentFlag))
        {
            FlagClear(sCurrentFlag);
            PlaySE(SE_PC_OFF);
        } 
        else 
        {
            FlagSet(sCurrentFlag);
            PlaySE(SE_PC_LOGIN);
        }
        Debug_DrawFlagEditMenu(windowId);
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        PlaySE(SE_SELECT);
        Debug_DestroyFlagEditMenu(taskId);
    }
    //u32 input = ListMenu_ProcessInput(gTasks[taskId].data[0]);
}

static void Debug_DestroyFlagEditMenu(u8 taskId)
{
    DestroyListMenuTask(gTasks[taskId].data[0], NULL, NULL);
    ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);
    DestroyTask(taskId);
    EnableBothScriptContexts();
}

#endif
