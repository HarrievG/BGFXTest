#pragma once

#include <bgfx/bgfx.h>
#include "../../idFramework/idlib/math/Vector.h"
#include "../../gltf-edit/gltfProperties.h"

class gltfData;

class LightShader
{
public:
	struct PointLightVertex
	{
		idVec3 position;
		float padding;
		// radiant intensity in W/sr
		// can be calculated from radiant flux
		idVec3 intensity;
		float radius;

		static void init( );
		static bgfx::VertexLayout layout;
	};

	LightShader();
    void Initialize(gltfData* data);
    void Shutdown();
	void Update();
    void BindLights();
	idVec3 GetLightPos (gltfExt_KHR_lights_punctual * light );
private:
    bgfx::UniformHandle lightCountVecUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle ambientLightIrradianceUniform = BGFX_INVALID_HANDLE;

	bgfx::DynamicVertexBufferHandle buffer = BGFX_INVALID_HANDLE;
	PointLightVertex pointLightvertex;
	gltfData* sceneData;
	int lightCount;
	idList<gltfExt_KHR_lights_punctual *> * lightList; 
	bool dirty;
};
