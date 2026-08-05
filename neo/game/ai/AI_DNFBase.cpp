/*
===========================================================================

DarklightNG Source Code
Copyright (C) 2026 - Justin Marshall(aka IceColdDuke).

This file is part of the DarklightNG GPL source code.

===========================================================================
*/

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "AI_DNFBase.h"

static const int DNF_DECISION_INTERVAL_MS = 200;
static const int DNF_FLIGHT_RECHECK_MS = 1000;

idAI_DNFBase::idAI_DNFBase() {
	isCaptain = false;
	muzzleIndex = 0;
	canFly = false;
	canTeleport = false;
	canCloak = false;
	flying = false;
	cloaked = false;
	groundSpeed = 280.0f;
	airSpeed = 500.0f;
	preferredMinRange = 200.0f;
	preferredMaxRange = 700.0f;
	maximumAttackRange = 1500.0f;
	flightChance = 0.8f;
	landChance = 0.5f;
	minimumFlightTime = 1.0f;
	teleportChance = 0.2f;
	teleportMinRange = 250.0f;
	teleportMaxRange = 1200.0f;
	teleportCooldown = 5.0f;
	cloakChance = 0.2f;
	cloakDuration = 3.0f;
	cloakCooldown = 8.0f;
	volleyDelay = 1.2f;
	burstInterval = 0.1f;
	burstMin = 3;
	burstRandom = 1;
	pendingBurstShots = 0;
	nextDecisionTime = 0;
	nextAttackTime = 0;
	nextFlightDecisionTime = 0;
	minimumLandTime = 0;
	nextTeleportTime = 0;
	nextCloakTime = 0;
	cloakEndTime = 0;
}

void idAI_DNFBase::Spawn( void ) {
	isCaptain = spawnArgs.GetBool( "dnf_captain", "0" );
	canFly = spawnArgs.GetBool( "dnf_can_fly", "1" );
	canTeleport = spawnArgs.GetBool( "dnf_can_teleport", "0" );
	canCloak = spawnArgs.GetBool( "dnf_can_cloak", "0" );

	groundSpeed = spawnArgs.GetFloat( "dnf_ground_speed", "280" );
	airSpeed = spawnArgs.GetFloat( "dnf_air_speed", "500" );
	preferredMinRange = spawnArgs.GetFloat( "dnf_preferred_min_range", "200" );
	preferredMaxRange = spawnArgs.GetFloat( "dnf_preferred_max_range", "700" );
	maximumAttackRange = spawnArgs.GetFloat( "dnf_max_attack_range", "1500" );
	flightChance = spawnArgs.GetFloat( "dnf_flight_chance", "0.8" );
	landChance = spawnArgs.GetFloat( "dnf_land_chance", "0.5" );
	minimumFlightTime = spawnArgs.GetFloat( "dnf_min_flight_time", "1" );
	teleportChance = spawnArgs.GetFloat( "dnf_teleport_chance", "0.2" );
	teleportMinRange = spawnArgs.GetFloat( "dnf_teleport_min_range", "250" );
	teleportMaxRange = spawnArgs.GetFloat( "dnf_teleport_max_range", "1200" );
	teleportCooldown = spawnArgs.GetFloat( "dnf_teleport_cooldown", "5" );
	cloakChance = spawnArgs.GetFloat( "dnf_cloak_chance", "0.2" );
	cloakDuration = spawnArgs.GetFloat( "dnf_cloak_duration", "3" );
	cloakCooldown = spawnArgs.GetFloat( "dnf_cloak_cooldown", "8" );
	volleyDelay = spawnArgs.GetFloat( "dnf_volley_delay", "1.2" );
	burstInterval = spawnArgs.GetFloat( "dnf_burst_interval", "0.1" );
	burstMin = spawnArgs.GetInt( "dnf_burst_min", "3" );
	burstRandom = spawnArgs.GetInt( "dnf_burst_random", "1" );

	flying = false;
	cloaked = false;
	pendingBurstShots = 0;
	muzzleIndex = 0;
	nextDecisionTime = gameLocal.time;
	nextAttackTime = gameLocal.time;
	nextFlightDecisionTime = gameLocal.time + DNF_FLIGHT_RECHECK_MS;
	minimumLandTime = 0;
	nextTeleportTime = gameLocal.time + SEC2MS( teleportCooldown );
	nextCloakTime = gameLocal.time + SEC2MS( cloakCooldown );
	cloakEndTime = 0;

	SetDNFMoveType( MOVETYPE_SLIDE, groundSpeed );
}

