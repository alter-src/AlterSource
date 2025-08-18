//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "handle.h"
#include "filesystem.h"
#include "tier1/convar.h" // THePixelMoon: SHUT UP, INTELLISENSE!

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

static int lua_msgprint(lua_State* L)
{
    int nargs = lua_gettop(L);
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
    {"print", lua_msgprint},
    {NULL, NULL} // sentinel
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

#ifdef GAME_DLL
	LuaBasePlayer_Register( L );
#else
#endif // GAME_DLL

	return true;
}

bool LuaHandle::DoScript( ScriptType scripttype, const char *scriptpath )
{
	switch (scripttype)
	{
		case SCRIPTTYPE_FOLDER:
		{
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

		// ThePixelMoon: the default is file
		default:
		{
			break;
		}
	}

	return true;
}