#pragma once

#include "idFramework/idlib/containers/StrList.h"
#include "bgfx-stubs/bgfxRender.h"

namespace tinygltf {
	class Model;
};
typedef idList<tinygltf::Model> gltfAssetList;
typedef idList<tinygltf::Model *> gltfAssetPtrList;
typedef tinygltf::Model *gltfAssetPtr;

typedef idList<bgfxModel> bgfxModelList;

class gltfSceneEditor : public imDrawable, bgfxRenderable {
	friend class gltfAssetExplorer;
public:
	gltfSceneEditor();
	void Init( );
	virtual bool Render( bgfxContext_t *context ) override;
	virtual bool imDraw( bgfxContext_t *context ) override;
	virtual bool Show( bool visible ) override;
	virtual bool isVisible( ) override { return windowOpen; };

	bool LoadFile( const char *file );
	bool IsFileLoaded( const char *file );

	idStrList& GetLoadedFiles( ) {return loadedFiles; }
	gltfAssetList& GetLoadedAssets( ) { return loadedAssets; }
	bgfxModel* GetRenderModel(const idStr & name );
private:
	bool windowOpen;
	idStrList loadedFiles;
	gltfAssetList loadedAssets;
	bgfxModelList renderModels;
	idStrList modelNames;
	idHashIndex modelNameMap;		
};
extern gltfSceneEditor * sceneEditor;

class gltfAssetExplorer : public imDrawable,bgfxRenderable
{
public:
	gltfAssetExplorer( );
	virtual ~gltfAssetExplorer( );
	virtual bool Render( bgfxContext_t *context ) override;
	virtual bool imDraw( bgfxContext_t *context ) override;
	virtual bool Show(bool visible ) override ;
	virtual bool isVisible( ) override { return guiVisible;};
private:
	bool guiVisible;
};

extern gltfAssetExplorer * assetExplorer;