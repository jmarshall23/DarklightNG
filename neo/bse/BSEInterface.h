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

#ifndef __BSE_INTERFACE_H__
#define __BSE_INTERFACE_H__

struct rvBSEStats {
	int effectsServiced;
	int segmentsServiced;
	int particlesEvaluated;
	int particlesRendered;
	int particlesCapped;
};

class rvBSEManager {
public:
	virtual				~rvBSEManager() {}
	virtual void		Init() = 0;
	virtual void		Shutdown() = 0;
	virtual void		BeginFrame() = 0;
	virtual void		ServiceEffect( class rvBSE &effect, const struct rvBSEOwner &owner,
							idList<struct rvBSEParticle> &particles ) = 0;
	virtual void		PrepareRender( const struct rvBSEOwner &owner,
							const idList<struct rvBSEParticle> &particles,
							idList<struct rvBSEParticle> &renderParticles ) = 0;
	virtual const rvBSEStats &GetStats() const = 0;
};

extern rvBSEManager *bse;

#endif
