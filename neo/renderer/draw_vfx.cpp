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

#include "tr_local.h"

extern void RB_STD_T_RenderShaderPasses( const drawSurf_t *surf );
extern void RB_SetProgramEnvironment( void );

static const int VFX_INITIAL_VERTEX_BYTES = 4 * 1024 * 1024;
static const int VFX_INITIAL_INDEX_BYTES = 1024 * 1024;

static idVertexBuffer *vfxVertexBuffer;
static idIndexBuffer *vfxIndexBuffer;
static int vfxVertexCapacity;
static int vfxIndexCapacity;
static int vfxVertexUsed;
static int vfxIndexUsed;
static int vfxGeneration;

static int RB_VFX_Align( int value, int alignment ) {
	return ( value + alignment - 1 ) & ~( alignment - 1 );
}

static int RB_VFX_GrowCapacity( int current, int required ) {
	int size = current > 0 ? current : 4096;
	while ( size < required ) {
		size *= 2;
	}
	return size;
}

void RB_VFX_Init() {
	if ( vfxVertexBuffer || vfxIndexBuffer ) {
		return;
	}
	if ( !glConfig.ARBVertexBufferObjectAvailable ) {
		common->Error( "RB_VFX_Init: particle rendering requires ARB_vertex_buffer_object" );
	}
	vfxVertexBuffer = new idVertexBuffer;
	vfxIndexBuffer = new idIndexBuffer;
	vfxVertexCapacity = VFX_INITIAL_VERTEX_BYTES;
	vfxIndexCapacity = VFX_INITIAL_INDEX_BYTES;
	if ( !vfxVertexBuffer->AllocBufferObject( NULL, vfxVertexCapacity, GL_STREAM_DRAW_ARB ) ||
		 !vfxIndexBuffer->AllocBufferObject( NULL, vfxIndexCapacity, GL_STREAM_DRAW_ARB ) ) {
		common->Error( "RB_VFX_Init: failed to allocate shared particle buffers" );
	}
	vfxVertexUsed = 0;
	vfxIndexUsed = 0;
	vfxGeneration = 1;
}

void RB_VFX_Shutdown() {
	delete vfxVertexBuffer;
	delete vfxIndexBuffer;
	vfxVertexBuffer = NULL;
	vfxIndexBuffer = NULL;
	vfxVertexCapacity = 0;
	vfxIndexCapacity = 0;
	vfxVertexUsed = 0;
	vfxIndexUsed = 0;
	++vfxGeneration;
}

void RB_VFX_BeginFrame() {
	if ( !vfxVertexBuffer || !vfxIndexBuffer ) {
		return;
	}
	vfxVertexBuffer->Bind();
	qglBufferDataARB( GL_ARRAY_BUFFER_ARB, (GLsizeiptrARB)vfxVertexCapacity, NULL, GL_STREAM_DRAW_ARB );
	vfxIndexBuffer->Bind();
	qglBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, (GLsizeiptrARB)vfxIndexCapacity, NULL, GL_STREAM_DRAW_ARB );
	vfxVertexUsed = 0;
	vfxIndexUsed = 0;
	++vfxGeneration;
}

bool RB_VFX_BindSurface( const srfTriangles_t *tri, int &vertexOffset, int &indexOffset ) {
	if ( tri == NULL || !tri->isBSE || tri->numVerts <= 0 || tri->numIndexes <= 0 ||
		 tri->verts == NULL || tri->indexes == NULL ) {
		return false;
	}
	if ( !vfxVertexBuffer || !vfxIndexBuffer ) {
		RB_VFX_Init();
	}

	if ( tri->vfxGeneration == vfxGeneration ) {
		vertexOffset = tri->vfxVertexOffset;
		indexOffset = tri->vfxIndexOffset;
		vfxVertexBuffer->Bind();
		vfxIndexBuffer->Bind();
		return true;
	}

	const int vertexBytes = tri->numVerts * sizeof( tri->verts[0] );
	const int indexBytes = tri->numIndexes * sizeof( tri->indexes[0] );
	int newVertexOffset = RB_VFX_Align( vfxVertexUsed, 16 );
	int newIndexOffset = RB_VFX_Align( vfxIndexUsed, 4 );

	if ( newVertexOffset + vertexBytes > vfxVertexCapacity || newIndexOffset + indexBytes > vfxIndexCapacity ) {
		vfxVertexCapacity = RB_VFX_GrowCapacity( vfxVertexCapacity, vertexBytes );
		vfxIndexCapacity = RB_VFX_GrowCapacity( vfxIndexCapacity, indexBytes );
		vfxVertexBuffer->AllocBufferObject( NULL, vfxVertexCapacity, GL_STREAM_DRAW_ARB );
		vfxIndexBuffer->AllocBufferObject( NULL, vfxIndexCapacity, GL_STREAM_DRAW_ARB );
		vfxVertexUsed = 0;
		vfxIndexUsed = 0;
		newVertexOffset = 0;
		newIndexOffset = 0;
		++vfxGeneration;
	}

	vfxVertexBuffer->Bind();
	qglBufferSubDataARB( GL_ARRAY_BUFFER_ARB, newVertexOffset, (GLsizeiptrARB)vertexBytes, tri->verts );
	vfxIndexBuffer->Bind();
	qglBufferSubDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, newIndexOffset, (GLsizeiptrARB)indexBytes, tri->indexes );

	vfxVertexUsed = newVertexOffset + vertexBytes;
	vfxIndexUsed = newIndexOffset + indexBytes;
	tri->vfxGeneration = vfxGeneration;
	tri->vfxVertexOffset = newVertexOffset;
	tri->vfxIndexOffset = newIndexOffset;
	vertexOffset = newVertexOffset;
	indexOffset = newIndexOffset;
	return true;
}

void RB_VFX_DrawScreenSpacePass( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	if ( drawSurfs == NULL || numDrawSurfs <= 0 ) return;
	RB_LogComment( "---------- RB_VFX_DrawScreenSpacePass ----------\n" );

	GL_SelectTexture( 1 );
	globalImages->BindNull();
	GL_SelectTexture( 0 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
	RB_SetProgramEnvironment();
	backEnd.currentSpace = NULL;

	for ( int i = 0; i < numDrawSurfs; i++ ) {
		const drawSurf_t *surf = drawSurfs[i];
		if ( surf == NULL || surf->geo == NULL || !surf->geo->isBSE ) continue;
		if ( surf->material->SuppressInSubview() ) continue;
		if ( backEnd.viewDef->isXraySubview && surf->space->entityDef && surf->space->entityDef->GetXrayIndex() != 2 ) continue;
		if ( surf->material->GetSort() >= SS_POST_PROCESS && !backEnd.currentRenderCopied ) {
			if ( r_skipPostProcess.GetBool() ) continue;
			if ( backEnd.viewDef->viewEntitys ) {
				globalImages->currentRenderImage->CopyFramebuffer( backEnd.viewDef->viewport.x1,
					backEnd.viewDef->viewport.y1,
					backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1,
					backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1, true );
				globalImages->currentDepthImage->CopyDepthbuffer( backEnd.viewDef->viewport.x1,
					backEnd.viewDef->viewport.y1,
					backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1,
					backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1 );
			}
			backEnd.currentRenderCopied = true;
		}
		RB_STD_T_RenderShaderPasses( surf );
	}

	GL_Cull( CT_FRONT_SIDED );
	qglColor3f( 1.0f, 1.0f, 1.0f );
}
