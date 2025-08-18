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
	bool CallGMHook( const char* hookName, int numArgs, ... )
	{
		if (!L)
			return false;

		lua_getglobal(L, "GM");           // stack: GM
		lua_getfield(L, -1, hookName);    // stack: GM, func

		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 2); // pop nil + GM table
			return false;
		}

		lua_pushvalue(L, -2); // stack: GM, func, GM(self)

		va_list args;
		va_start(args, numArgs);
		for (int i = 0; i < numArgs; ++i)
		{
			int type = va_arg(args, int);
			if (type == 0)
			{
				int val = va_arg(args, int);
				lua_pushinteger(L, val);
			}
			else if (type == 1)
			{
				const char* val = va_arg(args, const char*);
				lua_pushstring(L, val);
			}
			else if (type == 2)
			{
				double val = va_arg(args, double);
				lua_pushnumber(L, val);
			}
			else if (type == 3)
			{
				bool val = va_arg(args, int); // bool promoted to int
				lua_pushboolean(L, val);
			}
#ifdef GAME_DLL
			else if (type == 4) // player
			{
				CBasePlayer* player = va_arg(args, CBasePlayer*);
				PushPlayer(L, player); // pushes userdata with metatable
			}
#endif
		}
		va_end(args);

		int totalArgs = numArgs + 1;

		if (lua_pcall(L, totalArgs, 0, 0) != LUA_OK)
		{
			const char* err = lua_tostring(L, -1);
			Warning("Lua hook '%s' error: %s\n", hookName, err);
			lua_pop(L, 1);
			lua_pop(L, 1); // pop GM table
			return false;
		}

		// pop GM table (it was left on the stack by lua_getglobal at the start)
		lua_pop(L, 1);
		return true;
	}

private:
	lua_State *L;

};

extern LuaHandle *g_pLuaHandle;

// ThePixelMoon: okay, that's a bit cleaner
#define GAMEMODE_HOOK(name, ...) g_pLuaHandle->CallGMHook(name, __VA_ARGS__)

#endif // HANDLE_H