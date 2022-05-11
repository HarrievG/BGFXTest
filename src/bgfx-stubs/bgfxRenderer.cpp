#include "bgfxRenderer.h"

//#include "Scene/Scene.h"
//#include <bx/macros.h>
#include <bx/string.h>
#include <bx/math.h>
#include <Common.h>
#include <bgfx-stubs/bgfxRender.h>

bgfx::VertexLayout Renderer::PosVertex::layout;

const char * toneMappingModeStr = "\n\
NONE\t\t= 0\n\
EXPONENTIAL\t= 1\n\
REINHARD\t= 2\n\
REINHARD_LUM\t= 3\n\
HABLE\t= 4\n\
DUIKER\t= 5\n\
ACES\t= 6\n\
ACES_LUM\t= 7\n";

idCVar r_tonemappingMode( "r_tonemappingMode", "1", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, toneMappingModeStr );

extern idCVar r_multipleScatteringEnabled;
extern idCVar r_whiteFurnaceEnabled;

Renderer::Renderer(gltfData* sceneData) : data(sceneData) { }

void Renderer::initialize()
{
	PosVertex::init();

	blitSampler = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
	camPosUniform = bgfx::createUniform("u_camPos", bgfx::UniformType::Vec4);
	normalMatrixUniform = bgfx::createUniform("u_normalMatrix", bgfx::UniformType::Mat4);
	exposureVecUniform = bgfx::createUniform("u_exposureVec", bgfx::UniformType::Vec4);
	tonemappingModeVecUniform = bgfx::createUniform("u_tonemappingModeVec", bgfx::UniformType::Vec4);

	// triangle used for blitting
	constexpr float BOTTOM = -1.0f, TOP = 3.0f, LEFT = -1.0f, RIGHT = 3.0f;
	const PosVertex vertices[3] = { { LEFT, BOTTOM, 0.0f }, { RIGHT, BOTTOM, 0.0f }, { LEFT, TOP, 0.0f } };
	blitTriangleBuffer = bgfx::createVertexBuffer(bgfx::copy(&vertices, sizeof(vertices)), PosVertex::layout);

	bgfx::ShaderHandle vsh		= bgfxCreateShader("shaders/vs_tonemap.bin","vshader" );
	bgfx::ShaderHandle fsh		= bgfxCreateShader( "shaders/fs_tonemap.bin", "fsshader" );
	blitProgram = bgfx::createProgram( vsh, fsh, true );

	pbr.initialize();
	pbr.generateAlbedoLUT();
	lights.Initialize(data);

	onInitialize();

	// finish any queued precomputations before rendering the scene
	bgfx::frame();
}

void Renderer::reset(uint16_t width, uint16_t height)
{
	if(!bgfx::isValid(frameBuffer))
	{
		frameBuffer = createFrameBuffer(true, true);
		bgfx::setName(frameBuffer, "Render framebuffer (pre-postprocessing)");
	}
	this->width = width;
	this->height = height;

	onReset();
}

void Renderer::render(float dt)
{
	time += dt;

	if ( r_multipleScatteringEnabled.IsModified())
	{
		r_multipleScatteringEnabled.ClearModified();
		setMultipleScattering(r_multipleScatteringEnabled.GetBool());
	}

	if ( r_whiteFurnaceEnabled.IsModified( ) ) 	{
		r_whiteFurnaceEnabled.ClearModified( );
		setWhiteFurnace( r_whiteFurnaceEnabled.GetBool( ) );
	}


	//if(scene->loaded)
	//{
		bgfx::setUniform(camPosUniform, &camPos);

	//	idVec3 linear = idVec3(PBRShader::WHITE_FURNACE_RADIANCE)
	//		//pbr.whiteFurnaceEnabled
	//		//? idVec3(PBRShader::WHITE_FURNACE_RADIANCE)
	//		//: glm::convertSRGBToLinear(scene->skyColor); // tonemapping expects linear colors
	//	glm::u8vec3 result = glm::u8vec3(glm::round(glm::clamp(linear, 0.0f, 1.0f) * 255.0f));
	//	clearColor = (result[0] << 24) | (result[1] << 16) | (result[2] << 8) | 255;
	//}
	//else
	//	clearColor = 0x303030FF; // gray

	onRender(dt);
	blitToScreen(MAX_VIEW);
	// bigg doesn't do this
	//bgfx::setViewName(MAX_VIEW + 1, "imgui");
}

