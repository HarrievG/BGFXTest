#pragma once

#include "..\bgfxRenderer.h"

class ForwardRenderer : public Renderer
{
public:
    ForwardRenderer(gltfData* sceneData);

    static bool supported();

    virtual void onInitialize() override;
    virtual void onRender(float dt) override;
    virtual void onShutdown() override;
	void ForwardRenderer::RenderSceneNode(uint64_t state, gltfNode *node, idMat4 trans, gltfData* data );
private:
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
	int					selectedCameraId =0;
};
