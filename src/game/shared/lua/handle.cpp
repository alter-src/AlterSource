//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "handle.h"
#include "filesystem.h"
#include "tier1/convar.h" // THePixelMoon: SHUT UP, INTELLISENSE!

#include "lua_angle.h"
#ifdef GAME_DLL
#include "lua_baseplayer.h"
#else
#endif // GAME_DLL

LuaHandle *g_pLuaHandle = nullptr;

ConVar sv_gamemode(
    "sv_gamemode",
    "sandbox",
    FCVAR_ARCHIVE | FCVAR_NOTIFY,
    "Sets the current server gamemode"
);

ConVar sv_allow_clientside_lua(
    "sv_allow_clientside_lua",
    "1", // ThePixelMoon: server admins can set this to 0
    /*FCVAR_ARCHIVE |*/ FCVAR_NOTIFY | FCVAR_REPLICATED,
    "Allow clients to execute clientside Lua"
);

static int lua_msgprint( lua_State* L )
{
    int nargs = lua_gettop( L );
    std::string out;
    out.reserve( 128 );

    for ( int i = 1; i <= nargs; ++i )
    {
        const char* s = lua_tostring( L, i );
        if ( s ) out += s;
        else out += "(null)";

        lua_pop( L, 1 ); // pop string from lua_tostring

        if ( i < nargs ) out.push_back( '\t' );
    }

    out.push_back( '\n' );

    Msg( "%s", out.c_str() );

    return 0; // number of Lua return values
}

static const luaL_Reg lua_basefuncs[] = {
    { "print", lua_msgprint },
    { NULL, NULL } // sentinel
};

LuaHandle::LuaHandle()
{
}

LuaHandle::~LuaHandle()
{
	Shutdown();
}

void LuaHandle::Shutdown()
{
	if (L)
        lua_close(L);
}

bool LuaHandle::Initialize()
{
    L = luaL_newstate();
    if ( !L )
        return false;
	
    luaL_openlibs( L );
	luaL_register( L, "_G", lua_basefuncs );

	Lua_RegisterVector( L );
	Lua_RegisterQAngle( L );
#ifdef GAME_DLL
	LuaBasePlayer_Register( L );
#else
#endif // GAME_DLL

	return true;
}

bool LuaHandle::DoString( const char *code, const char *chunkname )
{
    if ( !L || !code )
        return false;

    int status = luaL_loadbuffer( L, code, Q_strlen( code ), chunkname );
    if ( status != 0 )
    {
        const char *err = lua_tostring( L, -1 );
        Warning( "Lua load error: %s\n", err ? err : "(unknown)" );
        lua_pop( L, 1 );
        return false;
    }

    status = lua_pcall( L, 0, 0, 0 );
    if ( status != 0 )
    {
        const char *err = lua_tostring( L, -1 );
        Warning( "Lua runtime error: %s\n", err ? err : "(unknown)" );
        lua_pop( L, 1 );
        return false;
    }

    return true;
}

// ThePixelMoon: scriptpath can also mean the Lua string if it's a string
bool LuaHandle::DoScript( ScriptType scripttype, const char *scriptpath )
{
	switch (scripttype)
	{
		case SCRIPTTYPE_FOLDER:
		{
			// TODO: add folder scripts
			break;
		}
		case SCRIPTTYPE_GAMEMODE:
		{
			char fullPath[ MAX_PATH ];
			Q_snprintf( fullPath, sizeof(fullPath), "gamemodes/%s.lua", scriptpath );

			lua_newtable(L);
			lua_setglobal(L, "GM");

			FileHandle_t file = g_pFullFileSystem->Open( fullPath, "rb", "MOD" );
			if ( file == FILESYSTEM_INVALID_HANDLE )
			{
				Warning( "Failed to open Lua gamemode file '%s'\n", fullPath );
				return false;
			}

			int fileSize = g_pFullFileSystem->Size( file );
			char* buffer = new char[fileSize + 1];
			g_pFullFileSystem->Read( buffer, fileSize, file );
			buffer[fileSize] = '\0'; // null terminate
			g_pFullFileSystem->Close( file );

			if ( luaL_loadbuffer(L, buffer, fileSize, fullPath) != LUA_OK || lua_pcall(L, 0, 0, 0) != LUA_OK )
			{
				const char* err = lua_tostring( L, -1 );
				Warning( "Failed to load Lua gamemode '%s': %s\n", fullPath, err );
				lua_pop( L, 1 );
				delete[] buffer;
				return false;
			}

			delete[] buffer;
			DevMsg( "gamemode '%s' loaded successfully\n", scriptpath );
			break;
		}

		case SCRIPTTYPE_STRING:
		{
			// TODO: add lua strings
			break;
		}

		// ThePixelMoon: the default is file
		default:
		{
			// TODO: add file scripts
			break;
		}
	}

	return true;
}

#ifdef GAME_DLL

static void CC_SV_Lua_DoStr( const CCommand &args )
{
    if ( !UTIL_IsCommandIssuedByServerAdmin() )
    {
        Warning( "sv_lua_dostr: access denied, server admin only.\n" );
        return;
    }

    if ( !g_pLuaHandle || !g_pLuaHandle->getState() )
    {
        Warning( "sv_lua_dostr: Lua not initialized\n" );
        return;
    }

    if ( args.ArgC() < 2 )
    {
        Msg( "usage: sv_lua_dostr <lua code>\n" );
        return;
    }

    const char *code = args.ArgS(); // everything after the command name
    if ( !g_pLuaHandle->DoString( code, "sv_lua_dostr" ) )
    {
        Warning( "sv_lua_dostr: execution failed\n" );
    }
}

static ConCommand sv_lua_dostr(
    "sv_lua_dostr",
    CC_SV_Lua_DoStr,
    "Execute a Lua string on the SERVER",
    FCVAR_GAMEDLL
);

#else

static void CC_CL_Lua_DoStr( const CCommand &args )
{
    if ( !sv_allow_clientside_lua.GetBool() )
    {
        Warning( "cl_lua_dostr: disabled by server (sv_allow_clientside_lua 0)\n" );
        return;
    }

    if ( !g_pLuaHandle || !g_pLuaHandle->getState() )
    {
        Warning( "cl_lua_dostr: Lua not initialized\n" );
        return;
    }

    if ( args.ArgC() < 2 )
    {
        Msg( "usage: cl_lua_dostr <lua code>\n" );
        return;
    }

    const char *code = args.ArgS();
    if ( !g_pLuaHandle->DoString( code, "cl_lua_dostr" ) )
    {
        Warning( "cl_lua_dostr: execution failed\n" );
    }
}

static ConCommand cl_lua_dostr(
    "cl_lua_dostr",
    CC_CL_Lua_DoStr,
    "Execute a Lua string on the CLIENT (server must allow: sv_allow_clientside_lua 1)"
);

#endif // GAME_DLL