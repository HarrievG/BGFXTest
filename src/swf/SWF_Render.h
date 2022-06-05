#ifndef	__SWF_RENDER_H__
#define	__SWF_RENDER_H__
#include "bx/handlealloc.h"

class swfRenderer
{
public:
	swfRenderer();
	~swfRenderer(){};

	idDrawVert * AllocTris( int vertCount, const triIndex_t * tempIndexes, int indexCount);
	void Flush();
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
	uint8_t swfView;
};

#endif // __SWF_RENDER_H__
