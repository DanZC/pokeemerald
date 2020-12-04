TestGym_MapScripts::
	.byte 0


TestGym_EventScript_GymGuy::
	lock
	faceplayer
	msgbox TestGym_EventScript_GymGuy_Text_0
	release
	end


TestGym_EventScript_ManTrainer::
	trainerbattle_single TRAINER_TYRON, TestGym_EventScript_ManTrainer_Text_0, TestGym_EventScript_ManTrainer_Text_1
	msgbox TestGym_EventScript_ManTrainer_Text_2, MSGBOX_AUTOCLOSE
	end


TestGym_EventScript_GymGuy_Text_0:
	.string "Yo. What's goin' on? Step on the colored\n"
	.string "panels to change the weather.$"

TestGym_EventScript_ManTrainer_Text_0:
	.string "I will not lose.$"

TestGym_EventScript_ManTrainer_Text_1:
	.string "I lost!$"

TestGym_EventScript_ManTrainer_Text_2:
	.string "I hate you.$"
