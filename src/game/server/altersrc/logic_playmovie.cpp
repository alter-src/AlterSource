//===== Copyright � 1996-2009, Valve Corporation, All rights reserved. ======//
//
//  Purpose: Plays a movie and reports on finish
//
//===========================================================================//

#include "cbase.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CLogicPlayMovie : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicPlayMovie, CLogicalEntity );
	DECLARE_DATADESC();

	CLogicPlayMovie( void ) { }
	~CLogicPlayMovie( void ) { }

	virtual void Precache( void );
	virtual void Spawn( void );
	
private:

	void		InputPlayMovie( inputdata_t &data );
#ifdef AS_DLL
	void		InputStopMovie( inputdata_t &data );
#endif // AS_DLL
	void		InputMovieFinished( inputdata_t &data );

	string_t	m_strMovieFilename;
	bool		m_bAllowUserSkip;
#ifdef AS_DLL
	bool		m_bLooping;
	bool		m_bMuted;

	bool		m_bPlayingVideo;
#endif // AS_DLL

	COutputEvent	m_OnPlaybackFinished;
};

LINK_ENTITY_TO_CLASS( logic_playmovie, CLogicPlayMovie );

BEGIN_DATADESC( CLogicPlayMovie )

	DEFINE_KEYFIELD( m_strMovieFilename, FIELD_STRING, "MovieFilename" ),
	DEFINE_KEYFIELD( m_bAllowUserSkip, FIELD_BOOLEAN, "allowskip" ),
#ifdef AS_DLL
	DEFINE_KEYFIELD( m_bLooping, FIELD_BOOLEAN, "loopvideo" ),
	DEFINE_KEYFIELD( m_bMuted, FIELD_BOOLEAN, "mute" ),

	DEFINE_FIELD( m_bPlayingVideo, FIELD_BOOLEAN ),
#endif // AS_DLL

	DEFINE_INPUTFUNC( FIELD_VOID, "PlayMovie", InputPlayMovie ),
#ifdef AS_DLL
	DEFINE_INPUTFUNC( FIELD_VOID, "StopMovie", InputStopMovie ),
#endif // AS_DLL
	DEFINE_INPUTFUNC( FIELD_VOID, "__MovieFinished", InputMovieFinished ),

	DEFINE_OUTPUT( m_OnPlaybackFinished, "OnPlaybackFinished" ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicPlayMovie::Precache( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicPlayMovie::Spawn( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicPlayMovie::InputPlayMovie( inputdata_t &data )
{
	// Build the hacked string
	
	char szClientCmd[256];
	Q_snprintf( szClientCmd, sizeof(szClientCmd), 
				"playvideo_complex %s \"ent_fire %s __MovieFinished\" %d %d %d\n", 
				STRING(m_strMovieFilename), 
				GetEntityNameAsCStr(),
				m_bAllowUserSkip,
#ifdef AS_DLL
				m_bLooping,
				m_bMuted
#else
				0,
				0
#endif // AS_DLL
				 );

	// Send it on
	engine->ServerCommand( szClientCmd );

#ifdef AS_DLL
	m_bPlayingVideo = true;
#endif // AS_DLL
}

#ifdef AS_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicPlayMovie::InputStopMovie( inputdata_t &data )
{
	if (m_bPlayingVideo)
	{
		// Send it on
		engine->ServerCommand( "stopvideos\n" );
	}
}
#endif // AS_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLogicPlayMovie::InputMovieFinished( inputdata_t &data )
{
	// Simply fire our output
	m_OnPlaybackFinished.FireOutput( this, this );

#ifdef AS_DLL
	m_bPlayingVideo = false;
#endif // AS_DLL
}
