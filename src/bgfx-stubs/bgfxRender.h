#pragma once
#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"
#include "imgui.h"
#include "SDL.h"
#include "SDL_syswm.h"
#include "idFramework/sys/platform.h"

struct bgfxContext_t {
    SDL_Window *window = nullptr;
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;

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


void bgfxShutdown( bgfxContext_t *context);
void bgfxInitShaders( bgfxContext_t *context );
void bgfxRender( bgfxContext_t* context );