#include "bgfxDebugRenderer.h"
#include "idFramework/idlib/geometry/DrawVert.h"
#include "idFramework/idlib/Lib.h"
#include <SDL_mouse.h>
#include "bgfxRender.h"
#include "bgfxRenderer.h"
#include "math.h"
#include "../idFramework/idlib/containers/List.h"
#include "..//gltf-edit/gltfProperties.h"

#define ALPHA_EPSILON	0.001f

#define STENCIL_DECR -1
#define STENCIL_INCR -2

bgfxDebugRenderer * bgfxDebugRenderer::gDebugRender = nullptr;


class SkinnedVertex {
public:
	idList< idVec3 > verts;
	idList< uint16 > indices;
};

void bgfxDebugRenderer::DrawSkin( gltfNode *node, gltfData *data ) {
	auto &skinList = data->SkinList( );
	auto &nodeList = data->NodeList( );
	auto &meshList = data->MeshList( );



}

void bgfxDebugRenderer::CreateRenderer( ) {
	if ( !gDebugRender )
		gDebugRender = new bgfxDebugRenderer( );
}

void bgfxDebugRenderer::_Flush( ) {
	if ( !idxCount )
		return;

	if ( !bgfx::isValid( frameBuffer ) ) {
		frameBuffer = Renderer::createFrameBuffer( false, false );
		bgfx::setName( frameBuffer, "debugRenderer framebuffer" );
	}

	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;

	uint32_t numVertices = ( uint32_t ) vtxCount;
	uint32_t numIndices = ( uint32_t ) idxCount;

	if ( ( numVertices != bgfx::getAvailTransientVertexBuffer(
		numVertices, m_vertexLayout ) ) ||
		( numIndices != bgfx::getAvailTransientIndexBuffer( numIndices ) ) ) {
		common->Warning( "No space in transient buffers" );
		// not enough space in transient buffer, quit drawing the rest...
		return;
	}

	bgfx::allocTransientVertexBuffer( &tvb, numVertices, m_vertexLayout );
	bgfx::allocTransientIndexBuffer( &tib, numIndices );

	idDrawVert *verts = ( idDrawVert * ) tvb.data;
	memcpy(
		verts, vtxData,
		numVertices * sizeof( idDrawVert ) );

	ImDrawIdx *indices = ( ImDrawIdx * ) tib.data;
	memcpy(
		indices, idxData,
		numIndices * sizeof( ImDrawIdx ) );


	// Setup render state: alpha-blending enabled, no face culling,
	// no depth testing, scissor enabled
	uint64_t state =
		BGFX_STATE_WRITE_RGB |
		BGFX_STATE_BLEND_FUNC(
			BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA );

	static const bgfx::Caps *caps = bgfx::getCaps( );

	// Setup viewport, orthographic projection matrix
	float ortho[16];
	bx::mtxOrtho(
		ortho, 0.0f, 1920.f, 1080.f, 0.0f, 0.0f, 100.0f,
		0.0f, caps->homogeneousDepth );

	bgfx::setViewName( debugView, "debugRender" );
	frameBuffer.idx = 0;
	bgfx::setViewTransform( debugView, NULL, ortho );
	bgfx::setViewRect( debugView, 0, 0, ( uint16_t ) 1920, ( uint16_t ) 1080 );
	bgfx::setViewClear( debugView, BGFX_CLEAR_NONE );
	bgfx::setViewFrameBuffer( debugView, frameBuffer );
	bgfx::setState( state );

	bgfx::setVertexBuffer( 0, &tvb, 0, vtxCount );
	bgfx::setIndexBuffer( &tib, 0, idxCount );
	bgfx::submit( debugView, program );

	vtxCount = 0;
	idxCount = 0;

}

// if writing to write-combined memory, always write indexes as pairs for 32 bit writes
ID_INLINE void WriteIndexPair( triIndex_t *dest, const triIndex_t a, const triIndex_t b ) {
	*( unsigned * ) dest = ( unsigned ) a | ( ( unsigned ) b << 16 );
}

idDrawVert *bgfxDebugRenderer::AllocTris( int vertCount, const triIndex_t *tempIndexes, int indexCount ) {
	if ( vtxData == nullptr )
		vtxData = ( idDrawVert * ) Mem_ClearedAlloc( sizeof( idDrawVert ) * MAX_VERTS );
	if ( idxData == nullptr )
		idxData = ( triIndex_t * ) Mem_ClearedAlloc( sizeof( triIndex_t ) * MAX_INDEXES );

	uint vtxDataSize = 0;

	if ( vertCount + vtxCount >= MAX_VERTS ) {
		common->Warning( "Max Vertex count reached for swf" );
		return nullptr;
	}

	int startVert = vtxCount;
	int startIndex = idxCount;

	vtxCount += vertCount;
	idxCount += indexCount;

	if ( ( startIndex & 1 ) || ( indexCount & 1 ) ) {
		// slow for write combined memory!
		// this should be very rare, since quads are always an even index count
		for ( int i = 0; i < indexCount; i++ ) {
			idxData[startIndex + i] = startVert + tempIndexes[i];
		}
	} else {
		for ( int i = 0; i < indexCount; i += 2 ) {
			WriteIndexPair( idxData + startIndex + i, startVert + tempIndexes[i], startVert + tempIndexes[i + 1] );
		}
	}

	return vtxData + startVert;

}

bgfxDebugRenderer::bgfxDebugRenderer( ) {
	debugView = 48;
	vtxCount = 0;
	vtxData = nullptr;
	idxCount = 0;
	idxData = nullptr;
	frameBuffer = BGFX_INVALID_HANDLE;

	bgfx::ShaderHandle vsh = bgfxCreateShader( "shaders/vs_debugRender.bin", "DBGRDRvs" );
	bgfx::ShaderHandle fsh = bgfxCreateShader( "shaders/fs_debugRender.bin", "DBGRDRfs" );
	program = bgfx::createProgram( vsh, fsh, true );
	//float3;short2;ubyte4;ubyte4;ubyte4;ubyte4;
	m_vertexLayout
		.begin( )
		.add( bgfx::Attrib::Position, 3, bgfx::AttribType::Float )
		.add( bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Half, true )
		.add( bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Uint8, true )
		.add( bgfx::Attrib::TexCoord2, 4, bgfx::AttribType::Uint8, true )
		.add( bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true )
		.add( bgfx::Attrib::Color1, 4, bgfx::AttribType::Uint8, true )
		.end( );

	s_texColor = bgfx::createUniform( "s_texColor", bgfx::UniformType::Sampler );
	u_dropShadowColor = bgfx::createUniform( "u_dropShadowColor", bgfx::UniformType::Vec4 );
	u_params = bgfx::createUniform( "u_params", bgfx::UniformType::Vec4 );
}
