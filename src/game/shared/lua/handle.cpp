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
#include "lua_color.h"
#ifdef GAME_DLL
#include "lua_baseplayer.h"
#include "lua_baseentity.h"
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

        if ( i < nargs ) out.push_back( '\t' );
    }

    out.push_back( '\n' );

    Msg( "%s", out.c_str() );

    return 0; // number of Lua return values
}

static int lua_include(lua_State* L)
{
    const char* filename = lua_tostring(L, 1);
    if (!filename)
    {
        lua_pushstring(L, "include: missing filename");
        lua_error(L);
        return 0;
    }

    char fullPath[MAX_PATH];
    Q_snprintf(fullPath, sizeof(fullPath), "scripts/lua/%s", filename);

    if (!g_pLuaHandle->DoScript(SCRIPTTYPE_FILE, fullPath))
    {
        lua_pushfstring(L, "include: failed to load '%s'", fullPath);
        lua_error(L);
    }

    return 0; // number of Lua return values
}

static const luaL_Reg lua_basefuncs[] = {
    { "print", lua_msgprint },
	{ "include", lua_include },
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
	Lua_RegisterColor( L );
#ifdef GAME_DLL
	LuaBaseEntity_Register( L );
	LuaBasePlayer_Register( L );
#else
#endif // GAME_DLL

	// global flags
#ifdef GAME_DLL
	lua_pushboolean( L, 1 );
	lua_setglobal( L, "IS_SERVER" );

	lua_pushboolean( L, 0 );
	lua_setglobal( L, "IS_CLIENT" );
#else
	lua_pushboolean( L, 0 );
	lua_setglobal( L, "IS_SERVER" );

	lua_pushboolean( L, 1 );
	lua_setglobal( L, "IS_CLIENT" );
#endif

    lua_newtable( L );
    lua_setglobal( L, "GM" );

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

bool LuaHandle::CallHookInternal( lua_State *L, const char *tableName, const char *hookName, int numArgs, bool stopOnFalse, va_list args )
{
    lua_getglobal(L, tableName); // stack: table?
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return true; // no Hooks table -> allow
    }

    lua_getfield(L, -1, hookName); // stack: table, Hooks[hookName]?
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 2);
        return true; // no hook registered -> allow
    }

    auto push_args = [&](va_list vargs) {
        for (int i = 0; i < numArgs; ++i)
        {
            int type = va_arg(vargs, int);
            if (type == 0) // int
            {
                int val = va_arg(vargs, int);
                lua_pushinteger(L, val);
            }
            else if (type == 1) // const char*
            {
                const char* val = va_arg(vargs, const char*);
                lua_pushstring(L, val);
            }
            else if (type == 2) // double
            {
                double val = va_arg(vargs, double);
                lua_pushnumber(L, val);
            }
            else if (type == 3) // bool (promoted to int)
            {
                int val = va_arg(vargs, int);
                lua_pushboolean(L, val);
            }
#ifdef GAME_DLL
            else if (type == 4) // CBasePlayer*
            {
                CBasePlayer* player = va_arg(vargs, CBasePlayer*);
                PushPlayer(L, player);
            }
            else if (type == 5) // CBaseEntity*
            {
                CBaseEntity* entity = va_arg(vargs, CBaseEntity*);
                PushEntity(L, entity);
            }
#endif
            else
            {
                // unknown type, push nil to keep stack aligned
                lua_pushnil(L);
            }
        }
    };

    if (lua_isfunction(L, -1))
    {
        lua_pushvalue(L, -1); // copy function
        va_list args_copy;
        va_copy(args_copy, args);
        push_args(args_copy);
        va_end(args_copy);

        int retCount = stopOnFalse ? 1 : 0;
        if (lua_pcall(L, numArgs, retCount, 0) != LUA_OK)
        {
            const char *err = lua_tostring(L, -1);
            Warning("Hook '%s' runtime error: %s\n", hookName, err ? err : "(unknown)");
            lua_pop(L, 1); // pop error
            lua_pop(L, 2); // pop original value and Hooks table
            return true; // treat runtime errors as "don't block"
        }

        if (retCount == 1)
        {
            bool result = lua_toboolean(L, -1);
            lua_pop(L, 1); // pop return value
            lua_pop(L, 2); // pop original value and Hooks
            return result;
        }

        lua_pop(L, 1); // pop original value
        lua_pop(L, 1); // pop Hooks table
        return true;
    }

    // If the hook is a table, iterate functions inside it
    if (lua_istable(L, -1))
    {
        lua_pushnil(L); // first key for lua_next
        while (lua_next(L, -2) != 0)
        {
            // stack: table, table[hookName], key, value
            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, -1); // copy function for call
                va_list args_copy;
                va_copy(args_copy, args);
                push_args(args_copy);
                va_end(args_copy);

                int retCount = stopOnFalse ? 1 : 0;
                if (lua_pcall(L, numArgs, retCount, 0) != LUA_OK)
                {
                    const char* err = lua_tostring(L, -1);
                    Warning("Hook '%s' runtime error: %s\n", hookName, err ? err : "(unknown)");
                    lua_pop(L, 1); // pop error
                    lua_pop(L, 1); // pop original value
                    continue; // proceed to next function
                }

                if (retCount == 1)
                {
                    bool result = lua_toboolean(L, -1);
                    lua_pop(L, 1); // pop return value
                    lua_pop(L, 1); // pop original value
                    if (!result)
                    {
                        lua_pop(L, 2); // pop table[hookName] and table
                        return false;
                    }
                }
                else
                {
                    lua_pop(L, 1); // pop original value
                }
            }
            else
            {
                lua_pop(L, 1); // not a function, pop value
            }
        }

        lua_pop(L, 2); // pop table[hookName] and Hooks table
        return true;
    }

    // anything else (number/string/etc) -> allow
    lua_pop(L, 2);
    return true;
}

