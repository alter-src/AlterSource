//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "addonsmenu.h"
#include "mountlogic.h"

#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>

using namespace vgui;

ConVar cl_showaddonsmenu("cl_showaddonsmenu", "0", FCVAR_CLIENTDLL, "Sets the state of the Addons Menu <state>");

class CAddonListPanel : public Panel
{
    DECLARE_CLASS_SIMPLE(CAddonListPanel, Panel);

public:
    CAddonListPanel(vgui::VPANEL parent);
    ~CAddonListPanel();

    virtual void PerformLayout() override;

private:
    ListPanel *m_pAddonList;
    void PopulateAddonList();
};

CAddonListPanel::CAddonListPanel(vgui::VPANEL parent)
    : BaseClass(NULL, "AddonListPanel")
{
    SetParent(parent);
    SetProportional(true);
    SetVisible(true);

    SetScheme(scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

    m_pAddonList = new ListPanel(this, "AddonList");
    m_pAddonList->AddColumnHeader(0, "name", "Name", 150, 0);
    m_pAddonList->AddColumnHeader(1, "description", "Description", 230, 0);

    PopulateAddonList();
}

CAddonListPanel::~CAddonListPanel()
{
}

void CAddonListPanel::PerformLayout()
{
    BaseClass::PerformLayout();
    if (m_pAddonList)
    {
        m_pAddonList->SetBounds(10, 30, GetWide() - 20, GetTall() - 40); // leave some space for title bar
    }
}

void CAddonListPanel::PopulateAddonList()
{
    if (!m_pAddonList)
        return;

    m_pAddonList->DeleteAllItems();

    for (int i = 0; i < g_Addons.Count(); i++)
    {
        KeyValues *kv = new KeyValues("AddonEntry");
        kv->SetString("name", g_Addons[i].name.Get());
        kv->SetString("description", g_Addons[i].description.Get());

        m_pAddonList->AddItem(kv, 0, false, true);
        kv->deleteThis();
    }
}

class CAddonsMenu : public Frame
{
    DECLARE_CLASS_SIMPLE(CAddonsMenu, Frame);

public:
    CAddonsMenu(vgui::VPANEL parent);
    virtual ~CAddonsMenu() {};

protected:
    virtual void OnTick();
    virtual void OnCommand(const char* pcCommand) override;

private:
    CAddonListPanel *m_pAddonListPanel;
};

CAddonsMenu::CAddonsMenu(vgui::VPANEL parent)
    : BaseClass(NULL, "AddonsMenu")
{
    SetParent(parent);

    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);

    SetProportional(false);
    SetTitleBarVisible(true);
    SetMinimizeButtonVisible(false);
    SetMaximizeButtonVisible(false);
    SetCloseButtonVisible(true);
    SetSizeable(false);
    SetMoveable(false);
    SetVisible(true);

    SetScheme(scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));
	LoadControlSettings("resource/ui/addonsmenu.res");

    vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

    // the addons list
    m_pAddonListPanel = new CAddonListPanel(GetVPanel());
    m_pAddonListPanel->SetBounds(10, 30, 600, 400);
}

void CAddonsMenu::OnTick()
{
    BaseClass::OnTick();
    SetVisible(cl_showaddonsmenu.GetBool());
}

void CAddonsMenu::OnCommand(const char* pcCommand)
{
    BaseClass::OnCommand(pcCommand);
}

class CAddonsMenuInterface : public IAddonsMenu
{
private:
    CAddonsMenu *AddonsMenu;
public:
    CAddonsMenuInterface() : AddonsMenu(nullptr) {}

    void Create(vgui::VPANEL parent)
    {
        AddonsMenu = new CAddonsMenu(parent);
    }
    void Destroy()
    {
        if (AddonsMenu)
        {
            AddonsMenu->SetParent(nullptr);
            delete AddonsMenu;
        }
    }
    void Activate()
    {
        if (AddonsMenu)
        {
			AddonsMenu->Activate();
			AddonsMenu->RequestFocus();
		}
    }
};

static CAddonsMenuInterface g_AddonsMenu;
IAddonsMenu* g_addonsMenu = (IAddonsMenu*)&g_AddonsMenu;

CON_COMMAND(OpenAddonsMenu, "Toggles the addons menu on or off")
{
	// TODO: fix this shit because im too lazy and the
	// august 21, 2025 patch needs to be released today
    cl_showaddonsmenu.SetValue(!cl_showaddonsmenu.GetBool());
    g_addonsMenu->Activate();
};
