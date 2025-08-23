//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "lua_color.h"

Color* CheckColor( lua_State *L, int idx )
{
	LuaColor *lc = ( LuaColor * )luaL_checkudata( L, idx, "LuaColorMeta" );
	if ( !lc )
		luaL_error( L, "Invalid Color at stack index %d", idx );

	return &lc->col;
}

int PushColor( lua_State *L, const Color &c )
{
	LuaColor *lc = ( LuaColor * )lua_newuserdata( L, sizeof( LuaColor ) );
	lc->col = c;

	luaL_getmetatable( L, "LuaColorMeta" );
	lua_setmetatable( L, -2 );

	return 1;
}

static int LuaColor_Get( lua_State *L )
{
	Color *c = CheckColor( L, 1 );

	lua_newtable( L );
	lua_pushinteger( L, c->r() ); lua_setfield( L, -2, "r" );
	lua_pushinteger( L, c->g() ); lua_setfield( L, -2, "g" );
	lua_pushinteger( L, c->b() ); lua_setfield( L, -2, "b" );
	lua_pushinteger( L, c->a() ); lua_setfield( L, -2, "a" );

	return 1;
}

static int LuaColor_Set( lua_State *L )
{
	Color *c = CheckColor( L, 1 );

	c->SetColor(
		( int )luaL_checkinteger( L, 2 ),
		( int )luaL_checkinteger( L, 3 ),
		( int )luaL_checkinteger( L, 4 ),
		lua_gettop( L ) >= 5 ? ( int )luaL_checkinteger( L, 5 ) : 255
	);

	return 0;
}

static int LuaColor_ToString( lua_State *L )
{
	Color *c = CheckColor( L, 1 );
	lua_pushfstring( L, "Color(%d, %d, %d, %d)", c->r(), c->g(), c->b(), c->a() );
	return 1;
}

static int LuaColor_New( lua_State *L )
{
	int r = ( int )luaL_checkinteger( L, 1 );
	int g = ( int )luaL_checkinteger( L, 2 );
	int b = ( int )luaL_checkinteger( L, 3 );
	int a = lua_gettop( L ) >= 4 ? ( int )luaL_checkinteger( L, 4 ) : 255;

	return PushColor( L, Color( r, g, b, a ) );
}

static const luaL_Reg ColorMethods[] =
{
	{ "Get",		LuaColor_Get },
	{ "Set",		LuaColor_Set },
	{ "__tostring",	LuaColor_ToString },
	{ NULL, NULL }
};

void Lua_RegisterColor( lua_State *L )
{
	luaL_newmetatable( L, "LuaColorMeta" );

	luaL_register( L, NULL, ColorMethods );

	// metatable.__index = metatable
	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );

	lua_pop( L, 1 );

	// global constructor
	lua_pushcfunction( L, LuaColor_New );
	lua_setglobal( L, "Color" );
}