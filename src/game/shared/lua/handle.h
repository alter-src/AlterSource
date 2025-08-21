//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef HANDLE_H
#define HANDLE_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include "lua.hpp"
#include <string>
#ifdef GAME_DLL
#include "lua_baseplayer.h"
#else
#endif // GAME_DLL

#ifndef LUA_OK
#define LUA_OK 0
#endif // LUA_OK

enum ScriptType
{
	SCRIPTTYPE_NONE = 0,

	SCRIPTTYPE_FILE,
	SCRIPTTYPE_FOLDER,
	SCRIPTTYPE_GAMEMODE,
	SCRIPTTYPE_STRING,

	SCRIPTTYPE_LAST
};

inline void PushArg(lua_State* L, const int& value) { lua_pushinteger(L, value); }
inline void PushArg(lua_State* L, const float& value) { lua_pushnumber(L, value); }
inline void PushArg(lua_State* L, const double& value) { lua_pushnumber(L, value); }
inline void PushArg(lua_State* L, const bool& value) { lua_pushboolean(L, value); }
inline void PushArg(lua_State* L, const char* value) { lua_pushstring(L, value); }
inline void PushArg(lua_State* L, const std::string& value) { lua_pushlstring(L, value.c_str(), value.size()); }

// TODO: make a parent class Handle for multiple language support
class LuaHandle
{
public:
	LuaHandle();
	~LuaHandle();

	bool Initialize();
	void Shutdown();

	bool DoScript( ScriptType scripttype, const char *scriptpath );

	lua_State *getState() const { return L; }
    bool DoString( const char *code, const char *chunkname = "console" );
	
	// ThePixelMoon: why..
	bool CallHookInternal( lua_State *L, const char *tableName, const char *hookName, int numArgs, bool stopOnFalse, va_list args );
	bool CallGMHook( const char* hookName, int numArgs, ... );
	bool CallHook( const char *hookName, int numArgs, ... );

private:
	lua_State *L;

};

extern LuaHandle *g_pLuaHandle;

// ThePixelMoon: okay, that's a bit cleaner
#define GENERAL_HOOK(name, ...) \
    ( (g_pLuaHandle && g_pLuaHandle->getState()) ? g_pLuaHandle->CallHook((name), __VA_ARGS__) : true )
#define GAMEMODE_HOOK(name, ...) \
    ( (g_pLuaHandle && g_pLuaHandle->getState()) ? g_pLuaHandle->CallGMHook((name), __VA_ARGS__) : true )

#endif // HANDLE_H