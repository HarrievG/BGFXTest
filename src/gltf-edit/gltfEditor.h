#pragma once

#include "idFramework/idlib/containers/StrList.h"

namespace tinygltf {
	class Model;
};
typedef idList<tinygltf::Model> gltfAssetList;
typedef idList<tinygltf::Model *> gltfAssetPtrList;
typedef tinygltf::Model *gltfAssetPtr;

class gltfSceneEditor {
public:
	gltfSceneEditor();
	void Init( );
	void Render( );
	void DrawUI( );
	bool LoadFile( const char * file );
	bool IsFileLoaded(const char * file );
	idStrList& GetLoadedFiles( ) {return loadedFiles; }
	gltfAssetList& GetLoadedAssets( ) { return loadedAssets; }
private:
	bool windowOpen;
	idStrList loadedFiles;
	gltfAssetList loadedAssets;
};