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

class gltfSceneEditor {
public:
	gltfSceneEditor();
	void Init( );
	void Render(const bgfxContext_t& context );
	void DrawUI( const bgfxContext_t &context );
	bool LoadFile( const char * file );
	bool IsFileLoaded(const char * file );
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