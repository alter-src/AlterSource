//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef LUA_ANGLE_H
#define LUA_ANGLE_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include "lua.hpp"
#include "mathlib/vector.h"

struct LuaVector {
    Vector vec;
};

struct LuaQAngle {
    QAngle ang;
};

void Lua_RegisterVector( lua_State *L );
void Lua_RegisterQAngle( lua_State *L );

int PushVector( lua_State *L, const Vector &v );
int PushQAngle( lua_State *L, const QAngle &a );

LuaQAngle *CheckQAngle( lua_State *L, int idx );
LuaVector *CheckVector( lua_State *L, int idx );

#endif // LUA_ANGLE_H