//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef LUA_COLOR_H
#define LUA_COLOR_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include "lua.hpp"
#include "Color.h"

struct LuaColor {
    Color col;
};

int PushColor(lua_State* L, const Color& c);
void Lua_RegisterColor(lua_State* L);
Color* CheckColor(lua_State* L, int idx);

#endif // LUA_COLOR_H