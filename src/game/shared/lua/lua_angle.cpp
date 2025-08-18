//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "lua_angle.h"

static LuaVector *CheckVector( lua_State *L, int idx )
{
    return (LuaVector *)luaL_checkudata( L, idx, "LuaVectorMeta" );
}

static LuaQAngle *CheckQAngle( lua_State *L, int idx )
{
    return (LuaQAngle *)luaL_checkudata( L, idx, "LuaQAngleMeta" );
}

//-----------------------------------------------------------------------------
// Vector binding
//-----------------------------------------------------------------------------
int PushVector( lua_State *L, const Vector &v )
{
    LuaVector *lv = (LuaVector *)lua_newuserdata( L, sizeof( LuaVector ) );
    lv->vec = v;

    luaL_getmetatable( L, "LuaVectorMeta" );
    lua_setmetatable( L, -2 );

    return 1;
}

static int LuaVector_Get(lua_State *L)
{
    LuaVector *v = CheckVector(L, 1);

    lua_newtable(L);
    lua_pushnumber(L, v->vec.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, v->vec.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, v->vec.z); lua_setfield(L, -2, "z");

    return 1;
}

static int LuaVector_Set( lua_State *L )
{
    LuaVector *v = CheckVector( L, 1 );
    v->vec.x = (float)luaL_checknumber( L, 2 );
    v->vec.y = (float)luaL_checknumber( L, 3 );
    v->vec.z = (float)luaL_checknumber( L, 4 );
    return 0;
}

static int LuaVector_ToString( lua_State *L )
{
    LuaVector *v = CheckVector( L, 1 );
    lua_pushfstring( L, "Vector(%.2f, %.2f, %.2f)", v->vec.x, v->vec.y, v->vec.z );
    return 1;
}

static int LuaVector_Add( lua_State *L )
{
    LuaVector *a = CheckVector( L, 1 );
    LuaVector *b = CheckVector( L, 2 );
    return PushVector( L, a->vec + b->vec );
}

static int LuaVector_Sub( lua_State *L )
{
    LuaVector *a = CheckVector( L, 1 );
    LuaVector *b = CheckVector( L, 2 );
    return PushVector( L, a->vec - b->vec );
}

static const luaL_Reg LuaVectorMethods[] = {
    { "Get",     LuaVector_Get },
    { "Set",     LuaVector_Set },
    { NULL, NULL }
};

static const luaL_Reg LuaVectorMeta[] = {
    { "__tostring", LuaVector_ToString },
    { "__add",      LuaVector_Add },
    { "__sub",      LuaVector_Sub },
    { NULL, NULL }
};

static int LuaVector_New(lua_State *L)
{
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);
    return PushVector(L, Vector(x, y, z));
}

void Lua_RegisterVector(lua_State *L)
{
    luaL_newmetatable(L, "LuaVectorMeta");

    // register metamethods
    luaL_register(L, NULL, LuaVectorMeta);

    // register methods in __index
    luaL_register(L, NULL, LuaVectorMethods);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);

    // global constructor
    lua_pushcfunction(L, LuaVector_New);
    lua_setglobal(L, "Vector");
}

//-----------------------------------------------------------------------------
// QAngle binding
//-----------------------------------------------------------------------------
int PushQAngle( lua_State *L, const QAngle &a )
{
    LuaQAngle *la = (LuaQAngle *)lua_newuserdata( L, sizeof( LuaQAngle ) );
    la->ang = a;

    luaL_getmetatable( L, "LuaQAngleMeta" );
    lua_setmetatable( L, -2 );

    return 1;
}

static int LuaQAngle_Get(lua_State *L)
{
    LuaQAngle *a = CheckQAngle(L, 1);

    lua_newtable(L);
    lua_pushnumber(L, a->ang.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, a->ang.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, a->ang.z); lua_setfield(L, -2, "z");

    return 1;
}

static int LuaQAngle_Set( lua_State *L )
{
    LuaQAngle *a = CheckQAngle( L, 1 );
    a->ang.x = (float)luaL_checknumber( L, 2 );
    a->ang.y = (float)luaL_checknumber( L, 3 );
    a->ang.z = (float)luaL_checknumber( L, 4 );
    return 0;
}

static int LuaQAngle_ToString( lua_State *L )
{
    LuaQAngle *a = CheckQAngle( L, 1 );
    lua_pushfstring( L, "QAngle(%.2f, %.2f, %.2f)", a->ang.x, a->ang.y, a->ang.z );
    return 1;
}

static const luaL_Reg LuaQAngleMethods[] = {
    { "Get", LuaQAngle_Get },
    { "Set", LuaQAngle_Set },
    { NULL, NULL }
};

static const luaL_Reg LuaQAngleMeta[] = {
    { "__tostring", LuaQAngle_ToString },
    { NULL, NULL }
};

// optional: global constructor
static int LuaQAngle_New(lua_State *L)
{
    float p = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float r = (float)luaL_checknumber(L, 3);
    return PushQAngle(L, QAngle(p, y, r));
}

void Lua_RegisterQAngle(lua_State *L)
{
    luaL_newmetatable(L, "LuaQAngleMeta");

    // register metamethods
    luaL_register(L, NULL, LuaQAngleMeta);

    // register methods in __index
    luaL_register(L, NULL, LuaQAngleMethods);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);

    // global constructor
    lua_pushcfunction(L, LuaQAngle_New);
    lua_setglobal(L, "QAngle");
}
