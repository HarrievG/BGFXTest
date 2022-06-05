#pragma once

#include "..\bgfxRenderer.h"
#include "..\..\idFramework\idlib\geometry\DrawVert.h"

class ForwardRenderer : public Renderer
{
public:

	struct RenderItem
	{
		int		gltfMeshID;
		int		gltfMesh_PrimitiveID;
		int		gltfSkinID;
		int		gltfMaterialID;
		int		state;
		idMat4	transform;
	};

	class idSort_RenderItem : public idSort_Quick< RenderItem, idSort_RenderItem > {
	public:
		const static int MAX_MESHES = 255;
		void SetData(gltfData * _data ) { data = _data ; }
		int getUniqueID( const RenderItem &item ) const {
			return item.gltfMaterialID 
				+ ((item.gltfMeshID + 1) * (data->MaterialList().Num()+1))
				+ ((item.gltfMesh_PrimitiveID + 1) * (data->MeshList().Num()+1 ) * (data->MaterialList().Num()+1)) ;
		}

		int Compare( const RenderItem & lhs, const RenderItem & rhs ) const { return getUniqueID(lhs) - getUniqueID(rhs);};
		gltfData * data;
	};

    ForwardRenderer(gltfData* sceneData);

    static bool supported();

    virtual void onInitialize() ;
    virtual void onRender(float dt) override;
    virtual void onShutdown() override;
	void RenderSceneNode(uint64_t state, gltfNode *node, idMat4 trans, gltfData* data );
	void SetRenderTargetNode(gltfNode * node);
	idDrawVert * AllocTris( int vertCount, const triIndex_t * tempIndexes, int indexCount);

	void Flush(uint64_t state);
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

	idList<RenderItem>	renderItems;
};
