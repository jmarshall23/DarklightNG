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
#include "AI_PigCop.h"

static const int PIG_DECISION_INTERVAL_MS = 200;

idAI_PigCop::idAI_PigCop() {
	berserk = false;
	berserkRollMade = false;
	shotgunVariant = false;
	initialHealth = 0;
	muzzleIndex = 0;
	pendingBurstShots = 0;
	nextDecisionTime = 0;
	nextAttackTime = 0;
	normalSpeed = 280.0f;
	berserkSpeed = 360.0f;
	maximumAttackRange = 1000.0f;
	berserkHealthFraction = 0.45f;
	berserkChance = 0.5f;
	volleyDelay = 0.5f;
	burstInterval = 0.05f;
	burstMin = 10;
	burstRandom = 6;
	shotgunPellets = 8;
	shotgunSpread = 10.0f;
}

void idAI_PigCop::Spawn( void ) {
	normalSpeed = spawnArgs.GetFloat( "pig_ground_speed", "280" );
	berserkSpeed = spawnArgs.GetFloat( "pig_berserk_speed", "360" );
	maximumAttackRange = spawnArgs.GetFloat( "pig_max_attack_range", "1000" );
	berserkHealthFraction = spawnArgs.GetFloat( "pig_berserk_health_fraction", "0.45" );
	berserkChance = spawnArgs.GetFloat( "pig_berserk_chance", "0.5" );
	volleyDelay = spawnArgs.GetFloat( "pig_volley_delay", "0.5" );
	burstInterval = spawnArgs.GetFloat( "pig_burst_interval", "0.05" );
	burstMin = spawnArgs.GetInt( "pig_burst_min", "10" );
	burstRandom = spawnArgs.GetInt( "pig_burst_random", "6" );
	shotgunVariant = spawnArgs.GetBool( "pig_shotgun", "0" );
	shotgunPellets = Max( 1, spawnArgs.GetInt( "pig_shotgun_pellets", "8" ) );
	shotgunSpread = Max( 0.0f, spawnArgs.GetFloat( "pig_shotgun_spread", "10" ) );

	initialHealth = Max( 1, health );
	berserk = spawnArgs.GetBool( "pig_start_berserk", "0" );
	berserkRollMade = berserk;
	muzzleIndex = 0;
	pendingBurstShots = 0;
	nextDecisionTime = gameLocal.time;
	nextAttackTime = gameLocal.time;

	SetPigMoveSpeed( berserk ? berserkSpeed : normalSpeed );
	ApplyBerserkVisuals();
}

void idAI_PigCop::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( berserk );
	savefile->WriteBool( berserkRollMade );
	savefile->WriteBool( shotgunVariant );
	savefile->WriteInt( initialHealth );
	savefile->WriteInt( muzzleIndex );
	savefile->WriteInt( pendingBurstShots );
	savefile->WriteInt( nextDecisionTime );
	savefile->WriteInt( nextAttackTime );
	savefile->WriteFloat( normalSpeed );
	savefile->WriteFloat( berserkSpeed );
	savefile->WriteFloat( maximumAttackRange );
	savefile->WriteFloat( berserkHealthFraction );
	savefile->WriteFloat( berserkChance );
	savefile->WriteFloat( volleyDelay );
	savefile->WriteFloat( burstInterval );
	savefile->WriteInt( burstMin );
	savefile->WriteInt( burstRandom );
	savefile->WriteInt( shotgunPellets );
	savefile->WriteFloat( shotgunSpread );
}

void idAI_PigCop::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( berserk );
	savefile->ReadBool( berserkRollMade );
	savefile->ReadBool( shotgunVariant );
	savefile->ReadInt( initialHealth );
	savefile->ReadInt( muzzleIndex );
	savefile->ReadInt( pendingBurstShots );
	savefile->ReadInt( nextDecisionTime );
	savefile->ReadInt( nextAttackTime );
	savefile->ReadFloat( normalSpeed );
	savefile->ReadFloat( berserkSpeed );
	savefile->ReadFloat( maximumAttackRange );
	savefile->ReadFloat( berserkHealthFraction );
	savefile->ReadFloat( berserkChance );
	savefile->ReadFloat( volleyDelay );
	savefile->ReadFloat( burstInterval );
	savefile->ReadInt( burstMin );
	savefile->ReadInt( burstRandom );
	savefile->ReadInt( shotgunPellets );
	savefile->ReadFloat( shotgunSpread );
	SetPigMoveSpeed( berserk ? berserkSpeed : normalSpeed );
	ApplyBerserkVisuals();
}

