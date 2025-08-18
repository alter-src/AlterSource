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

int PushPlayer( lua_State* L, CBasePlayer* player )
{
    if (!player)
    {
        lua_pushnil(L);
        return 1;
    }

    LuaPlayer* p = (LuaPlayer*)lua_newuserdata(L, sizeof(LuaPlayer));
    p->player = player;

    luaL_getmetatable(L, "LuaPlayerMeta");
    lua_setmetatable(L, -2);

    return 1;
}

static int LuaPlayer_GetPos(lua_State* L)
{
    LuaPlayer* ply = (LuaPlayer*)luaL_checkudata(L, 1, "LuaPlayerMeta");
    Vector pos = ply->player->GetAbsOrigin();

    lua_newtable(L);
    lua_pushnumber(L, pos.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, pos.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, pos.z); lua_setfield(L, -2, "z");

    return 1;
}

static int LuaPlayer_SetPos(lua_State* L)
{
    LuaPlayer* ply = (LuaPlayer*)luaL_checkudata(L, 1, "LuaPlayerMeta");
    if (!ply || !ply->player)
        return 0;

    luaL_checktype(L, 2, LUA_TTABLE); // table with x, y, z
    Vector newPos;
    
    lua_getfield(L, 2, "x"); newPos.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, 2, "y"); newPos.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, 2, "z"); newPos.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);

    ply->player->SetAbsOrigin(newPos);

    return 0;
}

static int LuaPlayer_GiveWeapon(lua_State* L)
{
    LuaPlayer* ply = (LuaPlayer*)luaL_checkudata(L, 1, "LuaPlayerMeta");
    const char* weaponName = luaL_checkstring(L, 2);

    CBaseEntity* weap = ply->player->GiveNamedItem(weaponName);
    lua_pushboolean(L, weap != nullptr);
    return 1;
}

static int LuaPlayer_GiveAmmo(lua_State* L)
{
    LuaPlayer* ply = (LuaPlayer*)luaL_checkudata(L, 1, "LuaPlayerMeta");
    int amount = (int)luaL_checkinteger(L, 2);
    const char* ammoType = luaL_checkstring(L, 3);

    if (!ply || !ply->player)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    ply->player->GiveAmmo(amount, ammoType);
    lua_pushboolean(L, true);
    return 1;
}

static void RegisterLuaPlayerMeta(lua_State* L)
{
    luaL_newmetatable(L, "LuaPlayerMeta");

    lua_newtable(L);

    lua_pushcfunction(L, LuaPlayer_GetPos);
    lua_setfield(L, -2, "GetPos");

    lua_pushcfunction(L, LuaPlayer_SetPos);
    lua_setfield(L, -2, "SetPos");

    lua_pushcfunction(L, LuaPlayer_GiveWeapon);
    lua_setfield(L, -2, "GiveWeapon");

    lua_pushcfunction(L, LuaPlayer_GiveAmmo);
    lua_setfield(L, -2, "GiveAmmo");

    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);
}

static int Lua_GetPlayerByIndex(lua_State* L)
{
    int index = luaL_checkinteger(L, 1);
    CBasePlayer* player = UTIL_PlayerByIndex(index);
    return PushPlayer(L, player);
}

static int Lua_GetAllPlayers(lua_State* L)
{
    lua_newtable(L);
    int t = 1;
    for (int i = 1; i <= MAX_PLAYERS; ++i)
    {
        CBasePlayer* player = UTIL_PlayerByIndex(i);
        if (player)
        {
            PushPlayer(L, player);
            lua_rawseti(L, -2, t++);
        }
    }
    return 1;
}

void LuaBasePlayer_Register(lua_State* L)
{
	RegisterLuaPlayerMeta(L);

	static const luaL_Reg funcs[] = {
		{ "GetPlayerByIndex", Lua_GetPlayerByIndex },
		{ "GetAllPlayers", Lua_GetAllPlayers },
		{ NULL, NULL }
	};

	luaL_register(L, "baseplayer", funcs);
}

