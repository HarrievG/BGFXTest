#ifndef	__BGFX_DEBUG_RENDER_H__
#define	__BGFX_DEBUG_RENDER_H__
#include "bx/handlealloc.h"
#include "bgfx/bgfx.h"
#include "../idFramework/sys/platform.h"
#include "../idFramework/Common.h"

class idDrawVert;
class gltfNode;
class gltfData;

class bgfxDebugRenderer {
public:
	bgfxDebugRenderer( );
	~bgfxDebugRenderer( ) { };

	idDrawVert *AllocTris( int vertCount, const triIndex_t *tempIndexes, int indexCount );

	void _Flush( );
	static void CreateRenderer( );
	static bgfxDebugRenderer * gDebugRender;
	static void Flush( ) { gDebugRender ? gDebugRender->_Flush() : common->DWarning("tried to flush an null Debug Renderer" ); }
	void DrawSkin( gltfNode * node , gltfData * data );
private:
	static const int MAX_INDEXES = ( 20000 * 6 );
	static const int MAX_VERTS = ( 20000 * 4 );

	idDrawVert *	vtxData;
	int				vtxCount;

	triIndex_t *	idxData;
	int				idxCount;

	bgfx::VertexLayout	m_vertexLayout;
	bgfx::UniformHandle s_texColor;
	bgfx::UniformHandle u_dropShadowColor;
	bgfx::UniformHandle u_params;

	bgfx::ProgramHandle program;
	bgfx::FrameBufferHandle frameBuffer;

	bgfx::VertexBufferHandle vertexBufferHandle;
	bgfx::IndexBufferHandle indexBufferHandle;
	
	uint8_t debugView;
	
};

#endif // __BGFX_DEBUG_RENDER_H__
