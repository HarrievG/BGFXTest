#include "SDL.h"
#include "SDL_syswm.h"
#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"
#include "imgui.h"
#include "sdl-imgui/imgui_impl_sdl.h"
#include "ImGuizmo.h"
#include "gltf-edit\gltfEditor.h"

#include "idFramework/sys/platform.h"
#include "idFramework/sys/sys_local.h"
#include "stubs/common_stubs.hpp"
#include "stubs/sys_stubs.hpp"
//#include "TypeInfoGen.h"
#include "idFramework/Common.h"
#include "idFramework/Session.h"
#include "idFramework/EventLoop.h"
#include "idFramework/FileSystem.h"
#include "idFramework/Licensee.h"
#include "idlib/containers/StrList.h"

idSession *session = NULL;
//idDeclManager *		declManager = NULL;
//idEventLoop *		eventLoop = NULL;

//int idEventLoop::JournalLevel( void ) const { return 0; }


idCVar com_developer( "developer", "0", CVAR_BOOL | CVAR_SYSTEM, "developer mode" );

idCommonLocal		commonLocal;
idCommon *common = &commonLocal;


idSysLocal		sysLocal;
idSys *sys = &sysLocal;

gltfSceneEditor gEditor;
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

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

    ImGui_Implbgfx_NewFrame( );
    ImGui_ImplSDL2_NewFrame( context->window );

    ImGui::NewFrame( );
    //ImGuizmo::ViewManipulate( )
    gEditor.DrawUI( );
    ImGui::ShowDemoWindow( ); // your drawing here
    ImGui::Render( );
    gEditor.Render();

    ImGui_Implbgfx_RenderDrawLists( ImGui::GetDrawData( ) );


    // simple input code for orbit camera
    int mouse_x, mouse_y;
    const int buttons = SDL_GetMouseState( &mouse_x, &mouse_y );
    if ( ( buttons & SDL_BUTTON( SDL_BUTTON_LEFT ) ) != 0 ) {
        int delta_x = mouse_x - context->prev_mouse_x;
        int delta_y = mouse_y - context->prev_mouse_y;
        context->cam_yaw += float( -delta_x ) * context->rot_scale;
        context->cam_pitch += float( -delta_y ) * context->rot_scale;
    }

    context->prev_mouse_x = mouse_x;
    context->prev_mouse_y = mouse_y;

    float cam_rotation[16];
    bx::mtxRotateXYZ( cam_rotation, context->cam_pitch, context->cam_yaw, 0.0f );

    float cam_translation[16];
    bx::mtxTranslate( cam_translation, 0.0f, 0.0f, -5.0f );

    float cam_transform[16];
    bx::mtxMul( cam_transform, cam_translation, cam_rotation );

    float view[16];
    bx::mtxInverse( view, cam_transform );

    float proj[16];
    bx::mtxProj(
        proj, 60.0f, float( context->width ) / float( context->height ), 0.1f,
        100.0f, bgfx::getCaps( )->homogeneousDepth );

    bgfx::setViewTransform( 0, view, proj );

    float model[16];
    bx::mtxIdentity( model );
    bgfx::setTransform( model );

    //bgfx::setVertexBuffer( 0, context->vbh );
    //bgfx::setIndexBuffer( context->ibh );

    //bgfx::submit( 0, context->program );

    bgfx::touch( 0 );//not really drawing so Uniforms and draw state will be applied 
    bgfx::frame( );

#if BX_PLATFORM_EMSCRIPTEN
    if ( context->quit ) {
        emscripten_cancel_main_loop( );
    }
#endif
}

int main( int argc, char **argv )
{
    idStr fileName, sourcePath;

    idLib::common = common;
    idLib::cvarSystem = cvarSystem;
    idLib::fileSystem = fileSystem;
    idLib::sys = sys;

    idLib::Init( );
    cvarSystem->Init( );
    idCVar::RegisterStaticVars( );
    cmdSystem->Init( );
    common->Init( argc, argv );


    fileSystem->Init( );
    fileName.Clear( );
    sourcePath.Clear( );

    fileSystem->Shutdown( false );
    cvarSystem->Shutdown( );
    cmdSystem->Shutdown( );
    idLib::ShutDown( );

    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize. SDL_Error: %s\n", SDL_GetError( ) );
        return 1;
    }

    const int width = WINDOW_WIDTH;
    const int height = WINDOW_HEIGHT;
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

    bgfx::PlatformData pd{};
#if BX_PLATFORM_WINDOWS
    pd.nwh = wmi.info.win.window;
#elif BX_PLATFORM_OSX
    pd.nwh = wmi.info.cocoa.window;
#elif BX_PLATFORM_LINUX
    pd.ndt = wmi.info.x11.display;
    pd.nwh = ( void * ) ( uintptr_t ) wmi.info.x11.window;
#elif BX_PLATFORM_EMSCRIPTEN
    pd.nwh = ( void * ) "#canvas";
#endif // BX_PLATFORM_WINDOWS ? BX_PLATFORM_OSX ? BX_PLATFORM_LINUX ?
        // BX_PLATFORM_EMSCRIPTEN

    bgfx::Init bgfx_init;
    bgfx_init.type = bgfx::RendererType::Count; // auto choose renderer
    bgfx_init.resolution.width = width;
    bgfx_init.resolution.height = height;
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
    bgfx_init.platformData = pd;
    bgfx::init( bgfx_init );

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0 );
    
    bgfx::setViewRect( 0, 0, 0, width, height );

    ImGui::CreateContext( );
    ImGuiIO &io = ImGui::GetIO( );

    ImGui_Implbgfx_Init( 255 );
#if BX_PLATFORM_WINDOWS
    ImGui_ImplSDL2_InitForD3D( window );
#elif BX_PLATFORM_OSX
    ImGui_ImplSDL2_InitForMetal( window );
#elif BX_PLATFORM_LINUX || BX_PLATFORM_EMSCRIPTEN
    ImGui_ImplSDL2_InitForOpenGL( window, nullptr );
#endif // BX_PLATFORM_WINDOWS ? BX_PLATFORM_OSX ? BX_PLATFORM_LINUX ?
    // BX_PLATFORM_EMSCRIPTEN

    context_t context;
    context.width = width;
    context.height = height;
    //context.program = program;
    context.window = window;
    //context.vbh = vbh;
    //context.ibh = ibh;

#if BX_PLATFORM_EMSCRIPTEN
    emscripten_set_main_loop_arg( main_loop, &context, -1, 1 );
#else
    while ( !context.quit ) {
        main_loop( &context );
    }
#endif // BX_PLATFORM_EMSCRIPTEN

    //bgfx::destroy( vbh );
    //bgfx::destroy( ibh );
    //bgfx::destroy( program );

    ImGui_ImplSDL2_Shutdown( );
    ImGui_Implbgfx_Shutdown( );

    ImGui::DestroyContext( );
    bgfx::shutdown( );

    SDL_DestroyWindow( window );
    SDL_Quit( );
    return 0;
}