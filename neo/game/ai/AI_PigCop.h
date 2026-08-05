/*
===========================================================================

DarklightNG Source Code
Copyright (C) 2026 - Justin Marshall(aka IceColdDuke).

This file is part of the DarklightNG GPL source code.

===========================================================================
*/

#ifndef __GAME_AI_PIGCOP_H__
#define __GAME_AI_PIGCOP_H__

/*
===============================================================================

	idAI_PigCop

	Native combat behavior for the DNF PigCop.  DoomScript owns animation
	sequencing; this class owns weapon-specific firing, melee damage and the
	one-time low-health berserk decision.

===============================================================================
*/

D3_CLASS()
class idAI_PigCop : public idAI {
public:
	CLASS_PROTOTYPE( idAI_PigCop );

							idAI_PigCop();

	void				Spawn( void );
	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

private:
	enum pigActionFlags_t {
		PIG_ACTION_NONE		= 0,
		PIG_ACTION_MELEE		= BIT( 3 ),
		PIG_ACTION_MISSILE	= BIT( 5 ),
		PIG_ACTION_BERSERK	= BIT( 6 )
	};

	bool				berserk;
	bool				berserkRollMade;
	bool				shotgunVariant;
	int					initialHealth;
	int					muzzleIndex;
	int					pendingBurstShots;
	int					nextDecisionTime;
	int					nextAttackTime;

	float				normalSpeed;
	float				berserkSpeed;
	float				maximumAttackRange;
	float				berserkHealthFraction;
	float				berserkChance;
	float				volleyDelay;
	float				burstInterval;
	int					burstMin;
	int					burstRandom;
	int					shotgunPellets;
	float				shotgunSpread;

	void				SetPigMoveSpeed( float speed );
	void				ApplyBerserkVisuals( void );
	bool				GetWeaponMuzzle( int side, idVec3 &muzzle, idMat3 &axis );
	void				PlayWeaponFireAnim( int side );

	D3_EVENT( AI_PigCop_ChooseAction, "pigChooseAction", integer )
	void				Event_ChooseAction( void );
	D3_EVENT( AI_PigCop_GetBurstCount, "pigGetBurstCount", integer )
	void				Event_GetBurstCount( void );
	D3_EVENT( AI_PigCop_GetBurstInterval, "pigGetBurstInterval", float )
	void				Event_GetBurstInterval( void );
	D3_EVENT( AI_PigCop_GetMuzzleSide, "pigGetMuzzleSide", integer )
	void				Event_GetMuzzleSide( void );
	D3_EVENT( AI_PigCop_FireProjectile, "pigFireProjectile", void )
	void				Event_FireProjectile( void );
	D3_EVENT( AI_PigCop_MeleeAttack, "pigMeleeAttack", void )
	void				Event_MeleeAttack( void );
	D3_EVENT( AI_PigCop_BeginBerserk, "pigBeginBerserk", void )
	void				Event_BeginBerserk( void );
	D3_EVENT( AI_PigCop_IsBerserk, "pigIsBerserk", integer )
	void				Event_IsBerserk( void );
};

#endif /* !__GAME_AI_PIGCOP_H__ */
