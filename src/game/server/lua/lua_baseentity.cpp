//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "lua_baseentity.h"
#include "lua_angle.h"

CBaseEntity* CheckBaseEntity( lua_State* L, int idx )
{
    LuaEntity* ent = ( LuaEntity * )luaL_checkudata( L, idx, "LuaEntityMeta" );
    if ( !ent || !ent->ent )
        luaL_error(L, "Invalid entity at stack index %d", idx);

    return ent->ent;
}

int PushEntity( lua_State* L, CBaseEntity* entity )
{
	if ( !entity )
	{
		lua_pushnil( L );
		return 1;
	}

	LuaEntity *e = ( LuaEntity * )lua_newuserdata( L, sizeof( LuaEntity ) );
	e->ent = entity;

	luaL_getmetatable( L, "LuaEntityMeta" );
	lua_setmetatable( L, -2 );

	return 1;
}

static int LuaEntity_GetPos( lua_State *L )
{
	LuaEntity *ent = ( LuaEntity * )luaL_checkudata( L, 1, "LuaEntityMeta" );
	if ( !ent || !ent->ent )
		return 0;

	return PushVector( L, ent->ent->GetAbsOrigin() );
}

static int LuaEntity_SetPos( lua_State *L )
{
	LuaEntity *ent = ( LuaEntity * )luaL_checkudata( L, 1, "LuaEntityMeta" );
	if ( !ent || !ent->ent )
		return 0;

	LuaVector *v = ( LuaVector * )luaL_checkudata( L, 2, "LuaVectorMeta" );
	ent->ent->SetAbsOrigin( v->vec );

	return 0;
}

static int LuaEntity_GetAngles( lua_State *L )
{
	LuaEntity *ent = ( LuaEntity * )luaL_checkudata( L, 1, "LuaEntityMeta" );
	if ( !ent || !ent->ent )
		return 0;

	return PushQAngle( L, ent->ent->GetAbsAngles() );
}

static int LuaEntity_SetAngles( lua_State *L )
{
	LuaEntity *ent = ( LuaEntity * )luaL_checkudata( L, 1, "LuaEntityMeta" );
	if ( !ent || !ent->ent )
		return 0;

	LuaQAngle *a = ( LuaQAngle * )luaL_checkudata( L, 2, "LuaQAngleMeta" );
	ent->ent->SetAbsAngles( a->ang );

	return 0;
}

static int LuaEntity_GetVelocity( lua_State *L )
{
	LuaEntity *ent = ( LuaEntity * )luaL_checkudata( L, 1, "LuaEntityMeta" );
	if ( !ent || !ent->ent )
		return 0;

	return PushVector( L, ent->ent->GetAbsVelocity() );
}

static int LuaEntity_SetVelocity( lua_State *L )
{
	LuaEntity *ent = ( LuaEntity * )luaL_checkudata( L, 1, "LuaEntityMeta" );
	if ( !ent || !ent->ent )
		return 0;

	LuaVector *v = ( LuaVector * )luaL_checkudata( L, 2, "LuaVectorMeta" );
	ent->ent->SetAbsVelocity( v->vec );

	return 0;
}

static int LuaEntity_SetModel( lua_State *L )
{
	LuaEntity *ent = ( LuaEntity * )luaL_checkudata( L, 1, "LuaEntityMeta" );
	if ( !ent || !ent->ent )
		return 0;

	const char *modelPath = luaL_checkstring( L, 2 );

	bool doPrecache = false;
	if ( lua_gettop( L ) >= 3 && lua_isboolean( L, 3 ) )
		doPrecache = lua_toboolean( L, 3 ) != 0;

	if ( doPrecache )
		ent->ent->PrecacheModel( modelPath );

	ent->ent->SetModel( modelPath );
	return 0;
}

static int LuaEntity_GetModel( lua_State *L )
{
	LuaEntity *ent = ( LuaEntity * )luaL_checkudata( L, 1, "LuaEntityMeta" );
	if ( !ent || !ent->ent )
		return 0;

	const char *model = STRING( ent->ent->GetModelName() );
	lua_pushstring( L, model ? model : "" );
	return 1;
}

static int LuaEntity_Remove( lua_State *L )
{
	LuaEntity *ent = ( LuaEntity * )luaL_checkudata( L, 1, "LuaEntityMeta" );
	if ( !ent || !ent->ent )
		return 0;

	ent->ent->Remove();
	return 0;
}

static int LuaEntity_IsValid( lua_State *L )
{
	LuaEntity *ent = ( LuaEntity * )luaL_checkudata( L, 1, "LuaEntityMeta" );
	lua_pushboolean( L, ent && ent->ent && ent->ent->IsEFlagSet( EFL_KILLME ) == false );
	return 1;
}

static const luaL_Reg LuaEntityMethods[] = 
{
	{ "GetPos",		LuaEntity_GetPos },
	{ "SetPos",		LuaEntity_SetPos },
	{ "GetAngles",	LuaEntity_GetAngles },
	{ "SetAngles",	LuaEntity_SetAngles },
	{ "GetVelocity",LuaEntity_GetVelocity },
	{ "SetVelocity",LuaEntity_SetVelocity },
	{ "SetModel",	LuaEntity_SetModel },
	{ "GetModel",	LuaEntity_GetModel },
	{ "Remove",		LuaEntity_Remove },
	{ "IsValid",	LuaEntity_IsValid },
	{ NULL, NULL }
};

static void RegisterLuaEntityMeta( lua_State *L )
{
	luaL_newmetatable( L, "LuaEntityMeta" );

	luaL_register( L, NULL, LuaEntityMethods );

	// metatable.__index = metatable
	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );

	lua_pop( L, 1 );
}

static int Lua_GetEntityByIndex( lua_State *L )
{
	int index = luaL_checkinteger( L, 1 );

	edict_t *edict = engine->PEntityOfEntIndex( index );
	if ( !edict )
		return 0;

	// convert edict -> CBaseEntity
	CBaseEntity *ent = CBaseEntity::Instance( edict );
	if ( !ent )
		return 0;

	return PushEntity( L, ent );
}

static int Lua_GetAllEntities( lua_State *L )
{
	lua_newtable( L );
	int t = 1;

	// gpGlobals->maxEntities is the upper bound
	for ( int i = 0; i < gpGlobals->maxEntities; ++i )
	{
		edict_t *edict = engine->PEntityOfEntIndex( i );
		if ( !edict )
			continue;

		CBaseEntity *ent = CBaseEntity::Instance( edict );
		if ( !ent )
			continue;

		PushEntity( L, ent );
		lua_rawseti( L, -2, t++ );
	}

	return 1;
}

void LuaBaseEntity_Register( lua_State *L )
{
	RegisterLuaEntityMeta( L );

	static const luaL_Reg funcs[] = 
	{
		{ "GetEntityByIndex",	Lua_GetEntityByIndex },
		{ "GetAllEntities",		Lua_GetAllEntities },
		{ NULL, NULL }
	};

	luaL_register( L, "baseentity", funcs );
}
