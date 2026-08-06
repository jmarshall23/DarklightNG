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

#ifndef __BSE_H__
#define __BSE_H__

#include "BSE_Particle.h"
#include "BSEInterface.h"

typedef enum {
	SEG_NONE = 0,
	SEG_EFFECT,
	SEG_EMITTER,
	SEG_SPAWNER,
	SEG_TRAIL,
	SEG_SOUND,
	SEG_DECAL,
	SEG_LIGHT,
	SEG_DELAY,
	SEG_SHAKE,
	SEG_TUNNEL,
	SEG_COUNT
} eBSESegment;

class rvSegmentTemplate {
public:
					rvSegmentTemplate();
	void			Init();
	int				Allocated() const;
	bool			HasVisualParticle() const;

	idStr			name;
	int				type;
	idVec2			startTime;
	idVec2			duration;
	idVec2			count;
	idVec2			density;
	idVec2			attenuation;
	idVec2			soundVolume;
	idVec2			frequencyShift;
	float			particleCap;
	float			scale;
	float			detail;
	int				decalAxis;
	idStr			soundShader;
	idStr			soundChannel;
	idList<idStr>	spawnEffects;
	bool			locked;
	bool			looping;
	bool			constant;
	bool			calculateDuration;
	bool			depthSort;
	bool			inverseDrawOrder;
	bool			useMaterialColor;
	bool			orientateIdentity;
	bool			attenuateEmitter;
	bool			inverseAttenuateEmitter;
	rvParticleTemplate particle;
};

class rvDeclEffect : public idDecl {
public:
					rvDeclEffect();
	virtual			~rvDeclEffect();

	virtual size_t	Size() const;
	virtual const char *DefaultDefinition() const;
	virtual bool	Parse( const char *text, const int textLength );
	virtual void	FreeData();

	float			GetSize() const { return size; }
	float			GetCutOffDistance() const { return cutOffDistance; }
	float			GetMinDuration() const { return minDuration; }
	float			GetMaxDuration() const { return maxDuration; }
	const idBounds &GetBounds() const { return bounds; }
	int				GetNumSegmentTemplates() const { return segments.Num(); }
	const rvSegmentTemplate *GetSegmentTemplate( int index ) const { return &segments[index]; }
	const idList<rvSegmentTemplate> &GetSegments() const { return segments; }

private:
	bool			ParseSegment( idLexer &src, int segmentType );
	bool			ParseParticle( idLexer &src, rvParticleTemplate &particle, int particleType );
	bool			ParseDomainGroup( idLexer &src, rvParticleTemplate &particle, int group );
	bool			ParseAction( idLexer &src, rvBSEAction &action );
	void			Finish();
	void			CalculateBounds();

	float			size;
	float			cutOffDistance;
	float			minDuration;
	float			maxDuration;
	idBounds		bounds;
	idList<rvSegmentTemplate> segments;
};

struct rvBSEOwner {
	float			time;
	float			stopTime;
	float			diversity;
	float			brightness;
	idVec4			color;
	idVec3			gravity;
	idVec3			wind;
	idVec3			endOrigin;
	idVec3			viewOrigin;
	bool			hasEndOrigin;
	bseModelSample_t modelSampler;
};

class rvSegment {
public:
					rvSegment();
	void			Init( const rvSegmentTemplate *segmentTemplate, int index, int effectSeed );
	void			Service( const rvBSEOwner &owner, idList<rvBSEParticle> &particles,
						int maxParticles, rvBSEStats *stats ) const;

private:
	const rvSegmentTemplate *segmentTemplate;
	int				segmentIndex;
	int				seedBase;
};

class rvBSE {
public:
					rvBSE();
	void			Init( const rvDeclEffect *effect );
	void			Service( const rvBSEOwner &owner, idList<rvBSEParticle> &particles );
	const rvDeclEffect *GetEffect() const { return effect; }

private:
	friend class rvBSEManagerLocal;
	void			ServiceInternal( const rvBSEOwner &owner, idList<rvBSEParticle> &particles,
						int depth, rvBSEStats *stats );
	const rvDeclEffect *effect;
	int				seedBase;
	idList<rvSegment> segments;
};

class rvBSEManagerLocal : public rvBSEManager {
public:
	virtual void	Init();
	virtual void	Shutdown();
	virtual void	BeginFrame();
	virtual void	ServiceEffect( rvBSE &effect, const rvBSEOwner &owner,
							idList<rvBSEParticle> &particles );
	virtual void	PrepareRender( const rvBSEOwner &owner,
							const idList<rvBSEParticle> &particles,
							idList<rvBSEParticle> &renderParticles );
	virtual const rvBSEStats &GetStats() const { return stats; }

private:
	rvBSEStats		stats;
};

class idRenderModelStatic;
void BSE_BuildRenderModel( idRenderModelStatic *snapshot, const char *snapshotName,
		const idList<rvBSEParticle> &particles, const rvBSEOwner &owner,
		const idVec3 &localViewOrigin, const idVec3 &viewRight, const idVec3 &viewUp );

// Parser utilities shared by the effect, segment, and particle template units.
idVec2 BSE_ParseRange( idLexer &src, float defaultValue = 0.0f );
idVec3 BSE_ParseVector( idLexer &src, int count );
idStr BSE_ParseString( idLexer &src );
void BSE_SkipUnknown( idLexer &src, const idToken &token );
int BSE_SegmentTypeForToken( const idToken &token );
int BSE_ParticleTypeForToken( const idToken &token );
int BSE_ParmForToken( const idToken &token );
bseDomainType_t BSE_DomainTypeForToken( const idToken &token );

// Renderer-neutral render preparation helpers.
void BSE_CreateElectricity( const rvBSEOwner &owner, const rvBSEParticle &particle,
		idList<rvBSEParticle> &output );
void BSE_CreateTrail( const rvBSEParticle &particle, idList<rvBSEParticle> &output );
void BSE_ResolveModelSafeParticle( rvBSEParticle &particle );
void BSE_SortParticles( const rvBSEOwner &owner, idList<rvBSEParticle> &particles );

#endif
