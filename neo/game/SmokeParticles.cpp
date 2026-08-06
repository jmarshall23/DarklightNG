/* Legacy smoke-particle compatibility ABI; BSE owns particle rendering. */

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

idSmokeParticles::idSmokeParticles( void ) {
}

void idSmokeParticles::Init( void ) {
}

void idSmokeParticles::Shutdown( void ) {
}

bool idSmokeParticles::EmitSmoke( const idDeclParticle *smoke, const int startTime,
		const float diversity, const idVec3 &origin, const idMat3 &axis, int timeGroup ) {
	return false;
}

void idSmokeParticles::FreeSmokes( void ) {
}
