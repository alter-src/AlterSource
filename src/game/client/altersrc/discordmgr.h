//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef DISCORDMGR_H
#define DISCORDMGR_H
#ifdef _WIN32
#pragma once
#endif // DISCORDMGR_H

#define AS_APPID "1356635310037008535"

class CDiscordRPC
{
public:
	CDiscordRPC();
	~CDiscordRPC();

	void Init( const char* appId );
	void Update( const char* state, const char* details, const char* largeImage = "default", const char* smallImage = nullptr );
	void RunCallbacks();
	void Shutdown();

private:
	bool m_bInitialized;
};

extern CDiscordRPC g_DiscordRPC;

#endif // DISCORDMGR_H