void idAI_DNFBase::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( isCaptain );
	savefile->WriteInt( muzzleIndex );
	savefile->WriteBool( canFly );
	savefile->WriteBool( canTeleport );
	savefile->WriteBool( canCloak );
	savefile->WriteBool( flying );
	savefile->WriteBool( cloaked );
	savefile->WriteFloat( groundSpeed );
	savefile->WriteFloat( airSpeed );
	savefile->WriteFloat( preferredMinRange );
	savefile->WriteFloat( preferredMaxRange );
	savefile->WriteFloat( maximumAttackRange );
	savefile->WriteFloat( flightChance );
	savefile->WriteFloat( landChance );
	savefile->WriteFloat( minimumFlightTime );
	savefile->WriteFloat( teleportChance );
	savefile->WriteFloat( teleportMinRange );
	savefile->WriteFloat( teleportMaxRange );
	savefile->WriteFloat( teleportCooldown );
	savefile->WriteFloat( cloakChance );
	savefile->WriteFloat( cloakDuration );
	savefile->WriteFloat( cloakCooldown );
	savefile->WriteFloat( volleyDelay );
	savefile->WriteFloat( burstInterval );
	savefile->WriteInt( burstMin );
	savefile->WriteInt( burstRandom );
	savefile->WriteInt( pendingBurstShots );
	savefile->WriteInt( nextDecisionTime );
	savefile->WriteInt( nextAttackTime );
	savefile->WriteInt( nextFlightDecisionTime );
	savefile->WriteInt( minimumLandTime );
	savefile->WriteInt( nextTeleportTime );
	savefile->WriteInt( nextCloakTime );
	savefile->WriteInt( cloakEndTime );
}

void idAI_DNFBase::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( isCaptain );
	savefile->ReadInt( muzzleIndex );
	savefile->ReadBool( canFly );
	savefile->ReadBool( canTeleport );
	savefile->ReadBool( canCloak );
	savefile->ReadBool( flying );
	savefile->ReadBool( cloaked );
	savefile->ReadFloat( groundSpeed );
	savefile->ReadFloat( airSpeed );
	savefile->ReadFloat( preferredMinRange );
	savefile->ReadFloat( preferredMaxRange );
	savefile->ReadFloat( maximumAttackRange );
	savefile->ReadFloat( flightChance );
	savefile->ReadFloat( landChance );
	savefile->ReadFloat( minimumFlightTime );
	savefile->ReadFloat( teleportChance );
	savefile->ReadFloat( teleportMinRange );
	savefile->ReadFloat( teleportMaxRange );
	savefile->ReadFloat( teleportCooldown );
	savefile->ReadFloat( cloakChance );
	savefile->ReadFloat( cloakDuration );
	savefile->ReadFloat( cloakCooldown );
	savefile->ReadFloat( volleyDelay );
	savefile->ReadFloat( burstInterval );
	savefile->ReadInt( burstMin );
	savefile->ReadInt( burstRandom );
	savefile->ReadInt( pendingBurstShots );
	savefile->ReadInt( nextDecisionTime );
	savefile->ReadInt( nextAttackTime );
	savefile->ReadInt( nextFlightDecisionTime );
	savefile->ReadInt( minimumLandTime );
	savefile->ReadInt( nextTeleportTime );
	savefile->ReadInt( nextCloakTime );
	savefile->ReadInt( cloakEndTime );
	ApplyCloakVisuals();
}

void idAI_DNFBase::SetDNFMoveType( moveType_t moveType, float speed ) {
	const float oldSpeed = fly_speed;
	move.moveType = moveType;
	travelFlags = ( moveType == MOVETYPE_FLY ) ? ( TFL_WALK | TFL_AIR | TFL_FLY ) : ( TFL_WALK | TFL_AIR );
	if ( move.speed == oldSpeed ) {
		move.speed = speed;
	}
	fly_speed = speed;
}

