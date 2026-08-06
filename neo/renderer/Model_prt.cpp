/*
===========================================================================

DarklightNG Source Code
Copyright (C) 2026 - Justin Marshall (IceColdDuke).

BSE dynamic-model front end.  Quake Wars-style particle evaluation and
tessellation live in neo/bse; the resulting surfaces are consumed exclusively
by the renderer's BSE screen-space pass.

===========================================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "tr_local.h"
#include "Model_local.h"
#include "../bse/BSE.h"

static const char *bseParticle_SnapshotName = "_BSEParticle_Snapshot_";

namespace {

static idList<idStr> bseMissingDomainModels;

class idRenderModelBSESnapshot : public idRenderModelStatic {
public:
	idRenderModelBSESnapshot() : effect( NULL ) {}

	void SetEffect( const rvDeclEffect *newEffect ) {
		if ( effect != newEffect ) {
			effect = newEffect;
			simulation.Init( effect );
		}
	}

	rvBSE simulation;
	const rvDeclEffect *effect;
};

static bool BSE_SampleModelSurface( const char *modelName, idRandom &random, idVec3 &point, idVec3 &normal ) {
	for ( int i = 0; i < bseMissingDomainModels.Num(); i++ ) {
		if ( !bseMissingDomainModels[i].Icmp( modelName ) ) return false;
	}
	idRenderModel *model = renderModelManager->CheckModel( modelName );
	if ( model == NULL || model->IsDefaultModel() || model->IsDynamicModel() != DM_STATIC ) {
		bseMissingDomainModels.Append( modelName );
		return false;
	}

	int triangleCount = 0;
	for ( int i = 0; i < model->NumSurfaces(); i++ ) {
		const modelSurface_t *surface = model->Surface( i );
		if ( surface != NULL && surface->geometry != NULL ) triangleCount += surface->geometry->numIndexes / 3;
	}
	if ( triangleCount <= 0 ) return false;

	int selected = random.RandomInt( triangleCount );
	for ( int i = 0; i < model->NumSurfaces(); i++ ) {
		const modelSurface_t *surface = model->Surface( i );
		if ( surface == NULL || surface->geometry == NULL ) continue;
		const srfTriangles_t *tri = surface->geometry;
		const int surfaceTriangles = tri->numIndexes / 3;
		if ( selected >= surfaceTriangles ) {
			selected -= surfaceTriangles;
			continue;
		}
		const idDrawVert &a = tri->verts[tri->indexes[selected * 3 + 0]];
		const idDrawVert &b = tri->verts[tri->indexes[selected * 3 + 1]];
		const idDrawVert &c = tri->verts[tri->indexes[selected * 3 + 2]];
		const float root = idMath::Sqrt( random.RandomFloat() );
		const float baryA = 1.0f - root;
		const float baryB = root * ( 1.0f - random.RandomFloat() );
		const float baryC = 1.0f - baryA - baryB;
		point = a.xyz * baryA + b.xyz * baryB + c.xyz * baryC;
		normal = a.normal * baryA + b.normal * baryB + c.normal * baryC;
		if ( normal.Normalize() == 0.0f ) {
			normal.Cross( b.xyz - a.xyz, c.xyz - a.xyz );
			normal.Normalize();
		}
		return true;
	}
	return false;
}

} // namespace

idRenderModelBSE::idRenderModelBSE() {
	effectSystem = NULL;
}

void idRenderModelBSE::InitFromFile( const char *fileName ) {
	name = fileName;
	idStr declName = fileName;
	declName.StripFileExtension();
	effectSystem = static_cast<const rvDeclEffect *>( declManager->FindType( DECL_EFFECT, declName ) );
}

void idRenderModelBSE::TouchData() {
	idStr declName = name;
	declName.StripFileExtension();
	effectSystem = static_cast<const rvDeclEffect *>( declManager->FindType( DECL_EFFECT, declName ) );
	if ( effectSystem == NULL ) return;
	for ( int i = 0; i < effectSystem->GetNumSegmentTemplates(); i++ ) {
		const rvParticleTemplate &particle = effectSystem->GetSegmentTemplate( i )->particle;
		if ( !particle.materialName.IsEmpty() ) declManager->FindMaterial( particle.materialName );
		if ( !particle.trailMaterialName.IsEmpty() ) declManager->FindMaterial( particle.trailMaterialName );
		if ( !particle.modelName.IsEmpty() ) renderModelManager->CheckModel( particle.modelName );
	}
}

idRenderModel *idRenderModelBSE::InstantiateDynamicModel( const idRenderEntity *renderEntity,
		const struct viewDef_s *viewDef, idRenderModel *cachedModel ) {
	if ( cachedModel != NULL && !r_useCachedDynamicModels.GetBool() ) {
		delete cachedModel;
		cachedModel = NULL;
	}
	if ( renderEntity == NULL || viewDef == NULL || effectSystem == NULL || r_skipParticles.GetBool() ) {
		delete cachedModel;
		return NULL;
	}

	idRenderModelBSESnapshot *snapshot = dynamic_cast<idRenderModelBSESnapshot *>( cachedModel );
	if ( cachedModel != NULL && snapshot == NULL ) {
		delete cachedModel;
		cachedModel = NULL;
	}
	if ( snapshot == NULL ) {
		snapshot = new idRenderModelBSESnapshot;
		snapshot->InitEmpty( bseParticle_SnapshotName );
	}
	snapshot->SetEffect( effectSystem );

	float modelMatrix[16];
	R_AxisToModelMatrix( renderEntity->GetAxis(), renderEntity->GetOrigin(), modelMatrix );
	rvBSEOwner owner;
	owner.time = viewDef->renderView.time * 0.001f + renderEntity->GetShaderParm( SHADERPARM_TIMEOFFSET );
	owner.diversity = renderEntity->GetShaderParm( SHADERPARM_DIVERSITY );
	const float shaderBrightness = renderEntity->GetShaderParm( SHADERPARM_BRIGHTNESS );
	// Doom render entities default parms 4..11 to zero, while ETQW render
	// effects default brightness to one. Preserve that BSE default for map
	// entities that do not explicitly initialize the bridge-only parm.
	owner.brightness = shaderBrightness == 0.0f ? 1.0f : shaderBrightness;
	owner.color.Set( renderEntity->GetShaderParm( SHADERPARM_RED ), renderEntity->GetShaderParm( SHADERPARM_GREEN ),
		renderEntity->GetShaderParm( SHADERPARM_BLUE ), renderEntity->GetShaderParm( SHADERPARM_ALPHA ) );
	owner.wind.Zero();
	const idVec3 worldGravity( 0.0f, 0.0f, -1066.0f );
	R_GlobalVectorToLocal( modelMatrix, worldGravity, owner.gravity );
	owner.hasEndOrigin = renderEntity->GetShaderParm( SHADERPARM_BSE_HAS_ENDORIGIN ) != 0.0f;
	owner.endOrigin.Zero();
	if ( owner.hasEndOrigin ) {
		const idVec3 worldEnd( renderEntity->GetShaderParm( SHADERPARM_BSE_END_X ),
			renderEntity->GetShaderParm( SHADERPARM_BSE_END_Y ), renderEntity->GetShaderParm( SHADERPARM_BSE_END_Z ) );
		R_GlobalPointToLocal( modelMatrix, worldEnd, owner.endOrigin );
		owner.stopTime = 0.0f;
	} else {
		const float globalStopTime = renderEntity->GetShaderParm( SHADERPARM_PARTICLE_STOPTIME );
		owner.stopTime = globalStopTime > 0.0f ? globalStopTime + renderEntity->GetShaderParm( SHADERPARM_TIMEOFFSET ) : 0.0f;
	}

	idVec3 localViewOrigin;
	idVec3 localViewLeft;
	idVec3 localViewUp;
	R_GlobalPointToLocal( modelMatrix, viewDef->renderView.vieworg, localViewOrigin );
	R_GlobalVectorToLocal( modelMatrix, viewDef->renderView.viewaxis[1], localViewLeft );
	R_GlobalVectorToLocal( modelMatrix, viewDef->renderView.viewaxis[2], localViewUp );
	idVec3 localViewRight = -localViewLeft;
	localViewRight.Normalize();
	localViewUp.Normalize();
	owner.viewOrigin = localViewOrigin;
	owner.modelSampler = BSE_SampleModelSurface;

	idList<rvBSEParticle> particles;
	snapshot->simulation.Service( owner, particles );
	idList<rvBSEParticle> renderParticles;
	if ( bse != NULL ) bse->PrepareRender( owner, particles, renderParticles );
	else renderParticles = particles;
	BSE_BuildRenderModel( snapshot, bseParticle_SnapshotName, renderParticles, owner,
		localViewOrigin, localViewRight, localViewUp );
	return snapshot;
}

dynamicModel_t idRenderModelBSE::IsDynamicModel() const {
	return DM_CONTINUOUS;
}

idBounds idRenderModelBSE::Bounds( const idRenderEntity *renderEntity ) const {
	return effectSystem != NULL ? effectSystem->GetBounds() :
		idBounds( idVec3( -8.0f, -8.0f, -8.0f ), idVec3( 8.0f, 8.0f, 8.0f ) );
}

float idRenderModelBSE::DepthHack() const {
	return 0.0f;
}

int idRenderModelBSE::Memory() const {
	return idRenderModelStatic::Memory() + ( effectSystem != NULL ? static_cast<int>( effectSystem->Size() ) : 0 );
}
