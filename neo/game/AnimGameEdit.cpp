/*
===========================================================================

DarklightNG Source Code
Copyright (C) 2026 - Justin Marshall(aka IceColdDuke).

This file is part of the DarklightNG GPL source code.
This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

DarklightNG is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DarklightNG is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

===========================================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

static const idDeclModelDef *AnimModelDefFromEntityDef( const idDict *args ) {
	const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, args->GetString( "model" ), false ) );
	return modelDef && modelDef->ModelHandle() ? modelDef : NULL;
}

idRenderModel *idGameEdit::ANIM_GetModelFromEntityDef( const idDict *args ) {
	idRenderModel *model = NULL;
	const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, args->GetString( "model" ), false ) );
	if ( modelDef ) {
		model = modelDef->ModelHandle();
	}
	if ( !model ) {
		model = renderModelManager->FindModel( args->GetString( "model" ) );
	}
	return model && !model->IsDefaultModel() ? model : NULL;
}

idRenderModel *idGameEdit::ANIM_GetModelFromEntityDef( const char *classname ) {
	const idDict *args = gameLocal.FindEntityDefDict( classname, false );
	return args ? ANIM_GetModelFromEntityDef( args ) : NULL;
}

const idVec3 &idGameEdit::ANIM_GetModelOffsetFromEntityDef( const char *classname ) {
	const idDict *args = gameLocal.FindEntityDefDict( classname, false );
	const idDeclModelDef *modelDef = args ? AnimModelDefFromEntityDef( args ) : NULL;
	return modelDef ? modelDef->GetVisualOffset() : vec3_origin;
}

idRenderModel *idGameEdit::ANIM_GetModelFromName( const char *modelName ) {
	const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, modelName, false ) );
	idRenderModel *model = modelDef ? modelDef->ModelHandle() : NULL;
	return model ? model : renderModelManager->FindModel( modelName );
}

const idMD5Anim *idGameEdit::ANIM_GetAnimFromEntityDef( const char *classname, const char *animname ) {
	const idDict *args = gameLocal.FindEntityDefDict( classname, false );
	if ( !args ) {
		return NULL;
	}
	const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, args->GetString( "model" ), false ) );
	if ( !modelDef ) {
		return NULL;
	}
	const idAnim *anim = modelDef->GetAnim( modelDef->GetAnim( animname ) );
	return anim ? anim->MD5Anim( 0 ) : NULL;
}

int idGameEdit::ANIM_GetNumAnimsFromEntityDef( const idDict *args ) {
	const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, args->GetString( "model" ), false ) );
	return modelDef ? modelDef->NumAnims() : 0;
}

const char *idGameEdit::ANIM_GetAnimNameFromEntityDef( const idDict *args, int animNum ) {
	const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, args->GetString( "model" ), false ) );
	const idAnim *anim = modelDef ? modelDef->GetAnim( animNum ) : NULL;
	return anim ? anim->FullName() : "";
}

const idMD5Anim *idGameEdit::ANIM_GetAnim( const char *fileName ) {
	return animationLib->GetAnim( fileName );
}

int idGameEdit::ANIM_GetLength( const idMD5Anim *anim ) {
	return anim ? anim->Length() : 0;
}

int idGameEdit::ANIM_GetNumFrames( const idMD5Anim *anim ) {
	return anim ? anim->NumFrames() : 0;
}

void idGameEdit::ANIM_CreateAnimFrame( const idRenderModel *model, const idMD5Anim *anim, int numJoints, idJointMat *joints, int time, const idVec3 &offset, bool removeOriginOffset ) {
	animationLib->CreateAnimFrame( model, anim, numJoints, joints, time, offset, removeOriginOffset );
}

idRenderModel *idGameEdit::ANIM_CreateMeshForAnim( idRenderWorld *renderWorld, idRenderModel *model, const char *classname, const char *animname, int frame, bool removeOriginOffset ) {
	if ( !renderWorld || !model || model->IsDefaultModel() ) {
		return NULL;
	}
	const idDict *args = gameLocal.FindEntityDefDict( classname, false );
	if ( !args ) {
		return NULL;
	}

	const idMD5Anim *md5anim = NULL;
	const idDeclSkin *customSkin = NULL;
	idVec3 offset;
	const idDeclModelDef *modelDef = AnimModelDefFromEntityDef( args );
	if ( modelDef ) {
		const idAnim *anim = modelDef->GetAnim( modelDef->GetAnim( animname ) );
		if ( anim ) {
			md5anim = anim->MD5Anim( 0 );
			customSkin = modelDef->GetDefaultSkin();
			offset = modelDef->GetVisualOffset();
		}
	} else {
		idStr filename = animname;
		idStr extension;
		filename.ExtractFileExtension( extension );
		const char *resolvedAnim = extension.Length() ? animname : args->GetString( va( "anim %s", animname ) );
		md5anim = animationLib->GetAnim( resolvedAnim );
		offset.Zero();
	}

	if ( !md5anim ) {
		return NULL;
	}
	const char *skin = args->GetString( "skin", "" );
	if ( skin[ 0 ] ) {
		customSkin = declManager->FindSkin( skin );
	}
	return animationLib->CreateMeshForAnim( renderWorld, model, md5anim, frame, offset, customSkin, removeOriginOffset );
}
