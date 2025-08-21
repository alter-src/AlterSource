//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "tier1/convar.h"
#include "Color.h"

#include "cdll_client_int.h"

// i took this from hl2:ce, fuck you
CON_COMMAND( srcfetch, "Displays...something." )
{
	Color cyan(0, 255, 255, 255); // we're going to use that A LOT

	ConColorMsg( cyan, "        @@@@@@@@@@@@@@@@@@@@@@@@@@@@\n" );
	ConColorMsg( cyan, "  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n" );
	ConColorMsg( cyan, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n" );
	ConColorMsg( cyan, "@@@@@@@        @@@@@@@@@@@@@@@@@@@@@@@@\n" );
	ConColorMsg( cyan, " @@                @@@@@@@@@@@@@@@@@@@@\n" );
	ConColorMsg( cyan, "                      @@@@@@@@@@@@@@@@@\n" );
	Msg( "   @@@@@@@@@@@@@" );
	ConColorMsg( cyan, "        @@@@@@@@@@@@@@ \n" );
	Msg( "  @@@@@@@@@@@@@@@@" );
	ConColorMsg( cyan, "       @@@@@@@@@@@@@ \n" );
	Msg( " @@@@@@@   @@@@@@@@" );
	ConColorMsg( cyan, "       @@@@@@@@@@@  \n" );
	Msg( " @@@@@@@     @@@@@@" );
	ConColorMsg( cyan, "        @@@@@@@@@   \n" );
	Msg( " @@@@@@@@@@@@@@@" );
	ConColorMsg( cyan, "           @@@@@@@@    \n" );
	Msg( "  @@@@@@@@@@@@@@@@" );
	ConColorMsg( cyan, "          @@@@@@     \n" );
	Msg( "     @@@@@@@@@@@@@@" );
	ConColorMsg( cyan, "         @@@@@@     \n" );
	Msg( "@@@@@@      @@@@@@@@" );
	ConColorMsg( cyan, "       @@@@@@      \n" );
	Msg( " @@@@@@      @@@@@@" );
	ConColorMsg( cyan, "        @@@@@       \n" );
	Msg( " @@@@@@@@@@@@@@@@@@" );
	ConColorMsg( cyan, "       @@@@@        \n" );
	Msg( "   @@@@@@@@@@@@@@" );
	ConColorMsg( cyan, "        @@@@@@        \n" );
	Msg( "       @@@@@@" );
	ConColorMsg( cyan, "           @@@@@@         \n" );
	ConColorMsg( cyan, "                      @@@@@@           \n" );
	ConColorMsg( cyan, "                     @@@@@@            \n" );
	ConColorMsg( cyan, "                       @@@             \n\n" );

	// and the info
	Msg( "Name: Alter Source\n" );
	Msg( "Source Engine version: %d\n", engine->GetEngineBuildNumber() );
	if ( engine->IsInGame() )
		Msg( "Map: %s\n", engine->GetLevelName() );
}
