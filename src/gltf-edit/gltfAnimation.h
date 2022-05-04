#pragma once
#include "../bgfx-imgui/imgui_impl_bgfx.h"
#include "gltfProperties.h"

class gltfAnimEditor : public imDrawable {
public:
	static void Init();
	gltfAnimEditor( ) { };
	~gltfAnimEditor( ) { };
	bool imDraw( ) override;
	bool Show( bool visible ) override;
	bool isVisible( ) override;
};
extern gltfAnimEditor * animEditor;

class gltfAnimated {
public:
	gltfAnimated( gltfAnimation *animation ) { }
	~gltfAnimated( ) { };
	void Advance( gltfData *data ) { };
};


