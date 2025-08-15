//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basemenu.h"

#include "vgui/ILocalize.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"
#include "ienginevgui.h"
#include <engine/IEngineSound.h>
#include "filesystem.h"
#include "vgui_int.h"
#include "fmtstr.h"
#ifdef _WIN32
#pragma warning( disable : 4005 )
#include <windows.h>
#undef CreateFont
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static CDllDemandLoader g_GameUIDLL( "GameUI" );

RootPanel *guiroot = NULL;

void OverrideRootUI(void)
{
	if( !OverrideUI->GetPanel() )
	{
		OverrideUI->Create(NULL);
	}
	if( guiroot->GetGameUI() )
	{
		guiroot->GetGameUI()->SetMainMenuOverride( guiroot->GetVPanel() );
		return;
	}
}

RootPanel::RootPanel(VPANEL parent) : Panel( NULL, "RootPanel" )
{
	SetParent(parent);
	guiroot = this;

	m_bCopyFrameBuffer = false;
	gameui = NULL;

	LoadGameUI();

	m_ExitingFrameCount = 0;

    m_pHTMLPanel = new CCommandHTML(this, "HTMLPanel");
    m_pHTMLPanel->SetScrollbarsEnabled(false);
	m_pHTMLPanel->SetContextMenuEnabled(false);
	m_pHTMLPanel->SetViewSourceEnabled(false);
}

IGameUI *RootPanel::GetGameUI()
{
	if( !gameui )
	{
		if ( !LoadGameUI() )
			return NULL;
	}

	return gameui;
}

bool RootPanel::LoadGameUI()
{
	if( !gameui )
	{
		CreateInterfaceFn gameUIFactory = g_GameUIDLL.GetFactory();
		if ( gameUIFactory )
		{
			gameui = (IGameUI *) gameUIFactory(GAMEUI_INTERFACE_VERSION, NULL);
			if( !gameui )
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

RootPanel::~RootPanel()
{
	gameui = NULL;
	g_GameUIDLL.Unload();
}

void RootPanel::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    int wide, tall;
    vgui::surface()->GetScreenSize(wide, tall);
    SetSize(wide, tall);
    m_pHTMLPanel->SetBounds(0, 0, wide, tall);

    if (!m_bPageLoaded)
    {
        const char *gameDir = engine->GetGameDirectory();
        char filePath[ MAX_PATH ];
        bool bInGame = engine->IsInGame();
        sprintf(filePath, "file://%s/html/menu.html?ingame=%d", gameDir, bInGame ? 1 : 0);

        m_pHTMLPanel->OpenURL(filePath, nullptr, true);
        m_bPageLoaded = true;
    }
}

void RootPanel::Refresh()
{
    bool bInGame = engine->IsInGame();
    m_pHTMLPanel->RunJavascript(CFmtStr("OnInGameResult(%d);", bInGame ? 1 : 0));
}

class COverrideInterface : public IOverrideInterface
{
private:
	RootPanel *MainMenu;
 
public:
	int UI_Interface( void )
	{
		MainMenu = NULL;
		return 0;
	}
 
	void Create( vgui::VPANEL parent )
	{
		MainMenu = new RootPanel(parent);
	}

	vgui::VPANEL GetPanel( void )
	{
		if ( !MainMenu )
			return NULL;
		return MainMenu->GetVPanel();
	}
 
	void Destroy( void )
	{
		if ( MainMenu )
		{
			MainMenu->SetParent( (vgui::Panel *)NULL );
			delete MainMenu;
		}
	}
 
    virtual RootPanel* GetMenuBase() override
    {
        return MainMenu;
    }
};
 
static COverrideInterface g_SMenu;
IOverrideInterface *OverrideUI = ( IOverrideInterface * )&g_SMenu;

// commands
void openURL(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		Msg("usage: open <URL>\n");
		return;
	}

	const char* url = args.Arg(1);
	steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage(url);
}

CON_COMMAND(open, "haha open url go brr")
{
	openURL(args);
}