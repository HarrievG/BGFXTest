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

struct bgfxContext_t {
    SDL_Window *window = nullptr;
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle colorUniformHandle;
    bgfx::TextureHandle colorTextureHandle;
    float cam_pitch = 0.0f;
    float cam_yaw = 0.0f;
    float rot_scale = 0.01f;

    int prev_mouse_x = 0;
    int prev_mouse_y = 0;

    int width = 0;
    int height = 0;

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

    void addVertexHandle( const bgfx::VertexBufferHandle vbh )
    {
        if ( numVertexHandles < maxVertexHandles ) {
            vertexHandles[numVertexHandles++] = vbh;
        } else {
            common->FatalError( "Cannot add additional vertex handle to this mesh." );
        }
    }

    void setBuffers( ) const         {
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
    int TransparencyMode;
};
typedef idList<bgfxMaterial> materialList;  

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

void bgfxShutdown( bgfxContext_t *context);
void bgfxInitShaders( bgfxContext_t *context );
void bgfxRender( bgfxContext_t* context );

bgfxModel loadGltfModel( const idStr &fileName );
