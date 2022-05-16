#pragma once

#include <bgfx/bgfx.h>
#include "Renderers/PBRShader.h"
#include "Renderers/LightShader.h"
#include "idlib/math/Vector.h"
#include "idlib/math/Math.h"
#include "idlib/math/Matrix.h"
#include "idlib/containers/StrList.h"
#include "..\gltf-edit\gltfProperties.h"

class idMat4;
class idVec3;
class idVec4;
class idStr;

class Renderer
{
public:
	Renderer(gltfData* sceneData);
	virtual ~Renderer() { }

	void initialize();
	void reset(uint16_t width, uint16_t height);
	void render(float dt);
	void shutdown();

	void setVariable(const idStr& name, const idStr& val);

	enum class TonemappingMode : int
	{
		NONE = 0,
		EXPONENTIAL,
		REINHARD,
		REINHARD_LUM,
		HABLE,
		DUIKER,
		ACES,
		ACES_LUM
	};

	void setTonemappingMode(TonemappingMode mode);
	void setMultipleScattering(bool enabled);
	void setWhiteFurnace(bool enabled);

	static bool supported();
	static const char* shaderDir();

	// subclasses should override these

	// the first reset happens before initialize
	virtual void onInitialize() { }
	// window resize/flags changed (MSAA, V-Sync, ...)
	virtual void onReset() { }
	virtual void onRender(float dt) = 0;
	virtual void onShutdown() { }

	// buffers for debug output (display in the UI)

	struct TextureBuffer
	{
		bgfx::TextureHandle handle;
		const char* name;
	};

	TextureBuffer* buffers = nullptr;

	// final output
	// used for tonemapping
	bgfx::FrameBufferHandle frameBuffer = BGFX_INVALID_HANDLE;
	static bgfx::FrameBufferHandle createFrameBuffer(bool hdr = true, bool depth = true);
	void SetCamera(int cameraID ) { camId = cameraID; }
protected:
	struct PosVertex
	{
		float x;
		float y;
		float z;

		static void init()
		{
			layout.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
		}

		static bgfx::VertexLayout layout;
	};

	static constexpr bgfx::ViewId MAX_VIEW = 199; // imgui in bigg uses view 255

	void setViewProjection(bgfx::ViewId view);
	void setNormalMatrix(const idMat4& modelMat);
	void setSkinningMatrix( gltfSkin *skin,gltfAccessor * acc );
	void blitToScreen(bgfx::ViewId view = MAX_VIEW);

	static bgfx::TextureFormat::Enum findDepthFormat(unsigned long long textureFlags, bool stencil = false);

	bgfx::VertexLayout	vertexLayout;
	idStrList			variables;
	gltfData *			data = nullptr;
	uint16_t			width = 0;
	uint16_t			height = 0;
	PBRShader			pbr;	
	LightShader			lights;
	uint32_t			clearColor = 0;
	float				time = 0.0f;
	idMat4 viewMat	=	mat4_identity;
	idMat4 projMat	=	mat4_identity;
	idVec3 camPos	=	vec3_zero;
	int	camId		=	0;
	
	idDrawVert *	vtxData;
	int				vtxCount;

	triIndex_t *	idxData;
	int				idxCount;

	bgfx::VertexBufferHandle blitTriangleBuffer = BGFX_INVALID_HANDLE;
private:

	bgfx::ProgramHandle blitProgram = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle blitSampler = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle camPosUniform = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle normalMatrixUniform = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle exposureVecUniform = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle tonemappingModeVecUniform = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle boneMatricesUniform = BGFX_INVALID_HANDLE;

};