void idAI_PigCop::SetPigMoveSpeed( float speed ) {
	const float oldSpeed = fly_speed;
	move.moveType = MOVETYPE_SLIDE;
	travelFlags = TFL_WALK | TFL_AIR;
	if ( move.speed == oldSpeed ) {
		move.speed = speed;
	}
	fly_speed = speed;
}

void idAI_PigCop::ApplyBerserkVisuals( void ) {
	for ( int i = 0; i < attachments.Num(); ++i ) {
		idEntity *attachment = attachments[ i ].ent.GetEntity();
		if ( !attachment ) {
			continue;
		}
		if ( berserk ) {
			attachment->Hide();
		} else if ( !IsHidden() ) {
			attachment->Show();
		}
	}
	UpdateVisuals();
}

void idAI_PigCop::Event_ChooseAction( void ) {
	idActor *enemyEnt = enemy.GetEntity();
	if ( !enemyEnt || health <= 0 || gameLocal.time < nextDecisionTime ) {
		idThread::ReturnInt( PIG_ACTION_NONE );
		return;
	}

	nextDecisionTime = gameLocal.time + PIG_DECISION_INTERVAL_MS;

	if ( !berserk && !berserkRollMade && health <= initialHealth * berserkHealthFraction ) {
		berserkRollMade = true;
		if ( gameLocal.random.RandomFloat() < berserkChance ) {
			idThread::ReturnInt( PIG_ACTION_BERSERK );
			return;
		}
	}

	if ( gameLocal.time < nextAttackTime ) {
		idThread::ReturnInt( PIG_ACTION_NONE );
		return;
	}

	if ( TestMelee() ) {
		nextAttackTime = gameLocal.time + SEC2MS( berserk ? 0.7f : 1.0f );
		idThread::ReturnInt( PIG_ACTION_MELEE );
		return;
	}

	if ( berserk ) {
		idThread::ReturnInt( PIG_ACTION_NONE );
		return;
	}

	const float enemyRange = ( enemyEnt->GetPhysics()->GetOrigin() - physicsObj.GetOrigin() ).LengthFast();
	if ( AI_ENEMY_VISIBLE && enemyRange <= maximumAttackRange ) {
		pendingBurstShots = shotgunVariant ? 1 : Max( 1, burstMin );
		if ( !shotgunVariant && burstRandom > 0 ) {
			pendingBurstShots += gameLocal.random.RandomInt( burstRandom + 1 );
		}
		nextAttackTime = gameLocal.time + SEC2MS( volleyDelay + gameLocal.random.RandomFloat() * 0.2f );
		idThread::ReturnInt( PIG_ACTION_MISSILE );
		return;
	}

	idThread::ReturnInt( PIG_ACTION_NONE );
}

void idAI_PigCop::Event_GetBurstCount( void ) {
	const int count = Max( 1, pendingBurstShots );
	pendingBurstShots = 0;
	idThread::ReturnInt( count );
}

void idAI_PigCop::Event_GetBurstInterval( void ) {
	idThread::ReturnFloat( burstInterval );
}

void idAI_PigCop::Event_GetMuzzleSide( void ) {
	idThread::ReturnInt( muzzleIndex & 1 );
}

