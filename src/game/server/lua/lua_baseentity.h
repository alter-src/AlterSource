//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef LUA_BASEENTITY_H
#define LUA_BASEENTITY_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include "lua.hpp"
#include "baseentity.h"

struct LuaEntity {
    CBaseEntity* ent;
};

void LuaBaseEntity_Register( lua_State *L );
int PushEntity( lua_State* L, CBaseEntity* entity );
CBaseEntity* CheckBaseEntity( lua_State* L, int idx );

#endif // LUA_BASEENTITY_H