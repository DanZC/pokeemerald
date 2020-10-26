TestMap_MapScripts::
	.byte 0


TestMap_EventScript_WelcomeBoy::
	lock
	faceplayer
	msgbox TestMap_EventScript_WelcomeBoy_Text_0
	release
	end


TestMap_EventScript_Sign::
	msgbox TestMap_EventScript_Sign_Text_0, MSGBOX_SIGN
	end


TestMap_EventScript_Sign2::
	msgbox TestMap_EventScript_Sign2_Text_0, MSGBOX_SIGN
	end


TestMap_EventScript_GymBlocker::
	lock
	faceplayer
	msgbox TestMap_EventScript_GymBlocker_Text_0
	release
	end


TestMap_EventScript_WelcomeBoy_Text_0:
	.string "Welcome to the TEST MAP.\pHow do you like the graphics?$"

TestMap_EventScript_Sign_Text_0:
	.string "This is a sign.$"

TestMap_EventScript_Sign2_Text_0:
	.string "This is a gym sign.$"

TestMap_EventScript_GymBlocker_Text_0:
	.string "This gym is not ready yet.\nCome back later.$"
