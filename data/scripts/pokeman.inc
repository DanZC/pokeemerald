Common_EventScript_Pokeman::
	lock
	faceplayer
	message Common_EventScript_Pokeman_Text_0
	goto Common_EventScript_Pokeman_MainLoop
	return


Common_EventScript_Pokeman_MainLoop:
	waitmessage
	multichoice 16, 0, MULTI_POKEMAN, 0
	switch VAR_RESULT
	case 0, Common_EventScript_Pokeman_MainLoop_3
	case 1, Common_EventScript_Pokeman_MainLoop_4
	case 2, Common_EventScript_Pokeman_MainLoop_5
	goto Common_EventScript_Pokeman_End
Common_EventScript_Pokeman_MainLoop_1:
	message Common_EventScript_Pokeman_MainLoop_Text_0
	goto Common_EventScript_Pokeman_MainLoop
	return

Common_EventScript_Pokeman_MainLoop_3:
	call Common_EventScript_Pokeman_NameRater
	goto Common_EventScript_Pokeman_MainLoop_1

Common_EventScript_Pokeman_MainLoop_4:
	call Common_EventScript_Pokeman_MoveDeleter
	goto Common_EventScript_Pokeman_MainLoop_1

Common_EventScript_Pokeman_MainLoop_5:
	call Common_EventScript_Pokeman_MoveRelearner
	goto Common_EventScript_Pokeman_MainLoop_1


Common_EventScript_Pokeman_End:
	msgbox Common_EventScript_Pokeman_End_Text_0
	release
	end


Common_EventScript_Pokeman_NameRater::
	msgbox NameRater_Text_CritiqueWhichMonNickname, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	compare VAR_0x8004, 255
	goto_if_ne Common_EventScript_Pokeman_NameRater_2
	return

	return

Common_EventScript_Pokeman_NameRater_2:
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	compare VAR_RESULT, SPECIES_EGG
	goto_if_eq Common_EventScript_Pokeman_NameRater_6
	special BufferMonNickname
	msgbox NameRater_Text_FineNameSuggestBetterOne, MSGBOX_YESNO
	compare VAR_RESULT, YES
	goto_if_eq Common_EventScript_Pokeman_NameRater_8
	return

Common_EventScript_Pokeman_NameRater_6:
	msgbox NameRater_Text_ThatIsMerelyAnEgg, MSGBOX_DEFAULT
	return

Common_EventScript_Pokeman_NameRater_8:
	msgbox NameRater_Text_WhatShallNewNameBe, MSGBOX_DEFAULT
	call Common_EventScript_NameReceivedPartyMon
	specialvar VAR_RESULT, TryPutNameRaterShowOnTheAir
	special BufferMonNickname
	compare VAR_RESULT, TRUE
	goto_if_eq Common_EventScript_Pokeman_NameRater_12
	msgbox NameRater_Text_NameNoDifferentYetSuperior, MSGBOX_DEFAULT
Common_EventScript_Pokeman_NameRater_11:
	return

Common_EventScript_Pokeman_NameRater_12:
	msgbox NameRater_Text_MonShallBeKnownAsName, MSGBOX_DEFAULT
	goto Common_EventScript_Pokeman_NameRater_11


Common_EventScript_Pokeman_MoveDeleter::
	msgbox MoveDeleter_Text_WhichMonShouldForget
	special ChoosePartyMon
	waitstate
	compare VAR_0x8004, 255
	goto_if_ne Common_EventScript_Pokeman_MoveDeleter_1
	return

Common_EventScript_Pokeman_MoveDeleter_1:
	special IsSelectedMonEgg
	compare VAR_RESULT, TRUE
	goto_if_eq Common_EventScript_Pokeman_MoveDeleter_5
	special GetNumMovesSelectedMonHas
	compare VAR_RESULT, 1
	goto_if_eq Common_EventScript_Pokeman_MoveDeleter_8
Common_EventScript_Pokeman_MoveDeleter_12:
	msgbox MoveDeleter_Text_WhichMoveShouldBeForgotten, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	special MoveDeleterChooseMoveToForget
	fadescreen FADE_FROM_BLACK
	compare VAR_0x8005, MAX_MON_MOVES
	goto_if_eq Common_EventScript_Pokeman_MoveDeleter_12
	special BufferMoveDeleterNicknameAndMove
	msgbox MoveDeleter_Text_MonsMoveShouldBeForgotten, MSGBOX_YESNO
	switch VAR_RESULT
	case YES, Common_EventScript_Pokeman_MoveDeleter_15
	case NO, Common_EventScript_Pokeman_MoveDeleter_16
	return

Common_EventScript_Pokeman_MoveDeleter_5:
	msgbox MoveDeleter_Text_EggCantForgetMoves, MSGBOX_DEFAULT
	return

Common_EventScript_Pokeman_MoveDeleter_8:
	special BufferMoveDeleterNicknameAndMove
	msgbox MoveDeleter_Text_MonOnlyKnowsOneMove, MSGBOX_DEFAULT
	return

Common_EventScript_Pokeman_MoveDeleter_15:
	special MoveDeleterForgetMove
	playfanfare MUS_MOVE_DELETED
	waitfanfare
	msgbox MoveDeleter_Text_MonHasForgottenMove, MSGBOX_DEFAULT
	return

