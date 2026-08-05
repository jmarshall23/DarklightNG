/*
===========================================================================

DarklightNG Source Code
Copyright (C) 2026 - Justin Marshall(aka IceColdDuke).

This file is part of the DarklightNG GPL source code.

===========================================================================
*/

#ifndef __GAME_AI_DNFBASE_H__
#define __GAME_AI_DNFBASE_H__

/*
===============================================================================

	idAI_DNFBase

	Reusable native behavior shared by AI ported from Duke Nukem Forever.
	DoomScript remains responsible for animation sequencing, while tactical
	decisions, cooldowns, flight, teleporting, cloaking and projectile launch
	are owned here and therefore survive save/restore.

===============================================================================
*/

D3_CLASS()
class idAI_DNFBase : public idAI {
public:
	CLASS_PROTOTYPE( idAI_DNFBase );

							idAI_DNFBase();

	void				Spawn( void );
	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

protected:
	virtual bool			GetDNFProjectileMuzzle( idVec3 &muzzle, idMat3 &axis );
	virtual void			PlayDNFWeaponFireAnim( void );

	bool				isCaptain;
	int					muzzleIndex;

private:
	enum dnfActionFlags_t {
		DNF_ACTION_NONE		= 0,
		DNF_ACTION_MELEE		= BIT( 3 ),
		DNF_ACTION_MISSILE	= BIT( 5 ),
		DNF_ACTION_TAKEOFF	= BIT( 6 ),
		DNF_ACTION_LAND		= BIT( 7 ),
		DNF_ACTION_TELEPORT	= BIT( 8 ),
		DNF_ACTION_CLOAK		= BIT( 9 )
	};

	bool				canFly;
	bool				canTeleport;
	bool				canCloak;
	bool				flying;
	bool				cloaked;

	float				groundSpeed;
	float				airSpeed;
	float				preferredMinRange;
	float				preferredMaxRange;
	float				maximumAttackRange;
	float				flightChance;
	float				landChance;
	float				minimumFlightTime;
	float				teleportChance;
	float				teleportMinRange;
	float				teleportMaxRange;
	float				teleportCooldown;
	float				cloakChance;
	float				cloakDuration;
	float				cloakCooldown;
	float				volleyDelay;
	float				burstInterval;
	int					burstMin;
	int					burstRandom;
	int					pendingBurstShots;

	int					nextDecisionTime;
	int					nextAttackTime;
	int					nextFlightDecisionTime;
	int					minimumLandTime;
	int					nextTeleportTime;
	int					nextCloakTime;
	int					cloakEndTime;

	void				SetDNFMoveType( moveType_t moveType, float speed );
	void				SetCloaked( bool enable );
	void				ApplyCloakVisuals( void );
	bool				FindTeleportDestination( idVec3 &destination ) const;

	D3_EVENT( AI_DNFBase_ChooseAction, "dnfChooseAction", integer )
	void				Event_ChooseAction( void );
	D3_EVENT( AI_DNFBase_GetBurstCount, "dnfGetBurstCount", integer )
	void				Event_GetBurstCount( void );
	D3_EVENT( AI_DNFBase_GetBurstInterval, "dnfGetBurstInterval", float )
	void				Event_GetBurstInterval( void );
	D3_EVENT( AI_DNFBase_FireProjectile, "dnfFireProjectile", void )
	void				Event_FireProjectile( void );
	D3_EVENT( AI_DNFBase_MeleeAttack, "dnfMeleeAttack", void )
	void				Event_MeleeAttack( void );
	D3_EVENT( AI_DNFBase_BeginFlight, "dnfBeginFlight", void )
	void				Event_BeginFlight( void );
	D3_EVENT( AI_DNFBase_EndFlight, "dnfEndFlight", void )
	void				Event_EndFlight( void );
	D3_EVENT( AI_DNFBase_IsFlying, "dnfIsFlying", integer )
	void				Event_IsFlying( void );
	D3_EVENT( AI_DNFBase_Teleport, "dnfTeleport", integer )
	void				Event_Teleport( void );
	D3_EVENT( AI_DNFBase_SetCloaked, "dnfSetCloaked", void )
	void				Event_SetCloaked( int enable );
	D3_EVENT( AI_DNFBase_IsCloaked, "dnfIsCloaked", integer )
	void				Event_IsCloaked( void );
};

#endif /* !__GAME_AI_DNFBASE_H__ */
