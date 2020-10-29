TestMap2_MapScripts::
	.byte 0


TestMap2_EventScript_ManTrainer::
	trainerbattle_single TRAINER_TYRON, TestMap2_EventScript_ManTrainer_Text_0, TestMap2_EventScript_ManTrainer_Text_1
	msgbox TestMap2_EventScript_ManTrainer_Text_2, MSGBOX_AUTOCLOSE
	end


TestMap2_EventScript_ManTrainer_Text_0:
	.string "I will not lose.$"

TestMap2_EventScript_ManTrainer_Text_1:
	.string "I lost!$"

TestMap2_EventScript_ManTrainer_Text_2:
	.string "I hate you.$"
