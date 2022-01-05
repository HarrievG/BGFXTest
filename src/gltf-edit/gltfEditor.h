#pragma once

#include "idFramework/idlib/containers/StrList.h"
#include "bgfx-stubs/bgfxRender.h"
#include "gltfProperties.h"

namespace tinygltf {
	class Model;
	class Image;
};

typedef idList<tinygltf::Model> gltfAssetList;
typedef idList<tinygltf::Model *> gltfAssetPtrList;
typedef tinygltf::Model *gltfAssetPtr;

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
	gltfAssetList& GetLoadedAssets( ) { return loadedAssets; }
	//return BgfxModel if loaded 
	// meshName : the gltf Mesh to return as rendermodel
	// assetName : if emtpy, searches all loaded gltf asset files,
	//				otherwise only in given assetfile.
	bgfxModel* GetRenderModel(const idStr & meshName, const idStr & assetName );
private:
	bool windowOpen;
	idStrList loadedFiles;
	gltfAssetList loadedAssets;
	//HVG_TODO -> Use hash index
	bgfxModelList renderModels; 
	idStrList modelNames;
	//
	bgfxMaterialList materialList;
	idHashIndex materialIndices;
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
	gltfImage * selectedImage;
	gltfMesh * selectedMesh;
};

extern gltfAssetExplorer * assetExplorer;