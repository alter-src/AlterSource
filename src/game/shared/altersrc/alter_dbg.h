//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef ALTER_DBG_H
#define ALTER_DBG_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include "Color.h"
#include "tier0/dbg.h"

// cyan color: RGB(0, 255, 255)
#define ALTER_MSG(msg, ...) \
    ConColorMsg(Color(0, 255, 255, 255), "[Alter] " msg, ##__VA_ARGS__)

// cyan color: RGB(0, 255, 255)
#define ALTER_DEVMSG(msg, ...) \
    ConDColorMsg(Color(0, 255, 255, 255), "[Alter] " msg, ##__VA_ARGS__)

// orange color: RGB(255, 165, 0)
#define ALTER_WARNING(msg, ...) \
    ConColorMsg(Color(255, 165, 0, 255), "[Alter] " msg, ##__VA_ARGS__)

// orange color: RGB(255, 165, 0)
#define ALTER_DEVWARNING(msg, ...) \
    ConDColorMsg(Color(255, 165, 0, 255), "[Alter] " msg, ##__VA_ARGS__)

#endif // ALTER_DBG_H