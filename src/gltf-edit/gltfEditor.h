#pragma once

#include "idFramework/idlib/containers/StrList.h"
#include "bgfx-stubs/bgfxRender.h"
#include "gltfProperties.h"

typedef idList<bgfxModel> bgfxModelList;
typedef idList<bgfxMaterial> bgfxMaterialList;

class gltfSceneEditor : public imDrawable, bgfxRenderable {
	friend class gltfAssetExplorer;
public:
	gltfSceneEditor();
	void Init( );
	void Shutdown( );
	virtual bool Render( bgfxContext_t *context ) override;
	virtual bool imDraw( bgfxContext_t *context ) override;
	virtual bool Show( bool visible ) override;
	virtual bool isVisible( ) override { return windowOpen; };

	bool LoadFile( const char *file );
	int IsFileLoaded( const char *file );

	idStrList& GetLoadedFiles( ) {return loadedFiles; }

private:
	void DrawSceneList ();
	bool DrawSceneNode ( gltfNode *node, const idList<gltfNode *> &nodeList );
	void RenderSceneNode( bgfxContext_t *context , gltfNode *node, idMat4 trans, const idList<gltfNode *> &nodeList );
	void DrawCameraInfo(gltfCamera* camera );
	void DrawNodeInfo(gltfNode * node );
	bool				windowOpen;
	bool				sceneViewOpen;
	bool				sceneListOpen;
	idStrList			loadedFiles;
	gltfScene *			selectedScene;
	gltfData *			currentData;
	gltfNode *			selectedNode;
	gltfCamera *		currentCamera;
	gltfData *			editorData;
	int					selectedCameraId;

	//HVG_TODO -> Use hash index
	bgfxModelList		renderModels; 
	idStrList			modelNames;
	//
	bgfxMaterialList	materialList;
	idHashIndex			materialIndices;

	idMat4				cameraView;
	idMat4				cameraProjection;

	bgfxMrtContext_t	renderTarget;

	idQuat				anglesQ;
	idAngles			anglesX;
	idVec3				scale;
	idVec3				pos;
	idMat4				curtrans;
};
extern gltfSceneEditor * sceneEditor;

class gltfAssetExplorer : public imDrawable,bgfxRenderable
{
public:
	gltfAssetExplorer( );
	void Init( );
	virtual ~gltfAssetExplorer( );
	virtual bool Render( bgfxContext_t *context ) override;
	virtual bool imDraw( bgfxContext_t *context ) override;
	virtual bool Show(bool visible ) override ;
	virtual bool isVisible( ) override { return guiVisible;};

	void DrawImAssetTree();
private:
	bool guiVisible;

	int selectedFileHash;
	gltfImage	*		selectedImage;
	gltfMesh	*		selectedMesh;
	gltfData	*		currentData;
	idMat4				cameraView;
	idMat4				cameraProjection;

	idVec3				camPos;
	idAngles			camAngle;

	bgfxMrtContext_t	renderTarget;
};

extern gltfAssetExplorer * assetExplorer;