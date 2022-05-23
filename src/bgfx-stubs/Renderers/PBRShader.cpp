#include "PBRShader.h"

#include "..\gltf-edit\gltfProperties.h"
#include "..\bgfxRenderer.h"
#include "..\bgfxSamplers.h"
#include <bx/string.h>
#include <bimg/encode.h>
#include <bx/file.h>

idCVar r_multipleScatteringEnabled( "r_multipleScatteringEnabled", "1", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "1 to r_multipleScatteringEnabled" );
idCVar r_whiteFurnaceEnabled( "r_whiteFurnaceEnabled", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "1 to r_whiteFurnaceEnabled" );

idCVar r_pbrDebug( "r_pbrDebug", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "" );
idCVar r_pbrDebugDrawNormals( "r_pbrDebugDrawNormals", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "" );
idCVar r_pbrDebugDrawNormalsMat( "r_pbrDebugDrawNormalsMat", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "" );
idCVar r_pbrDebugDrawBaseColour( "r_pbrDebugDrawBaseColour", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "" );


bgfx::VertexLayout PBRShader::TextureTransformVertex::layout;

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
	fragmentOptionsUniform =
		bgfx::createUniform( "u_fragmentOptions", bgfx::UniformType::Vec4 );
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
	textureTransformMask =
		bgfx::createUniform( "u_texTransformMask", bgfx::UniformType::Vec4 );

	defaultTexture = bgfx::createTexture2D( 1, 1, false, 1, bgfx::TextureFormat::RGBA8 );
	albedoLUTTexture = bgfx::createTexture2D( ALBEDO_LUT_SIZE,
		ALBEDO_LUT_SIZE,
		false,
		1,
		bgfx::TextureFormat::RGBA32F,
		BGFX_SAMPLER_UVW_CLAMP | BGFX_TEXTURE_COMPUTE_WRITE );

	bgfx::ShaderHandle csh = bgfxCreateShader( "shaders/cs_multiple_scattering_lut.bin", "cs_multiple_scattering_lut" );
	albedoLUTProgram = bgfx::createProgram( csh, true );

	textureTransformVertex.init( );
	textureTransformData = bgfx::createDynamicVertexBuffer(
		1, TextureTransformVertex::layout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE );
}

