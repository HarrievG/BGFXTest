#include "PBRShader.h"

#include "..\gltf-edit\gltfProperties.h"
#include "..\bgfxRenderer.h"
#include "..\bgfxSamplers.h"
#include <bx/string.h>
#include <bimg/encode.h>
#include <bx/file.h>

void PBRShader::initialize( ) {
	baseColorFactorUniform =
		bgfx::createUniform( "u_baseColorFactor", bgfx::UniformType::Vec4 );
	metallicRoughnessNormalOcclusionFactorUniform =
		bgfx::createUniform( "u_metallicRoughnessNormalOcclusionFactor", bgfx::UniformType::Vec4 );
	emissiveFactorUniform =
		bgfx::createUniform( "u_emissiveFactorVec", bgfx::UniformType::Vec4 );
	hasTexturesUniform =
		bgfx::createUniform( "u_hasTextures", bgfx::UniformType::Vec4 );
	multipleScatteringUniform =
		bgfx::createUniform( "u_multipleScatteringVec", bgfx::UniformType::Vec4 );
	albedoLUTSampler =
		bgfx::createUniform( "s_texAlbedoLUT", bgfx::UniformType::Sampler );
	baseColorSampler =
		bgfx::createUniform( "s_texBaseColor", bgfx::UniformType::Sampler );
	metallicRoughnessSampler =
		bgfx::createUniform( "s_texMetallicRoughness", bgfx::UniformType::Sampler );
	normalSampler =
		bgfx::createUniform( "s_texNormal", bgfx::UniformType::Sampler );
	occlusionSampler =
		bgfx::createUniform( "s_texOcclusion", bgfx::UniformType::Sampler );
	emissiveSampler =
		bgfx::createUniform( "s_texEmissive", bgfx::UniformType::Sampler );

	defaultTexture = bgfx::createTexture2D( 1, 1, false, 1, bgfx::TextureFormat::RGBA8 );
	albedoLUTTexture = bgfx::createTexture2D( ALBEDO_LUT_SIZE,
		ALBEDO_LUT_SIZE,
		false,
		1,
		bgfx::TextureFormat::RGBA32F,
		BGFX_SAMPLER_UVW_CLAMP | BGFX_TEXTURE_COMPUTE_WRITE );

	char csName[128];

	bgfx::ShaderHandle csh = bgfxCreateShader( "shaders/cs_multiple_scattering_lut.bin", "cs_multiple_scattering_lut" );
	albedoLUTProgram = bgfx::createProgram( csh, true );
}

void PBRShader::shutdown( ) {
	bgfx::destroy( baseColorFactorUniform );
	bgfx::destroy( metallicRoughnessNormalOcclusionFactorUniform );
	bgfx::destroy( emissiveFactorUniform );
	bgfx::destroy( hasTexturesUniform );
	bgfx::destroy( multipleScatteringUniform );
	bgfx::destroy( albedoLUTSampler );
	bgfx::destroy( baseColorSampler );
	bgfx::destroy( metallicRoughnessSampler );
	bgfx::destroy( normalSampler );
	bgfx::destroy( occlusionSampler );
	bgfx::destroy( emissiveSampler );
	bgfx::destroy( albedoLUTTexture );
	bgfx::destroy( defaultTexture );
	bgfx::destroy( albedoLUTProgram );

	baseColorFactorUniform = metallicRoughnessNormalOcclusionFactorUniform = emissiveFactorUniform =
		hasTexturesUniform = multipleScatteringUniform = albedoLUTSampler = baseColorSampler =
		metallicRoughnessSampler = normalSampler = occlusionSampler = emissiveSampler = BGFX_INVALID_HANDLE;
	albedoLUTTexture = defaultTexture = BGFX_INVALID_HANDLE;
	albedoLUTProgram = BGFX_INVALID_HANDLE;
}

void PBRShader::generateAlbedoLUT( ) {
	bindAlbedoLUT( true /* compute */ );
	bgfx::dispatch( 0, albedoLUTProgram, ALBEDO_LUT_SIZE / ALBEDO_LUT_THREADS, ALBEDO_LUT_SIZE / ALBEDO_LUT_THREADS, 1 );
}