Common_EventScript_Pokeman_MoveDeleter_16:
	return


Common_EventScript_Pokeman_MoveRelearner::
	msgbox MoveRelearner_Text_TutorWhichMon, MSGBOX_DEFAULT
	special ChooseMonForMoveRelearner
	waitstate
	compare VAR_0x8004, 255
	goto_if_eq Common_EventScript_Pokeman_MoveRelearner_2
	special IsSelectedMonEgg
	compare VAR_RESULT, TRUE
	goto_if_eq Common_EventScript_Pokeman_MoveRelearner_5
Common_EventScript_Pokeman_MoveRelearner_4:
	compare VAR_0x8005, 0
	goto_if_eq Common_EventScript_Pokeman_MoveRelearner_8
Common_EventScript_Pokeman_MoveRelearner_7:
	msgbox MoveRelearner_Text_TeachWhichMove, MSGBOX_DEFAULT
	special TeachMoveRelearnerMove
	waitstate
	compare VAR_0x8004, 0
	goto_if_eq Common_EventScript_Pokeman_MoveRelearner_11
	return

Common_EventScript_Pokeman_MoveRelearner_2:
	return

Common_EventScript_Pokeman_MoveRelearner_5:
	msgbox MoveRelearner_Text_CantTeachEgg, MSGBOX_DEFAULT
	goto Common_EventScript_Pokeman_MoveRelearner
	goto Common_EventScript_Pokeman_MoveRelearner_4

Common_EventScript_Pokeman_MoveRelearner_8:
	msgbox MoveRelearner_Text_DontHaveMoveToTeachPokemon, MSGBOX_DEFAULT
	goto Common_EventScript_Pokeman_MoveRelearner
	goto Common_EventScript_Pokeman_MoveRelearner_7

Common_EventScript_Pokeman_MoveRelearner_11:
	return


Common_EventScript_Pokeman_Text_0:
	.string "I'm the POKeMAN. …Or at least that's\n"
	.string "what people call me.\p"
	.string "I offer a variety of services to\n"
	.string "Trainers such as yourself.\p"
	.string "What can I do for you?$"

Common_EventScript_Pokeman_MainLoop_Text_0:
	.string "Is there anything else I can help you\n"
	.string "with?$"

Common_EventScript_Pokeman_End_Text_0:
	.string "If you need my services in the future,\n"
	.string "I'll be in any POKeMON CENTER.$"

NameRater_Text_CritiqueWhichMonNickname:
	.string "Which POKéMON's nickname should\n"
	.string "I critique?$"

NameRater_Text_ThatIsMerelyAnEgg:
	.string "Now, now.\n"
	.string "That is merely an EGG!$"

NameRater_Text_FineNameSuggestBetterOne:
	.string "Hmmm… {STR_VAR_1}, is it? That is\n"
	.string "quite a fine name you bestowed.\p"
	.string "But! What say you, if I were to\n"
	.string "suggest a slightly better name?$"

NameRater_Text_WhatShallNewNameBe:
	.string "Ah, good. Then, what shall the new\n"
	.string "nickname be?$"

NameRater_Text_MonShallBeKnownAsName:
	.string "Done! From now on, this POKéMON\n"
	.string "shall be known as {STR_VAR_1}!\p"
	.string "It is a better name than before!\n"
	.string "How fortunate for you!$"

NameRater_Text_NameNoDifferentYetSuperior:
	.string "Done! From now on, this POKéMON\n"
	.string "shall be known as {STR_VAR_1}!\p"
	.string "It looks no different from before,\n"
	.string "and yet, this is vastly superior!\p"
	.string "How fortunate for you!$"

MoveDeleter_Text_WhichMonShouldForget:
	.string "Which POKéMON should forget a move?$"

MoveDeleter_Text_EggCantForgetMoves:
	.string "What?\nNo EGG should know any moves.$"

MoveDeleter_Text_MonOnlyKnowsOneMove:
	.string "{STR_VAR_1} knows only one move\nso it can't be forgotten…$"

MoveDeleter_Text_WhichMoveShouldBeForgotten:
	.string "Which move should be forgotten?$"

MoveDeleter_Text_MonsMoveShouldBeForgotten:
	.string "Hm! {STR_VAR_1}'s {STR_VAR_2}?\nThat move should be forgotten?$"

MoveDeleter_Text_MonHasForgottenMove:
	.string "It worked to perfection!\p"
	.string "{STR_VAR_1} has forgotten\n"
	.string "{STR_VAR_2} completely.$"

MoveRelearner_Text_TutorWhichMon:
	.string "Which POKéMON needs tutoring?$"

MoveRelearner_Text_CantTeachEgg:
	.string "Hunh? There isn't a single move that\n"
	.string "I can teach an EGG.$"

MoveRelearner_Text_DontHaveMoveToTeachPokemon:
	.string "Sorry…\p"
	.string "It doesn't appear as if I have any move\n"
	.string "I can teach that POKéMON.$"

MoveRelearner_Text_TeachWhichMove:
	.string "Which move should I teach?$"
