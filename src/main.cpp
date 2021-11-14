#include "SDL.h"
#include "SDL_syswm.h"
#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"
#include "imgui.h"
#include "sdl-imgui/imgui_impl_sdl.h"

struct context_t {
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

void main_loop( void *data ) {
    auto context = static_cast< context_t * >( data );

    for ( SDL_Event currentEvent; SDL_PollEvent( &currentEvent ) != 0;) {
        ImGui_ImplSDL2_ProcessEvent( &currentEvent );
        if ( currentEvent.type == SDL_QUIT ) {
            context->quit = true;
            break;
        }
    }
//
//    ImGui_Implbgfx_NewFrame( );
//    ImGui_ImplSDL2_NewFrame( context->window );
//
//    ImGui::NewFrame( );
//    ImGui::ShowDemoWindow( ); // your drawing here
//    ImGui::Render( );
//    ImGui_Implbgfx_RenderDrawLists( ImGui::GetDrawData( ) );
//
//
//    // simple input code for orbit camera
//    int mouse_x, mouse_y;
//    const int buttons = SDL_GetMouseState( &mouse_x, &mouse_y );
//    if ( ( buttons & SDL_BUTTON( SDL_BUTTON_LEFT ) ) != 0 ) {
//        int delta_x = mouse_x - context->prev_mouse_x;
//        int delta_y = mouse_y - context->prev_mouse_y;
//        context->cam_yaw += float( -delta_x ) * context->rot_scale;
//        context->cam_pitch += float( -delta_y ) * context->rot_scale;
//    }
//
//    context->prev_mouse_x = mouse_x;
//    context->prev_mouse_y = mouse_y;
//
//    float cam_rotation[16];
//    bx::mtxRotateXYZ( cam_rotation, context->cam_pitch, context->cam_yaw, 0.0f );
//
//    float cam_translation[16];
//    bx::mtxTranslate( cam_translation, 0.0f, 0.0f, -5.0f );
//
//    float cam_transform[16];
//    bx::mtxMul( cam_transform, cam_translation, cam_rotation );
//
//    float view[16];
//    bx::mtxInverse( view, cam_transform );
//
//    float proj[16];
//    bx::mtxProj(
//        proj, 60.0f, float( context->width ) / float( context->height ), 0.1f,
//        100.0f, bgfx::getCaps( )->homogeneousDepth );
//
//    bgfx::setViewTransform( 0, view, proj );
//
//    float model[16];
//    bx::mtxIdentity( model );
//    bgfx::setTransform( model );
//
//    bgfx::setVertexBuffer( 0, context->vbh );
//    bgfx::setIndexBuffer( context->ibh );
//
//    bgfx::submit( 0, context->program );
//
//    bgfx::frame( );
//
//#if BX_PLATFORM_EMSCRIPTEN
//    if ( context->quit ) {
//        emscripten_cancel_main_loop( );
//    }
//#endif
}

int main( int argc, char **argv )
{
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize. SDL_Error: %s\n", SDL_GetError( ) );
        return 1;
    }


    const int width = 800;
    const int height = 600;
    SDL_Window *window = SDL_CreateWindow(
        argv[0], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width,
        height, SDL_WINDOW_SHOWN );

    if ( window == nullptr ) {
        printf( "Window could not be created. SDL_Error: %s\n", SDL_GetError( ) );
        return 1;
    }
 
#if !BX_PLATFORM_EMSCRIPTEN
    SDL_SysWMinfo wmi;
    SDL_VERSION( &wmi.version );
    if ( !SDL_GetWindowWMInfo( window, &wmi ) ) {
        printf(
            "SDL_SysWMinfo could not be retrieved. SDL_Error: %s\n",
            SDL_GetError( ) );
        return 1;
    }
    bgfx::renderFrame( ); // single threaded mode
#endif // !BX_PLATFORM_EMSCRIPTEN
    bgfx::init();
    return 0;
}