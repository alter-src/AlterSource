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

void LuaBaseEntity_Register( lua_State *L );
int PushEntity( lua_State* L, CBaseEntity* entity );

struct LuaEntity {
    CBaseEntity* ent;
};

#endif // LUA_BASEENTITY_H