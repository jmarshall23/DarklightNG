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
#include "AI_LizTroop.h"

bool idAI_LizTroop::GetDNFProjectileMuzzle( idVec3 &muzzle, idMat3 &axis ) {
	const char *jointName = "mount_muzzle";
	if ( isCaptain ) {
		jointName = va( "mount_muzzle_%d", ( muzzleIndex & 3 ) + 1 );
	}

	for ( int i = 0; i < attachments.Num(); ++i ) {
		idEntity *attachment = attachments[ i ].ent.GetEntity();
		if ( !attachment || !attachment->IsType( idAnimatedEntity::Type ) ) {
			continue;
		}
		idAnimatedEntity *animatedAttachment = static_cast<idAnimatedEntity *>( attachment );
		idAnimator *attachmentAnimator = animatedAttachment->GetAnimator();

		const jointHandle_t muzzleJoint = attachmentAnimator->GetJointHandle( jointName );
		if ( muzzleJoint != INVALID_JOINT && animatedAttachment->GetJointWorldTransform( muzzleJoint, gameLocal.time, muzzle, axis ) ) {
			return true;
		}
	}

	return idAI_DNFBase::GetDNFProjectileMuzzle( muzzle, axis );
}

void idAI_LizTroop::PlayDNFWeaponFireAnim( void ) {
	for ( int i = 0; i < attachments.Num(); ++i ) {
		idEntity *attachment = attachments[ i ].ent.GetEntity();
		if ( !attachment || !attachment->IsType( idAnimatedEntity::Type ) ) {
			continue;
		}
		idAnimatedEntity *animatedAttachment = static_cast<idAnimatedEntity *>( attachment );
		idAnimator *attachmentAnimator = animatedAttachment->GetAnimator();

		const int fireAnim = attachmentAnimator->GetAnim( "fire" );
		if ( fireAnim ) {
			attachmentAnimator->PlayAnim( ANIMCHANNEL_ALL, fireAnim, gameLocal.time, 0 );
			return;
		}
	}
}
