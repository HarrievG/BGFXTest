#pragma once


class gltfSceneEditor {
public:
	gltfSceneEditor();
	void Render( );
	void DrawUI( );
	bool LoadFile( const char * file );
private:
	bool mWindowOpen;
};