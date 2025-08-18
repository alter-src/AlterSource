//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef LUA_BASEPLAYER_H
#define LUA_BASEPLAYER_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include "lua.hpp"

void LuaBasePlayer_Register( lua_State *L );
int PushPlayer( lua_State* L, CBasePlayer* player );

struct LuaPlayer {
    CBasePlayer* player;
};

#endif // LUA_BASEPLAYER_H