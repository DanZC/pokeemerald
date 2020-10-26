EventScript_DebugWarp::
    lockall
	message Text_WhichMapToWarp
	waitmessage
    multichoice 16, 0, MULTI_DEBUG_WARP, 0
	copyvar VAR_TEMP_1, VAR_RESULT
	switch VAR_TEMP_1
    case 0, EventScript_DebugWarp_TestMap
    @ case 1, EventScript_DebugWarp_PetalburgShrine
    @ case 2, EventScript_DebugWarp_NewMauville
    @ case 3, EventScript_DebugWarp_MoltresChamber
    @ case 4, EventScript_DebugWarp_ArticunoChamber
    closemessage
EventScript_DebugWarpEnd::
    releaseall
    end

EventScript_DebugWarp_TestMap::
    closemessage
    warpteleport2 MAP_TEST_MAP, 255, 7, 7
    waitstate
    goto EventScript_DebugWarpEnd

EventScript_DebugTestTrainer::
    lockall
	message Text_WhichTrainerToFight
	waitmessage
EventScript_DebugTestTrainerEnd::
    releaseall
    end

EventScript_DebugTestTrainer_Roxanne::
    trainerbattle_no_intro TRAINER_ROXANNE_1, Text_AfterFight
    goto EventScript_DebugTestTrainerEnd
EventScript_DebugTestTrainer_Brawly::
    trainerbattle_no_intro TRAINER_BRAWLY_1, Text_AfterFight
    goto EventScript_DebugTestTrainerEnd
EventScript_DebugTestTrainer_Wattson::
    trainerbattle_no_intro TRAINER_WATTSON_1, Text_AfterFight
    goto EventScript_DebugTestTrainerEnd
EventScript_DebugTestTrainer_Flannery::
    trainerbattle_no_intro TRAINER_FLANNERY_1, Text_AfterFight
    goto EventScript_DebugTestTrainerEnd
EventScript_DebugTestTrainer_Norman::
    trainerbattle_no_intro TRAINER_NORMAN_1, Text_AfterFight
    goto EventScript_DebugTestTrainerEnd
EventScript_DebugTestTrainer_Winona::
    trainerbattle_no_intro TRAINER_WINONA_1, Text_AfterFight
    goto EventScript_DebugTestTrainerEnd
EventScript_DebugTestTrainer_TateLiza::
    @ trainerbattle TRAINER_BATTLE_DOUBLE_NO_INTRO_TEXT, TRAINER_TATE_AND_LIZA_1, 0, Text_AfterFight
    goto EventScript_DebugTestTrainerEnd
EventScript_DebugTestTrainer_Juan::
    trainerbattle_no_intro TRAINER_JUAN_1, Text_AfterFight
    goto EventScript_DebugTestTrainerEnd
EventScript_DebugTestTrainer_Anabel::
    trainerbattle_no_intro TRAINER_ANABEL, Text_AfterFight
    goto EventScript_DebugTestTrainerEnd

EventScript_DebugGiveNatDex::
    lock
    special EnableNationalPokedex
    msgbox Text_GaveNatDex, MSGBOX_DEFAULT
    release
    end  

EventScript_DebugGiveSwampert::
    lock
    setvar VAR_TEMP_1, SPECIES_SWAMPERT
    givemon SPECIES_SWAMPERT, 100, ITEM_NONE
    compare VAR_RESULT, 0
	goto_if_eq EventScript_DebugSendSwampertParty
	compare VAR_RESULT, 1
	goto_if_eq EventScript_DebugSendSwampertPC
    release
	goto Common_EventScript_NoMoreRoomForPokemon
    end

EventScript_DebugSendSwampertParty::
    call EventScript_DebugReceivedSwampertFanfare
    msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq EventScript_DebugReceivedSwampert
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto EventScript_DebugReceivedSwampert
	end

EventScript_DebugSendSwampertPC::
    call EventScript_DebugReceivedSwampertFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq EventScript_DebugSwampertTransferredToPC
	call Common_EventScript_NameReceivedBoxMon
	goto EventScript_DebugSwampertTransferredToPC
	end

EventScript_DebugSwampertTransferredToPC::
    call Common_EventScript_TransferredToPC
	goto EventScript_DebugReceivedSwampert
	end

EventScript_DebugReceivedSwampertFanfare::
    bufferspeciesname 1, SPECIES_SWAMPERT
	playfanfare MUS_OBTAIN_ITEM
	message Text_ObtainedSwampert
	waitmessage
	waitfanfare
	bufferspeciesname 0, SPECIES_SWAMPERT
	return

EventScript_DebugReceivedSwampert::
    release
    end

EventScript_DebugGiveBattlePoints::
    msgbox Text_GaveBP, MSGBOX_DEFAULT
    end

Text_HardModeEnabled:
    .string "HARD MODE enabled.$"

Text_HardModeDisabled:
    .string "HARD MODE disabled.$"

Text_WhichMapToWarp:
    .string "Select a destination.$"

Text_WhichTrainerToFight:
    .string "Select a trainer to battle.$"

Text_BeforeFight:
    .string "Commencing battle...$"

Text_AfterFight:
    .string "I lost!$"

Text_ObtainedSwampert: @ 8222BED
	.string "{PLAYER} obtained a Swampert.$"

Text_GaveNatDex:
    .string "Gave the NATIONAL DEX.$"

Text_GaveBP:
    .string "Gave 300 BP.$"
