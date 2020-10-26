TestMap_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	map_script MAP_SCRIPT_ON_TRANSITION, TestMap_PokemonCenter_1F_MapScripts_MAP_SCRIPT_ON_TRANSITION
	.byte 0

TestMap_PokemonCenter_1F_MapScripts_MAP_SCRIPT_ON_TRANSITION:
	end


TestMap_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end


TestMap_PokemonCenter_1F_EventScript_Block_2F::
	msgbox TestMap_PokemonCenter_1F_EventScript_Block_2F_Text_0, MSGBOX_NPC
	end


TestMap_PokemonCenter_1F_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart TestMap_PokemonCenter_1F_Pokemart
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end


TestMap_PokemonCenter_1F_EventScript_Clerk2::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart TestMap_PokemonCenter_1F_Pokemart2
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end


	.align 2
TestMap_PokemonCenter_1F_Pokemart:
    .2byte ITEM_POKE_BALL
    .2byte ITEM_GREAT_BALL
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_POTION
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_REVIVE
	.2byte ITEM_SUPER_REPEL
	.2byte ITEM_MAX_REPEL
    .2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_NONE
	release
	end

	.align 2
TestMap_PokemonCenter_1F_Pokemart2:
    .2byte ITEM_X_ATTACK
    .2byte ITEM_X_DEFENSE
	.2byte ITEM_X_SP_ATK
	.2byte ITEM_X_SP_DEF
	.2byte ITEM_X_SPEED
	.2byte ITEM_GUARD_SPEC
	.2byte ITEM_DIRE_HIT
	.2byte ITEM_NONE
	release
	end

TestMap_PokemonCenter_1F_EventScript_Pokeman::
	lock
	faceplayer
	return


TestMap_PokemonCenter_1F_EventScript_Block_2F_Text_0:
	.string "I'm sorry. There's a problem on the"
	.string "\nsecond floor."
	.string "\pYou can't go up now.$"