bool idAI_PigCop::GetWeaponMuzzle( int side, idVec3 &muzzle, idMat3 &axis ) {
	const char *attachmentJoint = shotgunVariant ? "mount_shotgun_r" : ( side ? "mount_pistol_r" : "mount_pistol_l" );
	for ( int i = 0; i < attachments.Num(); ++i ) {
		idEntity *attachment = attachments[ i ].ent.GetEntity();
		if ( !attachment || idStr::Icmp( attachment->spawnArgs.GetString( "joint" ), attachmentJoint ) ||
			!attachment->IsType( idAnimatedEntity::Type ) ) {
			continue;
		}

		idAnimatedEntity *animatedAttachment = static_cast<idAnimatedEntity *>( attachment );
		const jointHandle_t muzzleJoint = animatedAttachment->GetAnimator()->GetJointHandle( "mount_muzzle" );
		if ( muzzleJoint != INVALID_JOINT && animatedAttachment->GetJointWorldTransform( muzzleJoint, gameLocal.time, muzzle, axis ) ) {
			return true;
		}
	}

	const jointHandle_t handJoint = animator.GetJointHandle( attachmentJoint );
	if ( handJoint != INVALID_JOINT && GetJointWorldTransform( handJoint, gameLocal.time, muzzle, axis ) ) {
		muzzle += idVec3( 10.0f, 0.0f, 0.0f ) * axis;
		return true;
	}

	muzzle = GetEyePosition();
	axis = viewAxis;
	return false;
}

void idAI_PigCop::PlayWeaponFireAnim( int side ) {
	const char *attachmentJoint = shotgunVariant ? "mount_shotgun_r" : ( side ? "mount_pistol_r" : "mount_pistol_l" );
	for ( int i = 0; i < attachments.Num(); ++i ) {
		idEntity *attachment = attachments[ i ].ent.GetEntity();
		if ( !attachment || idStr::Icmp( attachment->spawnArgs.GetString( "joint" ), attachmentJoint ) ||
			!attachment->IsType( idAnimatedEntity::Type ) ) {
			continue;
		}

		idAnimator *attachmentAnimator = static_cast<idAnimatedEntity *>( attachment )->GetAnimator();
		const int fireAnim = attachmentAnimator->GetAnim( "fire" );
		if ( fireAnim ) {
			attachmentAnimator->PlayAnim( ANIMCHANNEL_ALL, fireAnim, gameLocal.time, 0 );
		}
		return;
	}
}

void idAI_PigCop::Event_FireProjectile( void ) {
	idActor *enemyEnt = enemy.GetEntity();
	if ( berserk || !enemyEnt || !projectileDef ) {
		return;
	}

	const int side = muzzleIndex & 1;
	idVec3 muzzle;
	idMat3 muzzleAxis;
	GetWeaponMuzzle( side, muzzle, muzzleAxis );

	idVec3 aimDir;
	if ( !GetAimDir( muzzle, enemyEnt, this, aimDir ) ) {
		aimDir = muzzleAxis[ 0 ];
	}

	PlayWeaponFireAnim( side );
	if ( shotgunVariant ) {
		const idMat3 aimAxis = aimDir.ToMat3();
		const float spreadRadians = DEG2RAD( shotgunSpread );
		for ( int pellet = 0; pellet < shotgunPellets; ++pellet ) {
			const float angle = idMath::Sin( spreadRadians * gameLocal.random.RandomFloat() );
			const float spin = DEG2RAD( 360.0f ) * gameLocal.random.RandomFloat();
			idVec3 pelletDir = aimAxis[ 0 ] + aimAxis[ 2 ] * ( angle * idMath::Sin( spin ) ) -
				aimAxis[ 1 ] * ( angle * idMath::Cos( spin ) );
			pelletDir.Normalize();
			Event_LaunchMissile( muzzle, pelletDir.ToAngles() );
		}
	} else {
		Event_LaunchMissile( muzzle, aimDir.ToAngles() );
		++muzzleIndex;
	}
}

void idAI_PigCop::Event_MeleeAttack( void ) {
	const char *damageDef = berserk ?
		spawnArgs.GetString( "def_melee_berserk", "melee_pigcop_berserk" ) :
		spawnArgs.GetString( "def_melee", "melee_pigcop" );
	AttackMelee( damageDef );
}

void idAI_PigCop::Event_BeginBerserk( void ) {
	if ( berserk ) {
		return;
	}

	berserk = true;
	berserkRollMade = true;
	nextAttackTime = gameLocal.time;
	SetPigMoveSpeed( berserkSpeed );
	ApplyBerserkVisuals();
}

void idAI_PigCop::Event_IsBerserk( void ) {
	idThread::ReturnInt( berserk );
}
