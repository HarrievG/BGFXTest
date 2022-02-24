#pragma once

#include <bgfx/bgfx.h>
#include "../../idFramework/idlib/math/Vector.h"
#include "../../gltf-edit/gltfProperties.h"

class gltfData;

class LightShader
{
public:
	enum Type { Directional, Point, Spot, old};
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

	struct LightVertex {
		idVec3 direction;	//24.24.24
		float range;		//24

		idVec3 color;		//24.24.24
		float intensity;	//24

		idVec3 position;	//24.24.24
		float innerConeCos;	//24

		float outerConeCos;	//24
		float type;			//24
		float pad1;			//24p
		float pad2;			//24p

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
	bgfx::DynamicVertexBufferHandle lightData = BGFX_INVALID_HANDLE;
	PointLightVertex pointLightvertex;
	LightVertex lightVertex;
	gltfData* sceneData;
	int lightCount;
	idList<gltfExt_KHR_lights_punctual *> * lightList; 
	bool dirty;
};
