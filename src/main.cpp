#include "SDL.h"
#include "bgfx-stubs/bgfxRender.h"
#include "imgui.h"
#include "sdl-imgui/imgui_impl_sdl.h"
#include "ImGuizmo.h"
#include "gltf-edit\gltfEditor.h"

#include "idFramework/sys/platform.h"
#include "idFramework/sys/sys_local.h"
#include "stubs/sys_stubs.hpp"
#include "idFramework/Common.h"
#include "idFramework/Session.h"
#include "idFramework/EventLoop.h"
#include "idFramework/FileSystem.h"
#include "idFramework/Licensee.h"
#include "idlib/containers/StrList.h"
#include "idFramework/idImGui/idImConsole.h"
#include "idFramework/KeyInput.h"
#include "idFramework/idImGui/idImConsole.h"

//idDeclManager *		declManager = NULL;
//int idEventLoop::JournalLevel( void ) const { return 0; }

idCVar com_developer( "developer", "0", CVAR_BOOL | CVAR_SYSTEM, "developer mode" );
idCVar win_outputDebugString( "win_outputDebugString", "1", CVAR_SYSTEM | CVAR_BOOL, "Output to debugger " );
idCVar win_outputEditString( "win_outputEditString", "1", CVAR_SYSTEM | CVAR_BOOL, "" );
idCVar win_viewlog( "win_viewlog", "0", CVAR_SYSTEM | CVAR_INTEGER, "" );

idSession *session = NULL;

idSysLocal		sysLocal;
idSys *sys = &sysLocal;

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

void main_loop( void *data ) {
    auto context = static_cast< bgfxContext_t * >( data );
    
    ImGui_Implbgfx_NewFrame( );
    ImGui_ImplSDL2_NewFrame( context->window );
    common->Frame();
    ImGui::NewFrame( );

 
    //ImGuizmo::ViewManipulate( )
    ImGui::ShowDemoWindow( ); // your drawing here
	imConsole->Draw( );
    
    bgfxRender( context );

    ImGui::Render( );

    ImGui_Implbgfx_RenderDrawLists( ImGui::GetDrawData( ) );
    bgfx::frame( );
    
#if BX_PLATFORM_EMSCRIPTEN
    if ( context->quit ) {
        emscripten_cancel_main_loop( );
    }
#endif
}

int main( int argc, char **argv )
{
    static bgfxContext_t context;

    idLib::common = common;
    idLib::cvarSystem = cvarSystem;
    idLib::fileSystem = fileSystem;
    idLib::sys = sys;

    idLib::Init( );
	idCVar::RegisterStaticVars( );
    cvarSystem->Init( );
    cmdSystem->Init( );
    common->Init( argc, argv );
    fileSystem->Init( );
    eventLoop->Init();
    sceneEditor->Init( );

    //eventLoop->RegisterCallback([]( const sysEvent_t &event )
    //-> auto { 
    //    if (event.evType == SE_MOUSE )
    //    {
    //        int mouse_x, mouse_y;
    //        mouse_x = event.evValue;
    //        mouse_y = event.evValue2;

    //        int action,val;
    //        Sys_ReturnMouseInputEvent(0,action,val );

    //        if ( action == K_MOUSE1) {
    //            int delta_x = mouse_x - context.prev_mouse_x;
    //            int delta_y = mouse_y - context.prev_mouse_y;
    //            context.cam_yaw    += float( -delta_x ) * context.rot_scale;
    //            context.cam_pitch  += float( -delta_y ) * context.rot_scale;
    //        }

    //        context.prev_mouse_x = mouse_x;
    //        context.prev_mouse_y = mouse_y; 
    //    }
    //});

    cmdSystem->AddCommand( "quit", []( const idCmdArgs &args ) -> auto {context.quit=true;}, CMD_FL_SYSTEM, "Exit game");

    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
         common->FatalError( "SDL could not initialize. SDL_Error: %s\n", SDL_GetError( ) );
        return 1;
    }

    const int width = WINDOW_WIDTH;
    const int height = WINDOW_HEIGHT;
    SDL_Window *window = SDL_CreateWindow(
        argv[0], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width,
        height, SDL_WINDOW_SHOWN );

    if ( window == nullptr ) {
        common->FatalError("Window could not be created. SDL_Error: %s\n", SDL_GetError( ) );
        return 1;
    }
 
#if !BX_PLATFORM_EMSCRIPTEN
    SDL_SysWMinfo wmi;
    SDL_VERSION( &wmi.version );
    if ( !SDL_GetWindowWMInfo( window, &wmi ) ) {
        common->FatalError(
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
    bgfx_init.callback = (bgfx::CallbackI *)&bgfx::bgfxCallbacksLocal;
    bgfx::init( bgfx_init );

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0 );
    bgfx::setViewClear(1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000078, 1.0f, 0 );
    
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

    context.width = width;
    context.height = height;
    context.window = window;
    bgfxInitShaders( &context );

    common->PrintWarnings();
    common->ClearWarnings("main loop");
#if BX_PLATFORM_EMSCRIPTEN
    emscripten_set_main_loop_arg( main_loop, &context, -1, 1 );
#else
    while ( !context.quit ) {
        main_loop( &context );
    }
#endif // BX_PLATFORM_EMSCRIPTEN


    common->PrintWarnings( );
    common->ClearWarnings( "shutdown" );

    ImGui_ImplSDL2_Shutdown( );
    ImGui_Implbgfx_Shutdown( );

    ImGui::DestroyContext( );
    bgfxShutdown( &context );

    common->PrintWarnings( );
    imConsole->ClearLog( );

    sceneEditor->Shutdown( );
    eventLoop->Shutdown( );
    common->Shutdown( );
    fileSystem->Shutdown( false );
    cvarSystem->Shutdown( );
    cmdSystem->Shutdown( );
    idLib::ShutDown( );

    SDL_DestroyWindow( window );
    SDL_Quit( );
    return 0;
}

/*
==============
Sys_Printf
==============
*/

enum {
	MAXPRINTMSG = 4096,
	MAXNUMBUFFEREDLINES = 16
};

static char bufferedPrintfLines[MAXNUMBUFFEREDLINES][MAXPRINTMSG];
static int curNumBufferedPrintfLines = 0;
static CRITICAL_SECTION printfCritSect;

void Sys_Printf( const char *fmt, ... ) {
	char		msg[MAXPRINTMSG];

	va_list argptr;
	va_start( argptr, fmt );
	int len = idStr::vsnPrintf( msg, MAXPRINTMSG - 1, fmt, argptr );
	va_end( argptr );
	msg[sizeof( msg ) - 1] = '\0';

	//native console
	printf( "%s", msg ); 
	//debugger output
	if ( win_outputDebugString.GetBool( ) ) {
		OutputDebugString( msg );
	}

	//if ( win_outputEditString.GetBool( ) ) {
	//	if ( Sys_IsMainThread( ) ) {
	//		Conbuf_AppendText( msg );
	//	} else {
	//		EnterCriticalSection( &printfCritSect );
	//		int idx = curNumBufferedPrintfLines++;
	//		if ( idx < MAXNUMBUFFEREDLINES ) {
	//			if ( len >= MAXPRINTMSG )
	//				len = MAXPRINTMSG - 1;
	//			memcpy( bufferedPrintfLines[idx], msg, len + 1 );
	//		}
	//		LeaveCriticalSection( &printfCritSect );
	//	}
	//}
}
