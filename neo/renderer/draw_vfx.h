#ifndef __DRAW_VFX_H__
#define __DRAW_VFX_H__

void RB_VFX_Init();
void RB_VFX_Shutdown();
void RB_VFX_BeginFrame();

// Uploads a camera-facing particle surface into the one shared VFX VBO/IBO,
// binds both objects, and returns byte offsets for vertex attributes/indexes.
bool RB_VFX_BindSurface( const srfTriangles_t *tri, int &vertexOffset, int &indexOffset );

// BSE surfaces are composited in one explicit late-view pass.  Their vertices
// remain world-relative so depth testing works, while sprite facing and line
// widths have already been resolved against the view by BSE_Render.cpp.
void RB_VFX_DrawScreenSpacePass( drawSurf_t **drawSurfs, int numDrawSurfs );

#endif
