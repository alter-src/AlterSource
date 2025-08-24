//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "discordmgr.h"
#include "discord_rpc.h"
#include <cstring>

CDiscordRPC g_DiscordRPC;

CDiscordRPC::CDiscordRPC()
{
	m_bInitialized = false;
}

CDiscordRPC::~CDiscordRPC()
{
	Shutdown();
}

void CDiscordRPC::Init( const char* appId )
{
	if ( m_bInitialized )
		return;

	DiscordEventHandlers handlers;
	memset( &handlers, 0, sizeof( handlers ) );

	Discord_Initialize( appId, &handlers, 1, NULL );
	m_bInitialized = true;
}

void CDiscordRPC::Update( const char* state, const char* details, const char* largeImage, const char* smallImage )
{
	if ( !m_bInitialized )
		return;

	DiscordRichPresence presence;
	memset( &presence, 0, sizeof( presence ) );

	presence.state = state;
	presence.details = details;
	presence.largeImageKey = largeImage;
	presence.smallImageKey = smallImage;

	Discord_UpdatePresence( &presence );
}

void CDiscordRPC::RunCallbacks()
{
	if ( !m_bInitialized )
		return;

	Discord_RunCallbacks();
}

void CDiscordRPC::Shutdown()
{
	if ( !m_bInitialized )
		return;

	Discord_Shutdown();
	m_bInitialized = false;
}