bool LuaHandle::CallGMHook( const char *hookName, int numArgs, ... )
{
	va_list args;
	va_start( args, numArgs );
	bool result = CallHookInternal( L, "GM", hookName, numArgs, false, args );
	va_end( args );
	return result;
}

bool LuaHandle::CallHook( const char *hookName, int numArgs, ... )
{
	va_list args;
	va_start( args, numArgs );
	bool result = CallHookInternal( L, "Hooks", hookName, numArgs, true, args );
	va_end( args );
	return result;
}

// ThePixelMoon: scriptpath can also mean the Lua string if it's a string
bool LuaHandle::DoScript( ScriptType scripttype, const char *scriptpath )
{
	switch (scripttype)
	{
		case SCRIPTTYPE_FOLDER:
		{
			char searchPath[ MAX_PATH ];
			Q_snprintf( searchPath, sizeof( searchPath ), "%s/*.lua", scriptpath );

			FileFindHandle_t findHandle;
			const char *filename = g_pFullFileSystem->FindFirstEx( searchPath, "MOD", &findHandle );

			if ( !filename )
			{
				Warning( "Lua folder '%s' not found or empty\n", scriptpath );
				return false;
			}

			CUtlVector< char * > files;

			// collect all files
			do
			{
				if ( filename[0] == '.' ) // skip '.' and '..'
					continue;

				char *copy = strdup( filename );
				files.AddToTail( copy );
			}
			while ( ( filename = g_pFullFileSystem->FindNext( findHandle ) ) != nullptr );

			g_pFullFileSystem->FindClose( findHandle );

			for ( int i = 1; i < files.Count(); ++i )
			{
				char *key = files[i];
				int j = i - 1;
				while ( j >= 0 && Q_stricmp( files[j], key ) > 0 )
				{
					files[j + 1] = files[j];
					--j;
				}
				files[j + 1] = key;
			}

			bool success = true;

			for ( int i = 0; i < files.Count(); ++i )
			{
				char fullPath[ MAX_PATH ];
				Q_snprintf( fullPath, sizeof( fullPath ), "%s/%s", scriptpath, files[i] );

				FileHandle_t file = g_pFullFileSystem->Open( fullPath, "rb", "MOD" );
				if ( file == FILESYSTEM_INVALID_HANDLE )
				{
					Warning( "Failed to open Lua file '%s'\n", fullPath );
					success = false;
					continue;
				}

				int fileSize = g_pFullFileSystem->Size( file );
				char *buffer = new char[ fileSize + 1 ];
				g_pFullFileSystem->Read( buffer, fileSize, file );
				buffer[fileSize] = '\0';
				g_pFullFileSystem->Close( file );

				if ( luaL_loadbuffer( L, buffer, fileSize, fullPath ) != LUA_OK || lua_pcall( L, 0, 0, 0 ) != LUA_OK )
				{
					const char *err = lua_tostring( L, -1 );
					Warning( "Failed to load Lua script '%s': %s\n", fullPath, err ? err : "(unknown)" );
					lua_pop( L, 1 );
					success = false;
				}
				else
				{
					DevMsg( "Loaded Lua file: %s\n", fullPath );
				}

				delete[] buffer;
				free( files[i] );
			}

			files.Purge();
			break;
		}
		case SCRIPTTYPE_GAMEMODE:
		{
			char fullPath[ MAX_PATH ];
			Q_snprintf( fullPath, sizeof(fullPath), "scripts/lua/gamemodes/%s.lua", scriptpath );

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
			DoString( scriptpath, "console" );
			break;
		}

		// ThePixelMoon: the default is file
		default:
		{
			FileHandle_t file = g_pFullFileSystem->Open( scriptpath, "rb", "MOD" );
			if ( file == FILESYSTEM_INVALID_HANDLE )
			{
				Warning( "Failed to open Lua file '%s'\n", scriptpath );
				return false;
			}

			int fileSize = g_pFullFileSystem->Size( file );
			char *buffer = new char[ fileSize + 1 ];
			g_pFullFileSystem->Read( buffer, fileSize, file );
			buffer[fileSize] = '\0';
			g_pFullFileSystem->Close( file );

			if ( luaL_loadbuffer( L, buffer, fileSize, scriptpath ) != LUA_OK || lua_pcall( L, 0, 0, 0 ) != LUA_OK )
			{
				const char *err = lua_tostring( L, -1 );
				Warning( "Failed to load Lua file '%s': %s\n", scriptpath, err ? err : "(unknown)" );
				lua_pop( L, 1 );
				delete[] buffer;
				return false;
			}

			delete[] buffer;
			DevMsg( "Lua file '%s' loaded successfully\n", scriptpath );
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
    "Execute a Lua string on the SERVER (for server admins only)",
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
    "Execute a Lua string on the CLIENT (server must have sv_allow_clientside_lua set to 1)"
);
#endif // GAME_DLL