void Renderer::shutdown()
{
	onShutdown();

	pbr.shutdown();
	lights.Shutdown();

	bgfx::destroy(blitProgram);
	bgfx::destroy(blitSampler);
	bgfx::destroy(camPosUniform);
	bgfx::destroy(normalMatrixUniform);
	bgfx::destroy(exposureVecUniform);
	bgfx::destroy(tonemappingModeVecUniform);
	bgfx::destroy(blitTriangleBuffer);
	if(bgfx::isValid(frameBuffer))
		bgfx::destroy(frameBuffer);

	blitProgram = BGFX_INVALID_HANDLE;
	blitSampler = camPosUniform = normalMatrixUniform = exposureVecUniform = tonemappingModeVecUniform =
		BGFX_INVALID_HANDLE;
	blitTriangleBuffer = BGFX_INVALID_HANDLE;
	frameBuffer = BGFX_INVALID_HANDLE;
}

void Renderer::setVariable(const idStr& name, const idStr& val)
{
	variables[idStr::Hash(name)] = val;
}

void Renderer::setTonemappingMode(TonemappingMode mode)
{
	r_tonemappingMode.SetInteger((int)mode);
}

void Renderer::setMultipleScattering(bool enabled)
{
	pbr.multipleScatteringEnabled = enabled;
}

void Renderer::setWhiteFurnace(bool enabled)
{
	pbr.whiteFurnaceEnabled = enabled;
}

bool Renderer::supported()
{
	const bgfx::Caps* caps = bgfx::getCaps();
	return
		// SDR color attachment
		(caps->formats[bgfx::TextureFormat::BGRA8] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) != 0 &&
		// HDR color attachment
		(caps->formats[bgfx::TextureFormat::RGBA16F] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) != 0;
}

void Renderer::setViewProjection(bgfx::ViewId view)
{
	idMat4 curTrans = data->GetViewMatrix( camId );
	idVec3 idup = idVec3( curTrans[0][1], curTrans[1][1], curTrans[2][1] );
	idVec3 forward = idVec3( -curTrans[0][2], -curTrans[1][2], -curTrans[2][2] );
	idVec3 pos = idVec3( curTrans[0][3], curTrans[1][3], curTrans[2][3] );
	camPos = pos;

	bx::Vec3 at = bx::Vec3( pos.x + forward.x, pos.y + forward.y, pos.z + forward.z );
	bx::Vec3 eye = bx::Vec3( pos.x, pos.y, pos.z );
	bx::Vec3 up = bx::Vec3( idup.x, idup.y, idup.z );
	bx::mtxLookAt( viewMat.ToFloatPtr( ), eye, at, up, bx::Handness::Right );

	if ( data->cameraManager->HasOverideID(camId) )
		camId = data->cameraManager->GetOverride(camId).newCameraID;

	gltfCamera_Perspective &sceneCam = data->CameraList( )[camId]->perspective;
	bx::mtxProj( projMat.ToFloatPtr( ), RAD2DEG( sceneCam.yfov ), sceneCam.aspectRatio, sceneCam.znear, sceneCam.zfar, bgfx::getCaps( )->homogeneousDepth, bx::Handness::Right );
	bgfx::setViewTransform( view, viewMat.ToFloatPtr( ), projMat.ToFloatPtr( ) );

}

void Renderer::setNormalMatrix(const idMat4& modelMat)
{
	// usually the normal matrix is based on the model view matrix
	// but shading is done in world space (not eye space) so it's just the model matrix
	idMat4 modelViewMat = viewMat * modelMat;

	// if we don't do non-uniform scaling, the normal matrix is the same as the model-view matrix
	// (only the magnitude of the normal is changed, but we normalize either way)
	//glm::mat3 normalMat = glm::mat3(modelMat);

	// use adjugate instead of inverse
	// see https://github.com/graphitemaster/normals_revisited#the-details-of-transforming-normals
	// cofactor is the transpose of the adjugate
	//idMat
	//glm::mat3 normalMat = glm::transpose( glm::adjugate( glm::mat3( modelMat ) ) );
	//common->DWarning(" Matrix adjugate!" );
	//idMat4 normalMat = modelMat.Inverse();//.Transpose();
	//idMat4 normalMat = modelViewMat;

	bgfx::setUniform(normalMatrixUniform, modelViewMat.ToFloatPtr());
}

