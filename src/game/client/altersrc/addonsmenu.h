//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef ADDONSMENU_H
#define ADDONSMENU_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

class IAddonsMenu
{
public:
	virtual void		Create( vgui::VPANEL parent ) = 0;
	virtual void		Destroy( void ) = 0;
	virtual void		Activate( void ) = 0;
};

extern IAddonsMenu* g_addonsMenu;

#endif // ADDONSMENU_H