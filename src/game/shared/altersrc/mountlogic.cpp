//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "mountlogic.h"
#include <filesystem.h>
#include <tier1/KeyValues.h>

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
		Warning( "mount.txt not found\n" );
		pKeyValues->deleteThis();
		return false;
	}

	KeyValues *pSubKey = pKeyValues->GetFirstSubKey();
	if ( !pSubKey )
	{
		Warning( "no mounts found in mount.txt\n" );
		pKeyValues->deleteThis();
		return false;
	}

	while ( pSubKey )
	{
		const char *path = pSubKey->GetString();
		if ( !path || !path[0] )
		{
			Warning( "empty mount path in mount.txt\n" );
			pSubKey = pSubKey->GetNextKey();
			continue;
		}

		DevMsg( "mounting path: %s\n", path );

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
					DevMsg( "found vpk file: %s\n", vpkPath );

					// double-check file exists via engine before trying to mount
					if ( g_pFullFileSystem->FileExists( vpkPath, "" ) )
					{
						DevMsg( "adding vpk: %s\n", vpkPath );
						g_pFullFileSystem->AddSearchPath( vpkPath, "GAME" );
					}
					else
						Warning( "vpk exists on FS but engine FileExists reports false: %s\n", vpkPath );
				}
			}
			while ( FindNextFileA( hFind, &findData ) );

			FindClose( hFind );
		}
		else
			DevMsg( "no *_dir.vpk files found in %s (Win32 search)\n", pathbuf );
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
					DevMsg( "found vpk file: %s\n", vpkPath );

					struct stat st;
					if ( stat( vpkPath, &st ) == 0 && S_ISREG( st.st_mode ) )
					{
						DevMsg( "adding vpk: %s\n", vpkPath );
						g_pFullFileSystem->AddSearchPath( vpkPath, "GAME" );
					}
					else
						Warning( "vpk exists entry found but stat failed or not a file: %s\n", vpkPath );
				}
			}
			closedir( dir );
		}
		else
			Warning( "couldn't open directory: %s\n", pathbuf );
#endif

		pSubKey = pSubKey->GetNextKey();
	}

	pKeyValues->deleteThis();
	return true;
}