void Renderer::blitToScreen(bgfx::ViewId view)
{
	bgfx::setViewName(view, "Tonemapping");
	bgfx::setViewClear(view, BGFX_CLEAR_NONE);
	bgfx::setViewRect(view, 0, 0, width, height);
	bgfx::setViewFrameBuffer(view, BGFX_INVALID_HANDLE);
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA );
	bgfx::TextureHandle frameBufferTexture = bgfx::getTexture(frameBuffer, 0);
	bgfx::setTexture(0, blitSampler, frameBufferTexture);
	float exposureVec[4] = { 1.0f };
	bgfx::setUniform(exposureVecUniform, exposureVec);
	float tonemappingModeVec[4] = { (float)r_tonemappingMode.GetInteger() };
	bgfx::setUniform(tonemappingModeVecUniform, tonemappingModeVec);
	bgfx::setVertexBuffer(0, blitTriangleBuffer);
	bgfx::submit(view, blitProgram);
}

bgfx::TextureFormat::Enum Renderer::findDepthFormat(unsigned long long  textureFlags, bool stencil)
{
	const bgfx::TextureFormat::Enum depthFormats[] = { bgfx::TextureFormat::D16, bgfx::TextureFormat::D32 };

	const bgfx::TextureFormat::Enum depthStencilFormats[] = { bgfx::TextureFormat::D24S8 };

	const bgfx::TextureFormat::Enum* formats = stencil ? depthStencilFormats : depthFormats;
	size_t count = stencil ? BX_COUNTOF(depthStencilFormats) : BX_COUNTOF(depthFormats);

	bgfx::TextureFormat::Enum depthFormat = bgfx::TextureFormat::Count;
	for(size_t i = 0; i < count; i++)
	{
		if(bgfx::isTextureValid(0, false, 1, formats[i], textureFlags))
		{
			depthFormat = formats[i];
			break;
		}
	}

	assert(depthFormat != bgfx::TextureFormat::Enum::Count);

	return depthFormat;
}

bgfx::FrameBufferHandle Renderer::createFrameBuffer(bool hdr, bool depth)
{
	bgfx::TextureHandle textures[2];
	uint8_t attachments = 0;

	const uint64_t samplerFlags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT |
		BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

	bgfx::TextureFormat::Enum format =
		hdr ? bgfx::TextureFormat::RGBA16F : bgfx::TextureFormat::BGRA8; // BGRA is often faster (internal GPU format)
	assert(bgfx::isTextureValid(0, false, 1, format, BGFX_TEXTURE_RT | samplerFlags));
	textures[attachments++] =
		bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, format, BGFX_TEXTURE_RT | samplerFlags);

	if(depth)
	{
		bgfx::TextureFormat::Enum depthFormat = findDepthFormat(BGFX_TEXTURE_RT_WRITE_ONLY | samplerFlags);
		assert(depthFormat != bgfx::TextureFormat::Enum::Count);
		textures[attachments++] = bgfx::createTexture2D(
			bgfx::BackbufferRatio::Equal, false, 1, depthFormat, BGFX_TEXTURE_RT_WRITE_ONLY | samplerFlags);
	}

	bgfx::FrameBufferHandle fb = bgfx::createFrameBuffer(attachments, textures, true);

	if(!bgfx::isValid(fb))
		common->FatalError("Failed to create framebuffer");

	return fb;
}

const char* Renderer::shaderDir()
{
	const char* path = "???";

	switch(bgfx::getRendererType())
	{
	case bgfx::RendererType::Noop:
	case bgfx::RendererType::Direct3D9:
		path = "shaders/dx9/";
		break;
	case bgfx::RendererType::Direct3D11:
	case bgfx::RendererType::Direct3D12:
		path = "shaders/dx11/";
		break;
	case bgfx::RendererType::Gnm:
		break;
	case bgfx::RendererType::Metal:
		path = "shaders/metal/";
		break;
	case bgfx::RendererType::OpenGL:
		path = "shaders/glsl/";
		break;
	case bgfx::RendererType::OpenGLES:
		path = "shaders/essl/";
		break;
	case bgfx::RendererType::Vulkan:
		path = "shaders/spirv/";
		break;
	default:
		assert(false);
		break;
	}

	return path;
}