void idAI_DNFBase::Event_ChooseAction( void ) {
	idActor *enemyEnt = enemy.GetEntity();
	if ( !enemyEnt || health <= 0 || gameLocal.time < nextDecisionTime ) {
		idThread::ReturnInt( DNF_ACTION_NONE );
		return;
	}

	nextDecisionTime = gameLocal.time + DNF_DECISION_INTERVAL_MS;

	if ( cloaked ) {
		idThread::ReturnInt( gameLocal.time >= cloakEndTime ? DNF_ACTION_CLOAK : DNF_ACTION_NONE );
		return;
	}

	const idVec3 enemyDelta = enemyEnt->GetPhysics()->GetOrigin() - physicsObj.GetOrigin();
	const float enemyRange = enemyDelta.LengthFast();

	if ( TestMelee() && gameLocal.time >= nextAttackTime ) {
		nextAttackTime = gameLocal.time + SEC2MS( volleyDelay );
		idThread::ReturnInt( DNF_ACTION_MELEE );
		return;
	}

	if ( !flying && canTeleport && gameLocal.time >= nextTeleportTime &&
		enemyRange >= teleportMinRange && enemyRange <= teleportMaxRange &&
		gameLocal.random.RandomFloat() < teleportChance ) {
		nextTeleportTime = gameLocal.time + SEC2MS( teleportCooldown );
		idThread::ReturnInt( DNF_ACTION_TELEPORT );
		return;
	}

	if ( canCloak && gameLocal.time >= nextCloakTime &&
		gameLocal.random.RandomFloat() < cloakChance ) {
		nextCloakTime = gameLocal.time + SEC2MS( cloakCooldown );
		idThread::ReturnInt( DNF_ACTION_CLOAK );
		return;
	}

	if ( flying ) {
		if ( gameLocal.time >= minimumLandTime && gameLocal.time >= nextFlightDecisionTime ) {
			nextFlightDecisionTime = gameLocal.time + DNF_FLIGHT_RECHECK_MS;
			if ( gameLocal.random.RandomFloat() < landChance ) {
				idThread::ReturnInt( DNF_ACTION_LAND );
				return;
			}
		}
	} else if ( canFly && gameLocal.time >= nextFlightDecisionTime &&
		enemyRange >= preferredMinRange && enemyRange <= maximumAttackRange ) {
		nextFlightDecisionTime = gameLocal.time + DNF_FLIGHT_RECHECK_MS;
		if ( gameLocal.random.RandomFloat() < flightChance ) {
			idThread::ReturnInt( DNF_ACTION_TAKEOFF );
			return;
		}
	}

	if ( AI_ENEMY_VISIBLE && gameLocal.time >= nextAttackTime && enemyRange <= maximumAttackRange ) {
		pendingBurstShots = burstMin;
		if ( burstRandom > 0 ) {
			pendingBurstShots += gameLocal.random.RandomInt( burstRandom + 1 );
		}
		nextAttackTime = gameLocal.time + SEC2MS( volleyDelay + gameLocal.random.RandomFloat() * 0.3f );
		idThread::ReturnInt( DNF_ACTION_MISSILE );
		return;
	}

	idThread::ReturnInt( DNF_ACTION_NONE );
}

void idAI_DNFBase::Event_GetBurstCount( void ) {
	const int count = Max( 1, pendingBurstShots );
	pendingBurstShots = 0;
	idThread::ReturnInt( count );
}

void idAI_DNFBase::Event_GetBurstInterval( void ) {
	idThread::ReturnFloat( burstInterval );
}

bool idAI_DNFBase::GetDNFProjectileMuzzle( idVec3 &muzzle, idMat3 &axis ) {
	const char *jointName = spawnArgs.GetString( "dnf_muzzle_joint", "mount_carryitem" );
	const jointHandle_t joint = animator.GetJointHandle( jointName );
	if ( joint == INVALID_JOINT || !GetJointWorldTransform( joint, gameLocal.time, muzzle, axis ) ) {
		muzzle = GetEyePosition();
		axis = viewAxis;
		return false;
	}

	const idVec3 offset = spawnArgs.GetVector( "dnf_muzzle_offset", "18 0 4" );
	muzzle += offset * axis;
	return true;
}

void idAI_DNFBase::PlayDNFWeaponFireAnim( void ) {
}

void idAI_DNFBase::Event_FireProjectile( void ) {
	idActor *enemyEnt = enemy.GetEntity();
	if ( !enemyEnt || !projectileDef ) {
		return;
	}

	if ( cloaked ) {
		SetCloaked( false );
	}

	idVec3 muzzle;
	idMat3 muzzleAxis;
	GetDNFProjectileMuzzle( muzzle, muzzleAxis );

	idVec3 aimDir;
	if ( !GetAimDir( muzzle, enemyEnt, this, aimDir ) ) {
		aimDir = muzzleAxis[ 0 ];
	}

	PlayDNFWeaponFireAnim();
	Event_LaunchMissile( muzzle, aimDir.ToAngles() );
	++muzzleIndex;
}

void idAI_DNFBase::Event_MeleeAttack( void ) {
	if ( cloaked ) {
		SetCloaked( false );
	}
	AttackMelee( spawnArgs.GetString( "def_melee", "melee_liztroop" ) );
}

