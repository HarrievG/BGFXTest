#include "LightShader.h"

//#include "Scene/Scene.h"
#include "..\bgfxSamplers.h"
#include <cassert>
#include "..\..\gltf-edit\gltfProperties.h"

bgfx::VertexLayout LightShader::PointLightVertex::layout;


idCVar l_lightOverride_Index( "l_lightOverride_Index", "-1", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, "Index of the light to override. Set -1 to disable" );
idCVar l_lightOverride_Intensity( "l_lightOverride_Intensity", "0.0f", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_FLOAT, "Intensity override " );
idCVar l_lightOverride_Radius( "l_lightOverride_Radius", "0.0f", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_FLOAT, "Radius override " );

idCVar l_ambientLightIrradiance_override( "l_ambientLightIrradiance_override", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "Override Ambient" );
idCVar l_ambientLightIrradiance_R( "l_ambientLightIrradiance_R", "0.0f", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_FLOAT, "Ambient override " );
idCVar l_ambientLightIrradiance_G( "l_ambientLightIrradiance_G", "0.0f", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_FLOAT, "Ambient override " );
idCVar l_ambientLightIrradiance_B( "l_ambientLightIrradiance_B", "0.0f", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_FLOAT, "Ambient override " );

LightShader::LightShader( ) : sceneData(nullptr),lightCount(0),lightList(nullptr),dirty(true){

}

void LightShader::Initialize( gltfData * data ) {
	assert( data != nullptr );

	sceneData = data;
	lightCountVecUniform = bgfx::createUniform( "u_lightCountVec", bgfx::UniformType::Vec4 );
	ambientLightIrradianceUniform = bgfx::createUniform( "u_ambientLightIrradiance", bgfx::UniformType::Vec4 );

	pointLightvertex.init( );
	buffer = bgfx::createDynamicVertexBuffer(
		1, PointLightVertex::layout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE );

	lightCount = 0;
	auto &ext = sceneData->ExtensionsList( );
	for ( auto &it : ext ) {
		for ( auto &light : it->KHR_lights_punctual ) 
		{
			if (lightList == nullptr )
				lightList = &(it->KHR_lights_punctual);

			lightCount++;
		}
	}
}

void LightShader::Update( ) {

	size_t stride = PointLightVertex::layout.getStride();
	const bgfx::Memory *mem = bgfx::alloc( uint32_t( stride * Max( lightCount, 1 ) ) );

	for ( size_t i = 0; i < lightCount; i++ ) 	{
		PointLightVertex *light = ( PointLightVertex * ) ( mem->data + ( i * stride ) );
		light->position = vec3_zero;
		idMat4 mat = sceneData->GetLightMatrix(i);
		//mat[0][3] = 0.0;
		//mat[1][3] = 0.0;
		//mat[2][3] = 0.0;
		light->position *= mat;

		float intense = (*lightList)[i]->intensity * idMath::ONEFOURTH_PI;
		light->intensity = idVec3(intense,intense,intense);// lights[i].flux / ( 4.0f * glm::pi<float>( ) );

		const float INTENSITY_CUTOFF = 1.0f;
		const float ATTENTUATION_CUTOFF = 0.05f;
		float maxIntensity =0.0f;

		float attenuation = Max( INTENSITY_CUTOFF, ATTENTUATION_CUTOFF * intense ) / intense;
		light->radius = 1.0f / sqrtf( attenuation );

		if (l_lightOverride_Index.GetInteger() != -1 )
		{
			if (l_lightOverride_Intensity.GetFloat() > 0.0f )
				light->intensity = idVec3(l_lightOverride_Intensity.GetFloat(),l_lightOverride_Intensity.GetFloat(),l_lightOverride_Intensity.GetFloat());
			light->radius = l_lightOverride_Radius.GetFloat();
		}

	}

	bgfx::update( buffer, 0, mem );
	dirty = false;
}

void LightShader::Shutdown( ) {
	bgfx::destroy( lightCountVecUniform );
	bgfx::destroy( ambientLightIrradianceUniform );

	bgfx::destroy( buffer );
	buffer = BGFX_INVALID_HANDLE;

	lightCountVecUniform = ambientLightIrradianceUniform = BGFX_INVALID_HANDLE;
}

void LightShader::BindLights( ) {
	assert( sceneData != nullptr );

	if (dirty)
		Update();

	//check for light extension

	// a 32-bit IEEE 754 float can represent all integers up to 2^24 (~16.7 million) correctly
	// should be enough for this use case (comparison in for loop)
	float lightCountVec[4] = { ( float ) lightCount };
	bgfx::setUniform( lightCountVecUniform, lightCountVec );

	idVec4 ambientLightIrradiance(0.09f, 0.03f, 0.03f, 1.0f);

	if (l_ambientLightIrradiance_override.GetBool())
		ambientLightIrradiance = idVec4( l_ambientLightIrradiance_R.GetFloat(),l_ambientLightIrradiance_G.GetFloat() ,l_ambientLightIrradiance_B.GetFloat(),1.0f );

	bgfx::setUniform( ambientLightIrradianceUniform, &ambientLightIrradiance );

	bgfx::setBuffer( Samplers::LIGHTS_POINTLIGHTS, buffer, bgfx::Access::Read );
}


idVec3 LightShader::GetLightPos( gltfExt_KHR_lights_punctual *light ) {
	return vec3_zero;
}

void LightShader::PointLightVertex::init( ) {
	layout.begin( )
		.add( bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float )
		.add( bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float )
		.end( );
}
