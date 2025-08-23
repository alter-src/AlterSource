//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "lua_baseplayer.h"
#include "player.h"
#include "filesystem.h"
#include "tier1/convar.h"
#include "lua_angle.h"

CBasePlayer* CheckBasePlayer( lua_State* L, int idx )
{
    LuaPlayer* ply = ( LuaPlayer * )luaL_checkudata( L, idx, "LuaPlayerMeta" );
    if ( !ply || !ply->player )
        luaL_error(L, "Invalid player at stack index %d", idx);

    return ply->player;
}

int PushPlayer( lua_State *L, CBasePlayer *player )
{
	if ( !player )
	{
		lua_pushnil( L );
		return 1;
	}

	LuaPlayer *p = ( LuaPlayer * )lua_newuserdata( L, sizeof( LuaPlayer ) );
	p->player = player;

	luaL_getmetatable( L, "LuaPlayerMeta" );
	lua_setmetatable( L, -2 );

	return 1;
}

static int LuaPlayer_GetHealth( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	lua_pushinteger( L, ply->player->GetHealth() );
	return 1;
}

static int LuaPlayer_SetHealth( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	int health = ( int )luaL_checkinteger( L, 2 );
	ply->player->SetHealth( health );
	return 0;
}

static int LuaPlayer_GetMaxHealth( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	lua_pushinteger( L, ply->player->GetMaxHealth() );
	return 1;
}

static int LuaPlayer_SetMaxHealth( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	int health = ( int )luaL_checkinteger( L, 2 );
	ply->player->SetMaxHealth( health );
	return 0;
}

static int LuaPlayer_GetArmor( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	lua_pushinteger( L, ply->player->ArmorValue() );
	return 1;
}

static int LuaPlayer_SetArmor( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	int armor = ( int )luaL_checkinteger( L, 2 );
	ply->player->SetArmorValue( armor );
	return 0;
}

static int LuaPlayer_GiveItem( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	const char *itemName = luaL_checkstring( L, 2 );
	CBaseEntity *item = ply->player->GiveNamedItem( itemName );

	lua_pushboolean( L, item != nullptr );
	return 1;
}

static int LuaPlayer_GiveAmmo( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
	{
		lua_pushboolean( L, false );
		return 1;
	}

	int amount = ( int )luaL_checkinteger( L, 2 );
	const char *ammoType = luaL_checkstring( L, 3 );

	ply->player->GiveAmmo( amount, ammoType );
	lua_pushboolean( L, true );
	return 1;
}

static int LuaPlayer_HasWeapon( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	const char *weaponName = luaL_checkstring( L, 2 );
	bool hasWeapon = ply->player->HasNamedPlayerItem( weaponName ) != nullptr;
	lua_pushboolean( L, hasWeapon );
	return 1;
}

static int LuaPlayer_RemoveWeapon( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	const char *weaponName = luaL_checkstring( L, 2 );
	CBaseCombatWeapon *weap = ( CBaseCombatWeapon * )ply->player->HasNamedPlayerItem( weaponName );
	if ( weap )
		weap->Remove();

	return 0;
}

static int LuaPlayer_IsAlive( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	lua_pushboolean( L, ply->player->IsAlive() );
	return 1;
}

static int LuaPlayer_SetTeam( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	int team = ( int )luaL_checkinteger( L, 2 );
	ply->player->ChangeTeam( team );
	return 0;
}

static int LuaPlayer_GetTeam( lua_State *L )
{
	LuaPlayer *ply = ( LuaPlayer * )luaL_checkudata( L, 1, "LuaPlayerMeta" );
	if ( !ply || !ply->player )
		return 0;

	lua_pushinteger( L, ply->player->GetTeamNumber() );
	return 1;
}

static const luaL_Reg LuaPlayerMethods[] = 
{
	{ "GetHealth",	LuaPlayer_GetHealth },
	{ "SetHealth",	LuaPlayer_SetHealth },
	{ "GetMaxHealth",LuaPlayer_GetMaxHealth },
	{ "SetMaxHealth",LuaPlayer_SetMaxHealth },
	{ "GetArmor",	LuaPlayer_GetArmor },
	{ "SetArmor",	LuaPlayer_SetArmor },
	{ "GiveItem",	LuaPlayer_GiveItem },
	{ "GiveAmmo",	LuaPlayer_GiveAmmo },
	{ "HasWeapon",	LuaPlayer_HasWeapon },
	{ "RemoveWeapon",LuaPlayer_RemoveWeapon },
	{ "IsAlive",	LuaPlayer_IsAlive },
	{ "SetTeam",	LuaPlayer_SetTeam },
	{ "GetTeam",	LuaPlayer_GetTeam },
	{ NULL, NULL }
};

static void RegisterLuaPlayerMeta( lua_State *L )
{
	luaL_newmetatable( L, "LuaPlayerMeta" );

	luaL_register( L, NULL, LuaPlayerMethods );

	// metatable.__index = metatable
	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );

	// set LuaEntityMeta as parent
	luaL_getmetatable( L, "LuaEntityMeta" );
	lua_setmetatable( L, -2 );

	lua_pop( L, 1 );
}

static int Lua_GetPlayerByIndex( lua_State *L )
{
	int index = luaL_checkinteger( L, 1 );
	CBasePlayer *player = UTIL_PlayerByIndex( index );
	if ( !player )
		return 0;

	return PushPlayer( L, player );
}

static int Lua_GetAllPlayers( lua_State *L )
{
	lua_newtable( L );
	int t = 1;
	for ( int i = 1; i <= MAX_PLAYERS; ++i )
	{
		CBasePlayer *player = UTIL_PlayerByIndex( i );
		if ( !player )
			continue;

		PushPlayer( L, player );
		lua_rawseti( L, -2, t++ );
	}
	return 1;
}

void LuaBasePlayer_Register( lua_State *L )
{
	RegisterLuaPlayerMeta( L );

	static const luaL_Reg funcs[] = 
	{
		{ "GetPlayerByIndex",	Lua_GetPlayerByIndex },
		{ "GetAllPlayers",		Lua_GetAllPlayers },
		{ NULL, NULL }
	};

	luaL_register( L, "baseplayer", funcs );
}