void idAI_DNFBase::Event_BeginFlight( void ) {
	if ( flying || !canFly ) {
		return;
	}

	flying = true;
	minimumLandTime = gameLocal.time + SEC2MS( minimumFlightTime );
	nextFlightDecisionTime = minimumLandTime;
	SetDNFMoveType( MOVETYPE_FLY, airSpeed );
	MoveToEnemy();
}

void idAI_DNFBase::Event_EndFlight( void ) {
	if ( !flying ) {
		return;
	}

	flying = false;
	nextFlightDecisionTime = gameLocal.time + DNF_FLIGHT_RECHECK_MS;
	SetDNFMoveType( MOVETYPE_SLIDE, groundSpeed );
	physicsObj.SetLinearVelocity( vec3_origin );
	StopMove( MOVE_STATUS_DONE );
}

void idAI_DNFBase::Event_IsFlying( void ) {
	idThread::ReturnInt( flying );
}

bool idAI_DNFBase::FindTeleportDestination( idVec3 &destination ) const {
	idActor *enemyEnt = enemy.GetEntity();
	if ( !enemyEnt ) {
		return false;
	}

	const idVec3 enemyOrigin = enemyEnt->GetPhysics()->GetOrigin();
	const idMat3 clipAxis = physicsObj.GetAxis();
	const idVec3 up = -physicsObj.GetGravityNormal();

	for ( int attempt = 0; attempt < 12; ++attempt ) {
		const float yaw = gameLocal.random.RandomFloat() * 360.0f;
		const idVec3 radial = idAngles( 0.0f, yaw, 0.0f ).ToMat3()[ 0 ];
		const float range = teleportMinRange + gameLocal.random.RandomFloat() * ( teleportMaxRange - teleportMinRange );
		const idVec3 start = enemyOrigin + radial * range + up * 160.0f;
		const idVec3 end = start - up * 640.0f;

		trace_t floorTrace;
		gameLocal.clip.Translation( floorTrace, start, end, physicsObj.GetClipModel(), clipAxis, MASK_MONSTERSOLID, this );
		if ( floorTrace.fraction >= 1.0f || floorTrace.c.normal * up < 0.65f ) {
			continue;
		}

		const idVec3 candidate = floorTrace.endpos + up * CM_CLIP_EPSILON;
		trace_t occupancyTrace;
		gameLocal.clip.Translation( occupancyTrace, candidate, candidate, physicsObj.GetClipModel(), clipAxis, MASK_MONSTERSOLID, this );
		if ( occupancyTrace.fraction < 1.0f ) {
			continue;
		}

		if ( aas && PointReachableAreaNum( candidate ) == 0 ) {
			continue;
		}

		destination = candidate;
		return true;
	}

	return false;
}

void idAI_DNFBase::Event_Teleport( void ) {
	idVec3 destination;
	if ( !canTeleport || !FindTeleportDestination( destination ) ) {
		idThread::ReturnInt( false );
		return;
	}

	idVec3 faceDirection = enemy.GetEntity()->GetPhysics()->GetOrigin() - destination;
	faceDirection.z = 0.0f;
	Teleport( destination, idAngles( 0.0f, faceDirection.ToYaw(), 0.0f ), NULL );
	StopMove( MOVE_STATUS_DONE );
	idThread::ReturnInt( true );
}

void idAI_DNFBase::SetCloaked( bool enable ) {
	if ( cloaked == enable ) {
		return;
	}

	cloaked = enable;
	if ( cloaked ) {
		cloakEndTime = gameLocal.time + SEC2MS( cloakDuration );
	} else {
		cloakEndTime = 0;
	}
	ApplyCloakVisuals();
}

void idAI_DNFBase::ApplyCloakVisuals( void ) {
	const char *skinName = cloaked ? spawnArgs.GetString( "skin_cloaked", "" ) : spawnArgs.GetString( "skin", "" );
	SetSkin( skinName[ 0 ] ? declManager->FindSkin( skinName ) : NULL );

	for ( int i = 0; i < attachments.Num(); ++i ) {
		idEntity *attachment = attachments[ i ].ent.GetEntity();
		if ( !attachment ) {
			continue;
		}
		if ( cloaked ) {
			attachment->Hide();
		} else if ( !IsHidden() ) {
			attachment->Show();
		}
	}

	UpdateVisuals();
}

void idAI_DNFBase::Event_SetCloaked( int enable ) {
	if ( canCloak || !enable ) {
		SetCloaked( enable != 0 );
	}
}

void idAI_DNFBase::Event_IsCloaked( void ) {
	idThread::ReturnInt( cloaked );
}