uint64_t PBRShader::bindMaterial( const gltfMaterial *material, gltfData *data ) {

	const gltfMaterial_pbrMetallicRoughness &pbrMR = material->pbrMetallicRoughness;
	float factorValues[4] = {
		pbrMR.metallicFactor, pbrMR.roughnessFactor, material->normalTexture.scale, material->occlusionTexture.strength
	};
	bgfx::setUniform( baseColorFactorUniform, &pbrMR.baseColorFactor );
	bgfx::setUniform( metallicRoughnessNormalOcclusionFactorUniform, factorValues );
	bgfx::setUniform( emissiveFactorUniform, &material->emissiveFactor );

	float hasTexturesValues[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	auto &matList = data->MaterialList( );
	auto &texList = data->TextureList( );
	auto &imgList = data->ImageList( );
	auto &smpList = data->SamplerList( );

	uint32_t hasTexturesMask = 0;

	if ( material->pbrMetallicRoughness.baseColorTexture.index != -1 ) {
		gltfTexture *texture = texList[material->pbrMetallicRoughness.baseColorTexture.index];
		gltfSampler *sampler = smpList[texture->sampler];
		gltfImage *image = imgList[texture->source];

		bgfx::setTexture( Samplers::PBR_BASECOLOR, baseColorSampler, image->bgfxTexture.handle );
		hasTexturesMask |= 1 << 0 ;
	}else
		bgfx::setTexture( Samplers::PBR_BASECOLOR, baseColorSampler, defaultTexture );
	if ( material->pbrMetallicRoughness.metallicRoughnessTexture.index != -1 ) {
		gltfTexture *texture = texList[material->pbrMetallicRoughness.metallicRoughnessTexture.index];
		gltfSampler *sampler = smpList[texture->sampler];
		gltfImage *image = imgList[texture->source];

		bgfx::setTexture( Samplers::PBR_METALROUGHNESS, metallicRoughnessSampler, image->bgfxTexture.handle );
		hasTexturesMask |= 1 << 1;
	}else
		bgfx::setTexture( Samplers::PBR_METALROUGHNESS, metallicRoughnessSampler, defaultTexture );
	if ( material->normalTexture.index != -1 ) {
		gltfTexture *texture = texList[material->normalTexture.index];
		gltfSampler *sampler = smpList[texture->sampler];
		gltfImage *image = imgList[texture->source];

		bgfx::setTexture( Samplers::PBR_NORMAL, normalSampler, image->bgfxTexture.handle );
		hasTexturesMask |= 1 << 2;
	}else
		bgfx::setTexture( Samplers::PBR_NORMAL, normalSampler, defaultTexture );
	if ( material->occlusionTexture.index != -1 ) {
		gltfTexture *texture = texList[material->occlusionTexture.index];
		gltfSampler *sampler = smpList[texture->sampler];
		gltfImage *image = imgList[texture->source];

		bgfx::setTexture( Samplers::PBR_OCCLUSION, occlusionSampler, image->bgfxTexture.handle );
		hasTexturesMask |= 1 << 3;
	}else
		bgfx::setTexture( Samplers::PBR_OCCLUSION, occlusionSampler, defaultTexture );
	if ( material->emissiveTexture.index != -1 ) {
		gltfTexture *texture = texList[material->emissiveTexture.index];
		gltfSampler *sampler = smpList[texture->sampler];
		gltfImage *image = imgList[texture->source];

		bgfx::setTexture( Samplers::PBR_EMISSIVE, emissiveSampler, image->bgfxTexture.handle );
		hasTexturesMask |= 1 << 4;
	}else
		bgfx::setTexture( Samplers::PBR_EMISSIVE, emissiveSampler, defaultTexture );

	//const uint32_t hasTexturesMask = 0
	//	| ( ( setTextureOrDefault( Samplers::PBR_BASECOLOR, baseColorSampler, bgfxmaterial.baseColorTexture ) ? 1 : 0 ) << 0 )
	//	| ( ( setTextureOrDefault( Samplers::PBR_METALROUGHNESS, metallicRoughnessSampler, bgfxmaterial.metallicRoughnessTexture ) ? 1 : 0 ) << 1 )
	//	| ( ( setTextureOrDefault( Samplers::PBR_NORMAL, normalSampler, bgfxmaterial.normalTexture ) ? 1 : 0 ) << 2 )
	//	| ( ( setTextureOrDefault( Samplers::PBR_OCCLUSION, occlusionSampler, bgfxmaterial.occlusionTexture ) ? 1 : 0 ) << 3 )
	//	| ( ( setTextureOrDefault( Samplers::PBR_EMISSIVE, emissiveSampler, bgfxmaterial.emissiveTexture ) ? 1 : 0 ) << 4 );
	hasTexturesValues[0] = static_cast< float >( hasTexturesMask );

	bgfx::setUniform( hasTexturesUniform, hasTexturesValues );

	float multipleScatteringValues[4] = {
		multipleScatteringEnabled ? 1.0f : 0.0f, whiteFurnaceEnabled ? WHITE_FURNACE_RADIANCE : 0.0f, 0.0f, 0.0f
	};
	bgfx::setUniform( multipleScatteringUniform, multipleScatteringValues );

	uint64_t state = 0;
	if ( material->bgfxMaterial.TransparencyMode == TransparencyMode::BLENDED )
		state |= BGFX_STATE_BLEND_ALPHA;
	if ( !material->doubleSided )
		state |= BGFX_STATE_CULL_CW;
	return state;
}

void PBRShader::bindAlbedoLUT( bool compute ) {
	if ( compute )
		bgfx::setImage( Samplers::PBR_ALBEDO_LUT, albedoLUTTexture, 0, bgfx::Access::Write );
	else
		bgfx::setTexture( Samplers::PBR_ALBEDO_LUT, albedoLUTSampler, albedoLUTTexture );
}
