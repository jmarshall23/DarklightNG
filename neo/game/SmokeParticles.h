/*
===========================================================================

Legacy smoke-particle compatibility ABI.

The Doom 3 smoke renderer has been retired in favor of BSE .effect models.
Game and savegame classes still carry idDeclParticle pointers, so this small
shell remains until those serialized fields are migrated without changing
their layout. It never allocates a render entity or emits geometry.

===========================================================================
*/

#ifndef __SMOKEPARTICLES_H__
#define __SMOKEPARTICLES_H__

class idSmokeParticles {
public:
				idSmokeParticles( void );
	void		Init( void );
	void		Shutdown( void );
	bool		EmitSmoke( const idDeclParticle *smoke, const int startTime, const float diversity,
					const idVec3 &origin, const idMat3 &axis, int timeGroup );
	void		FreeSmokes( void );
};

#endif
