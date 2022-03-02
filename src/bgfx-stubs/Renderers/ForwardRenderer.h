#pragma once

#include "..\bgfxRenderer.h"

class ForwardRenderer : public Renderer
{
public:
    ForwardRenderer(gltfData* sceneData);

    static bool supported();

    virtual void onInitialize() ;
    virtual void onRender(float dt) override;
    virtual void onShutdown() override;
	void RenderSceneNode(uint64_t state, gltfNode *node, idMat4 trans, gltfData* data );
	void RenderText(const char * text,idVec2 screenpos );
private:
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
	int					selectedCameraId =0;
};

class FontRenderer : public Renderer {
public:
	FontRenderer( gltfData * sceneData );

	static bool supported( );

	virtual void onInitialize( );
	virtual void onRender( float dt ) override;
	virtual void onShutdown( ) override;
	void RenderSceneNode( uint64_t state, gltfNode *node, idMat4 trans, gltfData *data );
	void RenderText( const char *text, idVec2 screenpos );
private:
	bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
	int					selectedCameraId = 0;
};
