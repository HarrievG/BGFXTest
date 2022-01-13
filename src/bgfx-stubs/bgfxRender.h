#pragma once
#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"
#include "imgui.h"
#include "SDL.h"
#include "SDL_syswm.h"
#include "idFramework/sys/platform.h"
#include "idFramework/idlib/bv/Box.h"
#include "idFramework/idlib/bv/Bounds.h"
#include "idFramework/idlib/containers/List.h" 
#include "common.h"
#include <bx/rng.h>


extern idCVar		r_SceneEditRenderWidth;
extern idCVar		r_SceneEditRenderHeight;

// all drawing is done to a 640 x 480 virtual screen size
// and will be automatically scaled to the real resolution
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

struct bgfxMrtContext_t{
    bgfx::TextureHandle fbTextureHandles[2] = { BGFX_INVALID_HANDLE, BGFX_INVALID_HANDLE };
    bgfx::FrameBufferHandle fbh = BGFX_INVALID_HANDLE;;
    bgfx::TextureHandle rb = BGFX_INVALID_HANDLE;;

    bgfx::UniformHandle colorUniformHandle = BGFX_INVALID_HANDLE;;

    int width = 0;
    int height = 0;
    bgfx::ViewId viewId;
};

struct bgfxContext_t {
    SDL_Window *window = nullptr;
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle colorUniformHandle;
    bgfx::TextureHandle fbTextureHandle[2];
    bgfx::TextureHandle rb;
    bgfx::FrameBufferHandle fbh;
    uint32_t Bgra8;

    float cam_pitch = 0.0f;
    float cam_yaw = 0.0f;
    float rot_scale = 0.01f;

    int prev_mouse_x = 0;
    int prev_mouse_y = 0;

    int width = 0;
    int height = 0;

    // sized from 0 to SCREEN_WIDTH / SCREEN_HEIGHT (640/480), not actual resolution
    int		x, y;
    float	fov_x, fov_y;
    idVec3	vieworg;
    idMat3	viewaxis;			// transformation matrix, view looks down the positive X axis

    idMat4 cameraView;
    idMat4 cameraProjection;
    idMat4 objMat;
    bx::RngMwc rng;

    bool quit = false;
};

struct PosColorVertex {
    float x;
    float y;
    float z;
    uint32_t abgr;
};

namespace bgfx {
    struct CallbackStub;
    extern CallbackStub bgfxCallbacksLocal;
}

struct bgfxMesh     {
    // Different handle for each "stream" of vertex attributes
    // 0 - Position
    // 1 - Normal
    // 2 - Tangent
    // 3 - TexCoord0
    bgfx::VertexBufferHandle vertexHandles[4] = {
        BGFX_INVALID_HANDLE,
        BGFX_INVALID_HANDLE,
        BGFX_INVALID_HANDLE,
        BGFX_INVALID_HANDLE,
    };
    bgfx::IndexBufferHandle indexHandle = BGFX_INVALID_HANDLE;
    uint8_t numVertexHandles = 0;

    static const uint8_t maxVertexHandles = 4;

    void addVertexHandle( const bgfx::VertexBufferHandle vbh ) {
        if ( numVertexHandles < maxVertexHandles ) {
            vertexHandles[numVertexHandles++] = vbh;
        } else {
            common->FatalError( "Cannot add additional vertex handle to this mesh." );
        }
    }

    void setBuffers( ) const {
        bgfx::setIndexBuffer( indexHandle );
        for ( uint8_t j = 0; j < numVertexHandles; ++j ) {
            bgfx::setVertexBuffer( j, vertexHandles[j] );
        }
    }
};

enum struct TransparencyMode {
    OPAQUE_,
    MASKED,
    BLENDED,
};

// Struct containing material information according to the GLTF spec
// Note: Doesn't fully support the spec :)
struct PBRMaterial
{
    // Uniform Data
    idVec4 baseColorFactor = idVec4( 1.0f, 1.0f, 1.0f, 1.0f );
    idVec4 emissiveFactor = idVec4( 0.0f, 0.0f, 0.0f, 1.0f );
    float alphaCutoff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    bgfx::TextureHandle baseColorTexture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle metallicRoughnessTexture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle normalTexture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle emissiveTexture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle occlusionTexture = BGFX_INVALID_HANDLE;
};

struct bgfxMaterial {
    PBRMaterial material;
    TransparencyMode TransparencyMode;
};

struct MeshGroup
{
    idList<PBRMaterial> materials;
    idList<bgfxMesh> meshes;
    idList<idMat4> transforms;
    idList<idBounds> boundingBoxes;
};

struct bgfxTextureHandle {
    bgfx::TextureHandle handle;
    idVec2 dim;
};

struct bgfxModel
{
    idList<bgfxTextureHandle> textures;

    MeshGroup opaqueMeshes;
    MeshGroup maskedMeshes;
    MeshGroup transparentMeshes;

    idBounds boundingBox = {};
};

class bgfxRenderable
{
public:
    virtual ~bgfxRenderable() {};
    virtual bool Render(bgfxContext_t * context) = 0;
};

class imDrawable 
{
public:
    virtual ~imDrawable(){};
    virtual bool imDraw( bgfxContext_t *context ) = 0;
    virtual bool Show(bool visible) = 0;
    virtual bool isVisible( ) = 0;
};

typedef void ( *bgfxCallback )( bgfxContext_t *context );
void bgfxShutdown( bgfxContext_t *context);
void bgfxInitShaders( bgfxContext_t *context );
void bgfxRender( bgfxContext_t* context );
void bgfxRegisterCallback( bgfxCallback callback );
void bgfxCreateMrtTarget( bgfxMrtContext_t &context, const char *name );

bgfxModel loadGltfModel( const idStr &fileName );

idMat3 ConvertToIdSpace( const idMat3 &mat );
idVec3 ConvertToIdSpace( const idVec3 &pos );
idVec3 ConvertFromIdSpace( const idVec3 &idpos );
idMat3 ConvertFromIdSpace( const idMat3 &idmat );
idMat4 ConvertFromIdSpace( const idMat4 &idmat );
void myGlMultMatrix( const float a[16], const float b[16], float out[16] );