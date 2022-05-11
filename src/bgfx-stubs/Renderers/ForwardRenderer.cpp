#include "ForwardRenderer.h"
#include <bx/string.h>
#include "..\gltf-edit\gltfParser.h"


idCVar transposetest( "transposetest", "1", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, "1 to transpose");

extern void WriteIndexPair( triIndex_t *dest, const triIndex_t a, const triIndex_t b );

ForwardRenderer::ForwardRenderer(gltfData* sceneData) : Renderer(sceneData) ,targetNode( nullptr )
{
}

bool ForwardRenderer::supported()
{
    return Renderer::supported();
}

void ForwardRenderer::onInitialize()
{
	bgfx::ShaderHandle vsh		= bgfxCreateShader("shaders/vs_forward.bin","vshader" );
	bgfx::ShaderHandle fsh		= bgfxCreateShader( "shaders/fs_forward.bin", "fsshader" );
	program = bgfx::createProgram( vsh, fsh, true );
}

void ForwardRenderer::RenderSceneNode(uint64_t state, gltfNode *node, idMat4 trans, gltfData* data )
{
	bgfx::ViewId vDefault = 0;
	auto & nodeList = data->NodeList( ); 
	auto & meshList = data->MeshList( );
	auto & matList	= data->MaterialList();
	auto & texList	= data->TextureList();
	auto & imgList	= data->ImageList();
	auto & smpList	= data->SamplerList();

	gltfData::ResolveNodeMatrix( node );
	idMat4 curTrans = trans * node->matrix ;

	for ( auto &child : node->children )
		RenderSceneNode( state, nodeList[child], curTrans, data );

	if ( node->mesh != -1 ) 		
	{
		for ( auto prim : meshList[node->mesh]->primitives )
		{
			bgfx::setTransform( curTrans.Transpose().ToFloatPtr() );

			if(transposetest.GetInteger() == 1)
				setNormalMatrix(curTrans.Transpose());
			else
				setNormalMatrix(curTrans);

			bgfx::setVertexBuffer( 0, prim->vertexBufferHandle );
			bgfx::setIndexBuffer( prim->indexBufferHandle );
			if ( prim->material != -1 ) 			{
				gltfMaterial *material = matList[prim->material];

				uint64_t materialState = pbr.bindMaterial( material, data );
				bgfx::setState( state | materialState );

				bgfx::submit( vDefault, program, 0, ~BGFX_DISCARD_BINDINGS );
			}
		}
	}


}


void ForwardRenderer::SetRenderTargetNode(gltfNode * node) {
	targetNode = node;
}

extern void WriteIndexPair( triIndex_t *dest, const triIndex_t a, const triIndex_t b );

idDrawVert *ForwardRenderer::AllocTris( int vertCount, const triIndex_t *tempIndexes, int indexCount ) {
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

void ForwardRenderer::onRender(float dt)
{
    bgfx::ViewId vDefault = 0;

    bgfx::setViewName(vDefault, "Forward render pass");
    bgfx::setViewClear(vDefault, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, /*0x6495EDFF*/0x0, 1.0f, 0);
    bgfx::setViewRect(vDefault, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vDefault, frameBuffer);

    // empty primitive in case nothing follows
    // this makes sure the clear happens
    bgfx::touch(vDefault);

    if(!data)
        return;

    setViewProjection(vDefault);

    uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA ;

    pbr.bindAlbedoLUT();
    lights.BindLights();

	if ( !targetNode  )
	{
		auto &nodeList = data->NodeList( );
		idMat4 mat;
		auto &scenes = data->SceneList( );
		for ( auto &scene : scenes ) {
			for ( auto &node : scene->nodes ) {
				idMat4 mat = mat4_identity;
				RenderSceneNode( state, nodeList[node], mat, data );
			}
			break; // only the first for now.
		}
	}else
		RenderSceneNode( state, targetNode, mat4_identity, data );

	lights.Update();

    //for(auto & mesh : scene.p)
    //{
    //    idMat4 model = glm::identity<idMat4>();
    //    bgfx::setTransform(glm::value_ptr(model));
    //    setNormalMatrix(model);
    //    bgfx::setVertexBuffer(0, mesh.vertexBuffer);
    //    bgfx::setIndexBuffer(mesh.indexBuffer);
    //    const Material& mat = scene->materials[mesh.material];
    //    uint64_t materialState = pbr.bindMaterial(mat);
    //    bgfx::setState(state | materialState);
    //    bgfx::submit(vDefault, program, 0, ~BGFX_DISCARD_TEXTURE_SAMPLERS);
    //}

    bgfx::discard(BGFX_DISCARD_ALL);
}

void ForwardRenderer::onShutdown()
{
    bgfx::destroy(program);
    program = BGFX_INVALID_HANDLE;
}
