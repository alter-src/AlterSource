//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef MOUNTLOGIC_H
#define MOUNTLOGIC_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include "tier1/utlvector.h"

struct AddonInfo_t
{
    CUtlString name;
    CUtlString description;
};

bool LoadGameMounts();
bool MountAddons();

extern CUtlVector<AddonInfo_t> g_Addons;

#endif // MOUNTLOGIC_H