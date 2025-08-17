"MainMenu"
{	
	"ResumeGame"
	{
		"text"			"#GameUI_GameMenu_ResumeGame"
		"command"		"cmd gamemenucommand resumegame"
		"priority"		"7"
		"family"		"ingame"
	}
	
	"CreateServer"
	{
		"text"			"#Alter_CreateServer"
		"command"		"cmd gamemenucommand opencreatemultiplayergamedialog"
		"priority"		"6"
		"family"		"all"
	}
	
	"ServerBrowser"
	{
		"text"			"#Alter_ServerBrowser"
		"command"		"cmd gamemenucommand openserverbrowser"
		"priority"		"6"
		"family"		"all"
	}

	"Options"
	{
		"text"			"#GameUI_GameMenu_Options"
		"command"		"cmd gamepadui_openoptionsdialog"
		"priority"		"3"
		"family"		"all"
	}

	"Options"
	{
		"text"			"#Alter_Disconnect"
		"command"		"cmd disconnect"
		"priority"		"3"
		"family"		"ingame"
	}
	
	"Quit"
	{
		"text"			"#GameUI_GameMenu_Quit"
		"command"		"cmd gamepadui_openquitgamedialog"
		"priority"		"1"
		"family"		"all"
	}
}