//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASEMENU_H
#define BASEMENU_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Panel.h"
#include <GameUI/IGameUI.h>
#include <vgui/VGUI.h>
#include <vgui_controls/HTML.h>
#include "steam/steamclientpublic.h"
#include "vgui/IVGui.h"

namespace vgui
{
	class Panel;
}

class CCommandHTML : public vgui::HTML
{
public:
    CCommandHTML(vgui::Panel *parent, const char *name)
     : vgui::HTML(parent, name) {}

    virtual bool OnStartRequest( const char *url, const char *target, const char *pchPostData, bool bIsRedirect ) OVERRIDE
    {
        const char *prefix = "command://";
        if ( Q_strnicmp( url, prefix, Q_strlen(prefix) ) == 0 )
        {
            CUtlString cmd( url + Q_strlen(prefix) );

            engine->ClientCmd_Unrestricted( cmd.Get() );
            return false;
        }

        return vgui::HTML::OnStartRequest( url, target, pchPostData, bIsRedirect );
    }
};
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class RootPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( RootPanel, vgui::Panel );
public:
	RootPanel(vgui::VPANEL parent);
	virtual ~RootPanel();

	IGameUI*		GetGameUI();

    // elements
    CCommandHTML *m_pHTMLPanel;

	void Refresh();

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	bool			LoadGameUI();

	int				m_ExitingFrameCount;
	bool			m_bCopyFrameBuffer;

	IGameUI*		gameui;

	bool m_bPageLoaded = false;
    bool m_bInGameLast = false;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class IOverrideInterface
{
public:
	virtual void		Create( vgui::VPANEL parent ) = 0;
	virtual vgui::VPANEL	GetPanel( void ) = 0;
	virtual void		Destroy( void ) = 0;

    virtual RootPanel *GetMenuBase() = 0;
};

extern RootPanel *guiroot;
extern IOverrideInterface *OverrideUI;

void OverrideRootUI(void);

#endif