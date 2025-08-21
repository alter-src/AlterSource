//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "bitmap/tgaloader.h"
#include "vtf/vtf.h"
#include "tier1/utlbuffer.h"
#include "tier1/convar.h"
#include "alter_dbg.h"

// Converts the source TGA to the destination VTF on disk. Returns true on success.
// VTF flags are members of the enum CompiledVtfFlags in vtf.h.
// Pass the optional argument as true to remove the source TGA after a successful conversion.
bool ConvertTGAtoVTF( const char *pSourceTGA, const char *pDestVTF, int pVTFFlags, bool pRemoveSourceTGA )
{
	enum ImageFormat indexImageFormat;
	int indexImageSize;
	float gamma;
	int w, h;

	if ( !TGALoader::GetInfo( pSourceTGA, &w, &h, &indexImageFormat, &gamma ) )
	{
		ALTER_WARNING( "Unable to find TGA: %s\n", pSourceTGA );
		return false;
	}

	indexImageSize = ImageLoader::GetMemRequired( w, h, 1, indexImageFormat, false );

	unsigned char *pImage = ( unsigned char* )new unsigned char[indexImageSize];

	if ( !TGALoader::Load( pImage, pSourceTGA, w, h, indexImageFormat, gamma, false ) )
	{
		ALTER_WARNING( "Unable to load TGA: %s\n", pSourceTGA );
		return false;
	}

	// Create a new VTF
	IVTFTexture *vtf = CreateVTFTexture();

	// Initialise it with some flags (nomipmap is good for using in the VGUI)
	vtf->Init( w, h, 1, indexImageFormat, pVTFFlags, 1 );
	unsigned char* pDest = vtf->ImageData( 0, 0, 0 );

	// Memcpy it to the VTF
	memcpy( pDest, pImage, indexImageSize );

	// Write
	CUtlBuffer buffer;
	vtf->Serialize( buffer );
	bool vtfWriteSuccess = g_pFullFileSystem->WriteFile( pDestVTF, "MOD", buffer );

	// Clean up
	DestroyVTFTexture( vtf );
	buffer.Clear();

	if ( !vtfWriteSuccess )
	{
		ALTER_WARNING( "Unable to write VTF: %s\n", pDestVTF );
		return false;
	}

	// Remove the original
	if ( pRemoveSourceTGA )
		g_pFullFileSystem->RemoveFile( pSourceTGA, "MOD" );

	return true;
}

CON_COMMAND( tga2vtf, "Convert a TGA file to a VTF file." )
{
    if ( args.ArgC() < 3 )
    {
        ALTER_MSG( "Usage: tga2vtf <input.tga> <output.vtf> [flags] [removeSource]\n" );
        return;
    }

    const char* pSourceTGA = args[ 1 ];
    const char* pDestVTF   = args[ 2 ];

    int pVTFFlags = 0;
    bool pRemoveSourceTGA = false;

    if ( args.ArgC() >= 4 )
        pVTFFlags = atoi( args[ 3 ] ); // optional flags

    if ( args.ArgC() >= 5 )
        pRemoveSourceTGA = atoi( args[ 4 ] ) != 0; // treat nonzero as true

    if ( ConvertTGAtoVTF(pSourceTGA, pDestVTF, pVTFFlags, pRemoveSourceTGA))
        ALTER_MSG( "Successfully converted %s -> %s\n", pSourceTGA, pDestVTF );
    else
        ALTER_WARNING( "Failed to convert %s -> %s\n", pSourceTGA, pDestVTF );
}
