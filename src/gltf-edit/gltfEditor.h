#pragma once

#include "idFramework/idlib/containers/StrList.h"

class gltfSceneEditor {
public:
	gltfSceneEditor();
	void Init( );
	void Render( );
	void DrawUI( );
	bool LoadFile( const char * file );
	bool IsFileLoaded(const char * file );
	idStrList& GetLoadedFiles( ) {return loadedFiles; }
private:
	bool windowOpen;
	idStrList loadedFiles;
};