//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "mountlogic.h"
#include "filesystem.h"
#include "tier1/KeyValues.h"
#include "alter_dbg.h"

#if defined( _WIN32 )
	#include <windows.h>
#else
	#include <dirent.h>
	#include <sys/stat.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static bool EndsWith( const char *str, const char *suffix )
{
	size_t lenstr = strlen( str );
	size_t lensuf = strlen( suffix );
	if ( lensuf > lenstr )
		return false;

	return ( strcmp( str + lenstr - lensuf, suffix ) == 0 );
}

bool LoadGameMounts()
{
	KeyValues *pKeyValues = new KeyValues( "mounts" );

	if ( !pKeyValues->LoadFromFile( g_pFullFileSystem, "cfg/mount.txt", "MOD", true ) )
	{
		ALTER_WARNING( "mount.txt not found\n" );
		pKeyValues->deleteThis();
		return false;
	}

	KeyValues *pSubKey = pKeyValues->GetFirstSubKey();
	if ( !pSubKey )
	{
		ALTER_WARNING( "no mounts found in mount.txt\n" );
		pKeyValues->deleteThis();
		return false;
	}

	while ( pSubKey )
	{
		const char *path = pSubKey->GetString();
		if ( !path || !path[0] )
		{
			ALTER_WARNING( "empty mount path in mount.txt\n" );
			pSubKey = pSubKey->GetNextKey();
			continue;
		}

		ALTER_DEVMSG( "mounting path: %s\n", path );

		g_pFullFileSystem->AddSearchPath( path, "GAME" );

		char pathbuf[ MAX_PATH ];
		V_strncpy( pathbuf, path, sizeof( pathbuf ) );

		// remove trailing slash
		int plen = Q_strlen( pathbuf );
		while ( plen > 0 && ( pathbuf[plen - 1] == '/' || pathbuf[plen - 1] == '\\' ) )
		{
			pathbuf[ plen - 1 ] = '\0';
			plen--;
		}

#if defined( _WIN32 )
		char searchPattern[ MAX_PATH ];
		Q_snprintf( searchPattern, sizeof( searchPattern ), "%s\\*_dir.vpk", pathbuf );

		WIN32_FIND_DATAA findData;
		HANDLE hFind = FindFirstFileA( searchPattern, &findData );
		if ( hFind != INVALID_HANDLE_VALUE )
		{
			do
			{
				if ( !( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
				{
					char vpkPath[ MAX_PATH ];
					Q_snprintf( vpkPath, sizeof( vpkPath ), "%s\\%s", pathbuf, findData.cFileName );
					ALTER_DEVMSG( "found vpk file: %s\n", vpkPath );

					// double-check file exists via engine before trying to mount
					if ( g_pFullFileSystem->FileExists( vpkPath, "" ) )
					{
						ALTER_DEVMSG( "adding vpk: %s\n", vpkPath );
						g_pFullFileSystem->AddSearchPath( vpkPath, "GAME" );
					}
					else
						ALTER_WARNING( "vpk exists on FS but engine FileExists reports false: %s\n", vpkPath );
				}
			}
			while ( FindNextFileA( hFind, &findData ) );

			FindClose( hFind );
		}
		else
			ALTER_DEVMSG( "no *_dir.vpk files found in %s (Win32 search)\n", pathbuf );
#else // POSIX
		DIR *dir = opendir( pathbuf );
		if ( dir )
		{
			struct dirent *ent;
			while ( ( ent = readdir( dir ) ) != NULL )
			{
				// skip "." and ".."
				if ( ent->d_name[0] == '.' )
					continue;

				if ( EndsWith( ent->d_name, "_dir.vpk" ) )
				{
					char vpkPath[MAX_PATH];
					Q_snprintf( vpkPath, sizeof( vpkPath ), "%s/%s", pathbuf, ent->d_name );
					ALTER_DEVMSG( "found vpk file: %s\n", vpkPath );

					struct stat st;
					if ( stat( vpkPath, &st ) == 0 && S_ISREG( st.st_mode ) )
					{
						ALTER_DEVMSG( "adding vpk: %s\n", vpkPath );
						g_pFullFileSystem->AddSearchPath( vpkPath, "GAME" );
					}
					else
						ALTER_WARNING( "vpk exists entry found but stat failed or not a file: %s\n", vpkPath );
				}
			}
			closedir( dir );
		}
		else
			ALTER_WARNING( "couldn't open directory: %s\n", pathbuf );
#endif

		pSubKey = pSubKey->GetNextKey();
	}

	pKeyValues->deleteThis();
	return true;
}

///////////////
// Addons
///////////////
CUtlVector< AddonInfo_t > g_Addons;

static void NormalizeTrailingSlash( char *buf )
{
	int len = Q_strlen( buf );
	while ( len > 0 && ( buf[ len - 1 ] == '/' || buf[ len - 1 ] == '\\' ) )
	{
		buf[ len - 1 ] = '\0';
		len--;
	}
}

bool MountAddons()
{
	const char *addonRoot = "addons";

	char searchPaths[ 4096 ];
	int len = g_pFullFileSystem->GetSearchPath("MOD", false, searchPaths, sizeof(searchPaths));

	if (len == 0)
	{
		// fallback
		len = g_pFullFileSystem->GetSearchPath("GAME", false, searchPaths, sizeof(searchPaths));
	}

	if (len == 0)
	{
		ALTER_WARNING("No filesystem search paths found for MOD/GAME\n");
		return false;
	}

	if ( searchPaths[ 0 ] == '\0' )
	{
		ALTER_WARNING( "FileSystem returned no search paths for MOD/GAME\n" );
		return false;
	}

	// searchPaths is a list of absolute paths separated by ';' on Windows or ':' on POSIX
	// we'll split on both to be robust
	const char *p = searchPaths;
	while ( *p != '\0' )
	{
		// extract a single base path
		const char *start = p;
		while ( *p != '\0' && *p != ';' && *p != ':' )
			p++;
		size_t baseLen = p - start;

		// advance past separator if present
		if ( *p != '\0' )
			p++;

		if ( baseLen == 0 )
			continue;

		// copy base path into a buffer
		char basePath[ MAX_PATH ];
		Q_strncpy( basePath, start, MIN( baseLen + 1, sizeof( basePath ) ) );
		basePath[ MIN( baseLen, (size_t) ( sizeof( basePath ) - 1 ) ) ] = '\0';

		NormalizeTrailingSlash( basePath );

		// build full path to addons dir
		char addonsFull[ MAX_PATH ];
#if defined( _WIN32 )
		Q_snprintf( addonsFull, sizeof( addonsFull ), "%s\\%s", basePath, addonRoot );
#else
		Q_snprintf( addonsFull, sizeof( addonsFull ), "%s/%s", basePath, addonRoot );
#endif

		// try to open the real directory
#if defined( _WIN32 )
		WIN32_FIND_DATAA findData;
		char pattern[MAX_PATH];
		Q_snprintf( pattern, sizeof( pattern ), "%s\\*", addonsFull );

		HANDLE hFind = FindFirstFileA( pattern, &findData );
		if ( hFind == INVALID_HANDLE_VALUE )
			continue;

		do
		{
			// skip "." and ".."
			if ( strcmp( findData.cFileName, "." ) == 0 || strcmp( findData.cFileName, ".." ) == 0 )
				continue;

			// build addon folder full path
			char addonPath[ MAX_PATH ];
			Q_snprintf( addonPath, sizeof( addonPath ), "%s\\%s", addonsFull, findData.cFileName );

			if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				g_pFullFileSystem->AddSearchPath( addonPath, "GAME" );

				char vpkPattern[MAX_PATH];
				Q_snprintf( vpkPattern, sizeof( vpkPattern ), "%s\\*_dir.vpk", addonPath );

				WIN32_FIND_DATAA vpkData;
				HANDLE hVPK = FindFirstFileA( vpkPattern, &vpkData );
				if ( hVPK != INVALID_HANDLE_VALUE )
				{
					do
					{
						if ( !( vpkData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
						{
							char vpkPath[ MAX_PATH ];
							Q_snprintf( vpkPath, sizeof( vpkPath ), "%s\\%s", addonPath, vpkData.cFileName );
							if ( g_pFullFileSystem->FileExists( vpkPath, "" ) )
							{
								g_pFullFileSystem->AddSearchPath( vpkPath, "GAME" );
							}
						}
					} while ( FindNextFileA( hVPK, &vpkData ) );
					FindClose( hVPK );
				}

				// parse addoninfo.txt
				char infoPath[ MAX_PATH ];
				Q_snprintf( infoPath, sizeof( infoPath ), "%s\\addoninfo.txt", addonPath );

				if ( g_pFullFileSystem->FileExists( infoPath, "" ) )
				{
					KeyValues *pKV = new KeyValues( "AddonInfo" );
					if ( pKV->LoadFromFile( g_pFullFileSystem, infoPath, "GAME" ) )
					{
						AddonInfo_t info;
						info.name = pKV->GetString( "name", "Unknown Addon" );
						info.description = pKV->GetString( "description", "" );
						g_Addons.AddToTail( info );
					}
					pKV->deleteThis();
				}
			}
		} while ( FindNextFileA( hFind, &findData ) );

		FindClose( hFind );

#else // POSIX
		DIR *dir = opendir( addonsFull );
		if ( !dir )
			continue;

		struct dirent *ent;
		while ( ( ent = readdir( dir ) ) != NULL )
		{
			// skip entries beginning with '.'
			if ( ent->d_name[ 0 ] == '.' )
				continue;

			char addonPath[ MAX_PATH ];
			Q_snprintf( addonPath, sizeof( addonPath ), "%s/%s", addonsFull, ent->d_name );

			struct stat st;
			if ( stat( addonPath, &st ) == 0 && S_ISDIR( st.st_mode ) )
			{
				g_pFullFileSystem->AddSearchPath( addonPath, "GAME" );

				DIR *subdir = opendir( addonPath );
				if ( subdir )
				{
					struct dirent *vpkEnt;
					while ( ( vpkEnt = readdir( subdir ) ) != NULL )
					{
						if ( EndsWith( vpkEnt->d_name, "_dir.vpk" ) )
						{
							char vpkPath[ MAX_PATH ];
							Q_snprintf( vpkPath, sizeof( vpkPath ), "%s/%s", addonPath, vpkEnt->d_name );
							if ( stat( vpkPath, &st ) == 0 && S_ISREG( st.st_mode ) )
							{
								g_pFullFileSystem->AddSearchPath( vpkPath, "GAME" );
							}
						}
					}
					closedir( subdir );
				}

				// parse addoninfo.txt
				char infoPath[ MAX_PATH ];
				Q_snprintf( infoPath, sizeof( infoPath ), "%s/addoninfo.txt", addonPath );

				if ( g_pFullFileSystem->FileExists( infoPath, "" ) )
				{
					KeyValues *pKV = new KeyValues( "AddonInfo" );
					if ( pKV->LoadFromFile( g_pFullFileSystem, infoPath, "GAME" ) )
					{
						AddonInfo_t info;
						info.name = pKV->GetString( "name", "Unknown Addon" );
						info.description = pKV->GetString( "description", "" );
						g_Addons.AddToTail( info );
					}
					pKV->deleteThis();
				}
			}
		}
		closedir( dir );
#endif

	} // while over base paths

	ALTER_DEVMSG( "Mounted %d addons\n", g_Addons.Count() );
	return true;
}
