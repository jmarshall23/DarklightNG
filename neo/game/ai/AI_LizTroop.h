/*
===========================================================================

DarklightNG Source Code
Copyright (C) 2026 - Justin Marshall(aka IceColdDuke).

This file is part of the DarklightNG GPL source code.

===========================================================================
*/

#ifndef __GAME_AI_LIZTROOP_H__
#define __GAME_AI_LIZTROOP_H__

#include "AI_DNFBase.h"

D3_CLASS()
class idAI_LizTroop : public idAI_DNFBase {
public:
	CLASS_PROTOTYPE( idAI_LizTroop );

protected:
	virtual bool			GetDNFProjectileMuzzle( idVec3 &muzzle, idMat3 &axis );
	virtual void			PlayDNFWeaponFireAnim( void );
};

#endif /* !__GAME_AI_LIZTROOP_H__ */
