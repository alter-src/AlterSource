//========= Copyright Alter Source, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef TGAVTF_UTILS_H
#define TGAVTF_UTILS_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

bool ConvertTGAtoVTF( const char *pSourceTGA, const char *pDestVTF, int pVTFFlags, bool pRemoveSourceTGA = false );

#endif // TGAVTF_UTILS_H