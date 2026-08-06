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

#include "BSE.h"

static rvBSEManagerLocal bseManagerLocal;
rvBSEManager *bse = &bseManagerLocal;

void rvBSEManagerLocal::Init() {
	BeginFrame();
}

void rvBSEManagerLocal::Shutdown() {
	BeginFrame();
}

void rvBSEManagerLocal::BeginFrame() {
	memset( &stats, 0, sizeof( stats ) );
}

void rvBSEManagerLocal::ServiceEffect( rvBSE &effect, const rvBSEOwner &owner,
		idList<rvBSEParticle> &particles ) {
	particles.Clear();
	effect.ServiceInternal( owner, particles, 0, &stats );
}

void rvBSEManagerLocal::PrepareRender( const rvBSEOwner &owner,
		const idList<rvBSEParticle> &particles, idList<rvBSEParticle> &renderParticles ) {
	renderParticles.Clear();
	for ( int i = 0; i < particles.Num(); i++ ) {
		const rvBSEParticle &particle = particles[i];
		if ( particle.type == PTYPE_ELECTRICITY ) {
			BSE_CreateElectricity( owner, particle, renderParticles );
		} else {
			renderParticles.Append( particle );
		}
		BSE_CreateTrail( particle, renderParticles );
	}
	BSE_SortParticles( owner, renderParticles );
	stats.particlesRendered += renderParticles.Num();
}
