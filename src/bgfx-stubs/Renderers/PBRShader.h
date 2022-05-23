#pragma once

#include <bgfx/bgfx.h>
#include "idFramework/idlib/math/Vector.h"
#include "../../gltf-edit/gltfProperties.h"
#include "../bgfxSamplers.h"

class gltfMaterial;
class gltfData;
class PBRShader {
public:
	struct TextureTransformVertex {
		idVec2 offset;		//8

		idVec2 scale;		//8

		float rotation;		//4
		uint16 texCoord;	//2
		uint16 index;		//2

		static void init( );
		static bgfx::VertexLayout layout;
	};

	void initialize( );
	void shutdown( );

	void generateAlbedoLUT( );

	uint64_t bindMaterial( const gltfMaterial *material, gltfData *data  );
	void bindAlbedoLUT( bool compute = false );

	static constexpr float WHITE_FURNACE_RADIANCE = 1.0f;

	bool multipleScatteringEnabled = true;
	bool whiteFurnaceEnabled = false;

	void UpdateTextureTransforms( );
private:
	static constexpr uint16_t ALBEDO_LUT_SIZE = 32;
	static constexpr uint16_t ALBEDO_LUT_THREADS = 32;

	int16_t SetTextureTransform(gltfExt_KHR_texture_transform *transform,uint16_t &hasTransformMask , uint32_t mask);
	bgfx::UniformHandle baseColorFactorUniform = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle metallicRoughnessNormalOcclusionFactorUniform = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle emissiveFactorUniform = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle hasTexturesUniform = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle multipleScatteringUniform = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle albedoLUTSampler = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle baseColorSampler = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle metallicRoughnessSampler = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle normalSampler = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle occlusionSampler = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle emissiveSampler = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle fragmentOptionsUniform = BGFX_INVALID_HANDLE;

	bgfx::TextureHandle albedoLUTTexture = BGFX_INVALID_HANDLE;
	bgfx::TextureHandle defaultTexture = BGFX_INVALID_HANDLE;

	bgfx::ProgramHandle albedoLUTProgram = BGFX_INVALID_HANDLE;
	
	bgfx::DynamicVertexBufferHandle				textureTransformData = BGFX_INVALID_HANDLE;
	TextureTransformVertex						textureTransformVertex;
	idList<gltfExt_KHR_texture_transform *> 	textureTransformList; 
	bgfx::UniformHandle							textureTransformMask = BGFX_INVALID_HANDLE;
};
