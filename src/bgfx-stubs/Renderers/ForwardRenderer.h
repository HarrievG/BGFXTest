#pragma once

#include "..\bgfxRenderer.h"
#include "..\..\idFramework\idlib\geometry\DrawVert.h"

class ForwardRenderer : public Renderer
{
public:
    ForwardRenderer(gltfData* sceneData);

    static bool supported();

    virtual void onInitialize() ;
    virtual void onRender(float dt) override;
    virtual void onShutdown() override;
	void RenderSceneNode(uint64_t state, gltfNode *node, idMat4 trans, gltfData* data );
	void SetRenderTargetNode(gltfNode * node);
	idDrawVert * AllocTris( int vertCount, const triIndex_t * tempIndexes, int indexCount);
private:
	static const int MAX_INDEXES = ( 20000 * 6 );
	static const int MAX_VERTS = ( 20000 * 4 );

    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
	int					selectedCameraId =0;

	idDrawVert			*vtxData;
	int					vtxCount;

	triIndex_t			*idxData;
	int					idxCount;
	gltfNode *			targetNode;

};
