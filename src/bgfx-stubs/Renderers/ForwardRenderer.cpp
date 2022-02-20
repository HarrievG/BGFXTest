#include "ForwardRenderer.h"
#include <bx/string.h>
#include "..\gltf-edit\gltfParser.h"

ForwardRenderer::ForwardRenderer(gltfData* sceneData) : Renderer(sceneData) { }

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

	idMat4 mat;
	gltfData::ResolveNodeMatrix( node, &mat);
	idMat4 curTrans = trans * node->matrix ;

	if ( node->mesh != -1 ) 		
	{
		for ( auto prim : meshList[node->mesh]->primitives )
		{
			bgfx::setTransform( curTrans.Transpose().ToFloatPtr() );

			setNormalMatrix(curTrans.Transpose());

			bgfx::setVertexBuffer( 0, prim->vertexBufferHandle );
			bgfx::setIndexBuffer( prim->indexBufferHandle );
			if ( prim->material != -1 ) 			{
				gltfMaterial *material = matList[prim->material];

				uint64_t materialState = pbr.bindMaterial( material, data );
				bgfx::setState( state | materialState );

				bgfx::submit( vDefault, program, 0, ~BGFX_DISCARD_BINDINGS );
			}

		//	if (r_forceRenderMode.GetInteger() != -1 )
		//		bgfxSetRenderMode(viewId, context ,r_forceRenderMode.GetInteger());

			bgfx::submit( vDefault, program );
		}
	}

	for ( auto &child : node->children )
		RenderSceneNode(state, nodeList[child], curTrans, data );
}

void ForwardRenderer::onRender(float dt)
{
    bgfx::ViewId vDefault = 0;

    bgfx::setViewName(vDefault, "Forward render pass");
    bgfx::setViewClear(vDefault, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0);
    bgfx::setViewRect(vDefault, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vDefault, frameBuffer);

    // empty primitive in case nothing follows
    // this makes sure the clear happens
    bgfx::touch(vDefault);

    if(!data)
        return;

    setViewProjection(vDefault);

    uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;

    pbr.bindAlbedoLUT();
    lights.BindLights();

	auto &nodeList = data->NodeList( ); 
	idMat4 mat;
	auto &scenes = data->SceneList( );
	for ( auto &scene : scenes )
		for ( auto &node : scene->nodes)
		{
			idMat4 mat = mat4_identity;
			RenderSceneNode(state, nodeList[node], mat, data);
		}

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