void PBRShader::shutdown( ) {
	bgfx::destroy( baseColorFactorUniform );
	bgfx::destroy( metallicRoughnessNormalOcclusionFactorUniform );
	bgfx::destroy( emissiveFactorUniform );
	bgfx::destroy( hasTexturesUniform );
	bgfx::destroy( multipleScatteringUniform );
	bgfx::destroy( fragmentOptionsUniform );
	bgfx::destroy( albedoLUTSampler );
	bgfx::destroy( baseColorSampler );
	bgfx::destroy( metallicRoughnessSampler );
	bgfx::destroy( normalSampler );
	bgfx::destroy( occlusionSampler );
	bgfx::destroy( emissiveSampler );
	bgfx::destroy( albedoLUTTexture );
	bgfx::destroy( defaultTexture );
	bgfx::destroy( albedoLUTProgram );
	bgfx::destroy( textureTransformData );
	bgfx::destroy( textureTransformMask );
	
	textureTransformMask = fragmentOptionsUniform = 
		baseColorFactorUniform = metallicRoughnessNormalOcclusionFactorUniform = emissiveFactorUniform =
		hasTexturesUniform = multipleScatteringUniform = albedoLUTSampler = baseColorSampler =
		metallicRoughnessSampler = normalSampler = occlusionSampler = emissiveSampler = BGFX_INVALID_HANDLE;

	albedoLUTTexture = defaultTexture = BGFX_INVALID_HANDLE;
	albedoLUTProgram = BGFX_INVALID_HANDLE;
	textureTransformData = BGFX_INVALID_HANDLE;
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
	uint16_t hasTransformMask = 0;
	int16_t indices[Samplers::PBR_MAXTEXTRANS+1];

	gltfTexture *texture;
	gltfImage	*image;

	if ( material->pbrMetallicRoughness.baseColorTexture.index != -1 ) {
		texture = texList[material->pbrMetallicRoughness.baseColorTexture.index];
		image = imgList[texture->source];

		uint32 maskedIdx = (Samplers::PBR_BASECOLOR-Samplers::PBR_MASKOFFSET);
		indices[maskedIdx] =
			SetTextureTransform(material->pbrMetallicRoughness.baseColorTexture.extensions.KHR_texture_transform,hasTransformMask,maskedIdx);

		bgfx::setTexture( Samplers::PBR_BASECOLOR, baseColorSampler, image->bgfxTexture.handle );
		hasTexturesMask |= 1 << maskedIdx;
		
	}else
		bgfx::setTexture( Samplers::PBR_BASECOLOR, baseColorSampler, defaultTexture );

	if ( material->pbrMetallicRoughness.metallicRoughnessTexture.index != -1 ) {
		texture = texList[material->pbrMetallicRoughness.metallicRoughnessTexture.index];
		image = imgList[texture->source];

		uint32 maskedIdx = (Samplers::PBR_METALROUGHNESS-Samplers::PBR_MASKOFFSET);
		indices[maskedIdx] = 
			SetTextureTransform(material->pbrMetallicRoughness.metallicRoughnessTexture.extensions.KHR_texture_transform,hasTransformMask,maskedIdx);

		bgfx::setTexture( Samplers::PBR_METALROUGHNESS, metallicRoughnessSampler, image->bgfxTexture.handle );
		hasTexturesMask |= 1 << maskedIdx;

	}else
		bgfx::setTexture( Samplers::PBR_METALROUGHNESS, metallicRoughnessSampler, defaultTexture );

	if ( material->normalTexture.index != -1 ) {
		texture = texList[material->normalTexture.index];
		image = imgList[texture->source];

		uint32 maskedIdx = (Samplers::PBR_NORMAL-Samplers::PBR_MASKOFFSET);
		indices[maskedIdx] = 
			SetTextureTransform( material->normalTexture.extensions.KHR_texture_transform, hasTransformMask, maskedIdx );

		bgfx::setTexture( Samplers::PBR_NORMAL, normalSampler, image->bgfxTexture.handle );
		hasTexturesMask |= 1 << maskedIdx;
	}else
		bgfx::setTexture( Samplers::PBR_NORMAL, normalSampler, defaultTexture );

	if ( material->occlusionTexture.index != -1 ) {
		texture = texList[material->occlusionTexture.index];
		image = imgList[texture->source];
		
		uint32 maskedIdx = (Samplers::PBR_OCCLUSION-Samplers::PBR_MASKOFFSET);
		indices[maskedIdx] = 
			SetTextureTransform( material->occlusionTexture.extensions.KHR_texture_transform, hasTransformMask, maskedIdx );

		bgfx::setTexture( Samplers::PBR_OCCLUSION, occlusionSampler, image->bgfxTexture.handle );
		hasTexturesMask |= 1 << maskedIdx;
	}else
		bgfx::setTexture( Samplers::PBR_OCCLUSION, occlusionSampler, defaultTexture );

	if ( material->emissiveTexture.index != -1 ) {
		texture = texList[material->emissiveTexture.index];
		image = imgList[texture->source];

		uint32 maskedIdx = ( Samplers::PBR_EMISSIVE - Samplers::PBR_MASKOFFSET );
		indices[maskedIdx] =
			SetTextureTransform( material->occlusionTexture.extensions.KHR_texture_transform, hasTransformMask, maskedIdx );

		bgfx::setTexture( Samplers::PBR_EMISSIVE, emissiveSampler, image->bgfxTexture.handle );
		hasTexturesMask |= 1 << maskedIdx;;
	}else
		bgfx::setTexture( Samplers::PBR_EMISSIVE, emissiveSampler, defaultTexture );

	uint64_t state = 0;
	if ( material->intType == gltfMaterial::gltfAlphaMode::gltfBLEND )
		state |= BGFX_STATE_BLEND_ALPHA;
	else if ( material->intType == gltfMaterial::gltfAlphaMode::gltfMASK )
		hasTexturesValues[3] = material->alphaCutoff + 1.0f;

	if ( !material->doubleSided )
		state |= BGFX_STATE_CULL_CW;

	hasTexturesValues[0] = static_cast< float >( hasTexturesMask );

	bgfx::setUniform( hasTexturesUniform, hasTexturesValues );

	float multipleScatteringValues[4] = {
		multipleScatteringEnabled ? 1.0f : 0.0f, whiteFurnaceEnabled ? WHITE_FURNACE_RADIANCE : 0.0f, 0.0f, 0.0f
	};
	bgfx::setUniform( multipleScatteringUniform, multipleScatteringValues );

	float fragmentOptions[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	uint32_t fragmentOptionsMask = 0;
	
	fragmentOptionsMask |= r_pbrDebug.GetBool()					? (1 << 0) : 0;
	fragmentOptionsMask |= r_pbrDebugDrawBaseColour.GetBool()	? (1 << 1) : 0;
	fragmentOptionsMask |= r_pbrDebugDrawNormals.GetBool()		? (1 << 2) : 0;
	fragmentOptionsMask |= r_pbrDebugDrawNormalsMat.GetBool()	? (1 << 3) : 0; 
	fragmentOptions[0] = static_cast< float >( fragmentOptionsMask );
	bgfx::setUniform( fragmentOptionsUniform, fragmentOptions );

	//mask to indicate which texture has an transform 
	indices[Samplers::PBR_MAXTEXTRANS] =  hasTransformMask;
	bgfx::setUniform( textureTransformMask,  (float*)&indices);

	return state;
}

void PBRShader::bindAlbedoLUT( bool compute ) {
	if ( compute )
		bgfx::setImage( Samplers::PBR_ALBEDO_LUT, albedoLUTTexture, 0, bgfx::Access::Write );
	else
		bgfx::setTexture( Samplers::PBR_ALBEDO_LUT, albedoLUTSampler, albedoLUTTexture );
}

void PBRShader::UpdateTextureTransforms( ) {

	int transformCount = textureTransformList.Num( );
	size_t stride = TextureTransformVertex::layout.getStride( );
	const bgfx::Memory *mem = bgfx::alloc( uint32_t( stride * Max( textureTransformList.Num( ), 1 ) ) );
	for ( size_t i = 0; i < transformCount; i++ ) {
		TextureTransformVertex *transform = ( TextureTransformVertex * ) ( mem->data + ( i * stride ) );
		auto &gltfTransform = ( textureTransformList )[i];

		transform->offset = gltfTransform->offset;
		transform->scale = gltfTransform->scale;
		transform->rotation = gltfTransform->rotation;
		
		transform->texCoord = gltfTransform->texCoord;
		transform->index = gltfTransform->index;
	}
	bgfx::update( textureTransformData, 0, mem );
	textureTransformList.Clear();
}


int16_t PBRShader::SetTextureTransform(gltfExt_KHR_texture_transform *transform,uint16_t &hasTransformMask , uint32_t mask) {
	if ( transform != nullptr ) {
		auto &texTrans = textureTransformList.Alloc( );
		texTrans = transform;
		texTrans->index = textureTransformList.Num( );
		hasTransformMask |= 1 << mask;
		return textureTransformList.Num( );
	}
	return -1;
}

void PBRShader::TextureTransformVertex::init( ) {
	//float2; float2; float; ushort; ushort;
	layout.begin( )
		.add( bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float )
		.add( bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float )
		.add( bgfx::Attrib::TexCoord2, 2, bgfx::AttribType::Float )
		.end( );
}
