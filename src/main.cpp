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
#include "bgfx-stubs/Renderers/ForwardRenderer.h"
#include "gltf-edit/gltfParser.h"
#include "idFramework/Font.h"
#include <shellapi.h>
#include <objbase.h>
#include <timeapi.h>
#include "idFramework/sys/win32/win_local.h"
#include "bgfx-stubs/Font/text_buffer_manager.h"
#include "swf/SWF.h"

//idDeclManager *		declManager = NULL;
//int idEventLoop::JournalLevel( void ) const { return 0; }

idCVar com_editing( "edit", "0", CVAR_BOOL | CVAR_SYSTEM, "editor mode" );
idCVar com_sceneName( "sceneName", "Materials_Scifi_02.glb", CVAR_TOOL, "the gltf scene that is currently being edited" );
idCVar com_developer( "developer", "0", CVAR_BOOL | CVAR_SYSTEM, "developer mode" );
idCVar com_showImguiDemo( "ImGui demo", "0", CVAR_BOOL | CVAR_SYSTEM, "draw imgui demo window" );
idCVar win_outputDebugString( "win_outputDebugString", "1", CVAR_SYSTEM | CVAR_BOOL, "Output to debugger " );
idCVar win_outputEditString( "win_outputEditString", "1", CVAR_SYSTEM | CVAR_BOOL, "" );
idCVar win_viewlog( "win_viewlog", "0", CVAR_SYSTEM | CVAR_INTEGER, "" );
idCVar r_useRenderThread( "r_useRenderThread", "0", CVAR_ARCHIVE | CVAR_RENDERER | CVAR_INTEGER, "Multithreaded renderering" );
idCVar r_fullscreen( "r_fullscreen", "0", CVAR_ARCHIVE | CVAR_RENDERER | CVAR_BOOL, "Fullscreen" );
idCVar r_allowHighDPI( "allowHighDPI", "0", CVAR_SYSTEM | CVAR_BOOL, "windows sizes are corrected in respect to the users DPI settings" );


Win32Vars_t	win32;

extern idCVar in_grabMouse;

idSession *session = NULL;

//idSysLocal		sysLocal;
//idSys *sys = &sysLocal;

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

//#define WINDOW_WIDTH 3840
//#define WINDOW_HEIGHT 2160
ForwardRenderer * fwRender;
static TextBufferManager * textMan;
static TextBufferHandle * gTextHandle = nullptr;
void main_loop( void *data ) {
	auto context = static_cast< bgfxContext_t * >( data );

	static int cnt = 0;
	static idFont *fnt = nullptr;
	static bool initFont = true;
	
	static TextBufferHandle bufferHandle;
	static idSWF *swfTest;
	if ( !fnt && !com_editing.GetBool( ) )
	{
		fnt = idFont::RegisterFont("NotoSans-Regular.ttf");
		textMan = new TextBufferManager( fnt );
		bufferHandle = textMan->createTextBuffer( FONT_TYPE_ALPHA , BufferType::Static );

		//swfTest = new idSWF("kinterface.swf",0,textMan);
		//swfTest = new idSWF("pig.swf",0,textMan);
		//swfTest = new idSWF("test_ext.swf",0,textMan);
		//swfTest = new idSWF("line.swf",0,textMan);
		//swfTest = new idSWF("edge.swf",0,textMan);
		swfTest = new idSWF("clicktest_simlpe.swf",0,textMan);
		eventLoop->RegisterCallback( []( const sysEvent_t &event )
			-> auto {
			swfTest->HandleEvent(&event);
		} );
		swfTest->Activate(true);
		
	}

	ImGui_Implbgfx_NewFrame( );
	ImGui_ImplSDL2_NewFrame( context->window );
	common->Frame();
	ImGui::NewFrame( );
	ImGui::DockSpaceOverViewport( ImGui::GetMainViewport( ), ImGuiDockNodeFlags_PassthruCentralNode );

	ImGuizmo::BeginFrame( );

	//ImGuizmo::ViewManipulate( )
	if (com_showImguiDemo.GetBool() )
		ImGui::ShowDemoWindow( ); // your drawing here

	if (com_developer.GetBool() )
		imConsole->Draw( );
	
	if ( com_editing.GetBool() )
		sceneEditor->Render( context );
	else
	{
		swfTest->Render( Sys_Milliseconds() );
		fwRender->render( com_frameTime );

		const bgfx::Caps *caps = bgfx::getCaps( );
		{
			int txtView = 50;
			bgfx::setViewName( 50, "TEXT" );
			// Setup a top - left ortho matrix for screen space drawing.
			const bx::Vec3 at = { 0.0f, 0.0f,  0.0f };
			const bx::Vec3 eye = { 0.0f, 0.0f, -1.0f };

			float view[16];
			bx::mtxLookAt( view, eye, at );
			float ortho[16];
			bx::mtxOrtho(
				ortho
				, 0.0f
				, context->width
				, context->height
				, 0.0f
				, 0.0f
				, 100.0f
				, 0.0f
				, caps->homogeneousDepth
			);
			bgfx::setViewTransform( txtView, view, ortho );
			bgfx::setViewRect( txtView, 0, 0, uint16_t( context->width ), uint16_t( context->height ) );
			bgfx::setViewClear( txtView, BGFX_CLEAR_NONE );
			bgfx::setViewFrameBuffer( txtView, fwRender->frameBuffer );
			bgfx::setState( BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA );
			textMan->submitTextBuffer( bufferHandle, txtView );
			if ( gTextHandle != nullptr )
				textMan->submitTextBuffer( *gTextHandle, txtView );
		}
	}

	ImGui_Implbgfx_RenderDrawables( );
	ImGui::Render( );
	ImGui_Implbgfx_RenderDrawLists( ImGui::GetDrawData( ) );


	ImGuiIO &io = ImGui::GetIO( );
	// Update and Render additional Platform Windows
	// Update and Render additional Platform Windows
	if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) {
		ImGui::UpdatePlatformWindows( );
		ImGui::RenderPlatformWindowsDefault( );
	}

	static bool renderThreadStarted = false;
	if ( r_useRenderThread.GetBool( ) && !renderThreadStarted)
	{
		renderThreadStarted = true;
		bgfxStartRenderThread( );
	}else
		bgfx::frame( );


#if BX_PLATFORM_EMSCRIPTEN
	if ( context->quit ) {
		emscripten_cancel_main_loop( );
	}
#endif
}

// code that tells windows we're High DPI aware so it doesn't scale our windows
// taken from Yamagi Quake II
typedef enum D3_PROCESS_DPI_AWARENESS {
	D3_PROCESS_DPI_UNAWARE = 0,
	D3_PROCESS_SYSTEM_DPI_AWARE = 1,
	D3_PROCESS_PER_MONITOR_DPI_AWARE = 2
} YQ2_PROCESS_DPI_AWARENESS;

static void setHighDPIMode( void ) {

	if (!r_allowHighDPI.GetBool())
		return;

	/* For Vista, Win7 and Win8 */
	BOOL( WINAPI * SetProcessDPIAware )( void ) = NULL;

	/* Win8.1 and later */
	HRESULT( WINAPI * SetProcessDpiAwareness )( D3_PROCESS_DPI_AWARENESS dpiAwareness ) = NULL;

	HINSTANCE userDLL = LoadLibrary( "USER32.DLL" );

	if ( userDLL ) 	{
		SetProcessDPIAware = ( BOOL( WINAPI * )( void ) ) GetProcAddress( userDLL, "SetProcessDPIAware" );
	}

	HINSTANCE shcoreDLL = LoadLibrary( "SHCORE.DLL" );

	if ( shcoreDLL ) 	{
		SetProcessDpiAwareness = ( HRESULT( WINAPI * )( YQ2_PROCESS_DPI_AWARENESS ) )
			GetProcAddress( shcoreDLL, "SetProcessDpiAwareness" );
	}

	if ( SetProcessDpiAwareness ) {
		SetProcessDpiAwareness( D3_PROCESS_PER_MONITOR_DPI_AWARE );
	} 	else if ( SetProcessDPIAware ) {
		SetProcessDPIAware( );
	}
}

int main( int argc, char **argv )
{
	static bgfxContext_t context;

	idLib::common = common;
	idLib::cvarSystem = cvarSystem;
	idLib::fileSystem = fileSystem;
	idLib::sys = sys;

	//idLib::Init( );
	common->Init( argc, argv );
	setHighDPIMode();

	eventLoop->RegisterCallback([]( const sysEvent_t &event )
		-> auto {
		if ( event.evType == SE_KEY && event.evValue2 == 1 ) {
			if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
				idKeyInput::ExecKeyBinding( event.evValue );
		}
	});
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
	cmdSystem->AddCommand( "vid_restart", []( const idCmdArgs &args )
		-> auto {
		if ( !r_fullscreen.GetBool( ) && FULL_SCREEN ) {
			SDL_SetWindowFullscreen( context.window,0);
			SDL_SetWindowSize(context.window,WINDOW_WIDTH, WINDOW_HEIGHT);
			FULL_SCREEN = false;
			
		} else if ( r_fullscreen.GetBool( ) && !FULL_SCREEN )
		{
			SDL_SetWindowFullscreen( context.window, SDL_WINDOW_FULLSCREEN );
			FULL_SCREEN = true;
		}
	}, CMD_FL_SYSTEM, "restarts vid_subsystem", idCmdSystem::ArgCompletion_GltfName );

	const int width = WINDOW_WIDTH;
	const int height = WINDOW_HEIGHT;

	Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
	
	if (in_grabMouse.GetBool())
		flags |= SDL_WINDOW_MOUSE_GRABBED;

	SDL_Window *window = SDL_CreateWindow(
		argv[0],
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height,
		flags);


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
	bgfxCreateSysCommands( &context );
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0 );
	bgfx::setViewClear(1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0 );

	bgfx::setViewRect( 0, 0, 0, width, height );

	ImGui::CreateContext( );
	ImGuiIO &io = ImGui::GetIO( );
	io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports | ImGuiBackendFlags_RendererHasViewports;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
																//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
																//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows still not really functional.
																//io.ConfigViewportsNoAutoMerge = false;

	ImGui::StyleColorsDark( );
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
	
	bgfxStartImageLoadThread();

	if ( com_editing.GetBool() )
	{
		sceneEditor->Create();
		sceneEditor->Init(com_sceneName.GetString());
	}
	else
	{
		gltfParser->Load( "Materials_Scifi_02.glb" );
		fwRender = new ForwardRenderer( gltfParser->currentAsset );
		fwRender->reset( width,height);
		fwRender->initialize();
		
		cmdSystem->AddCommand( "r_restart", []( const idCmdArgs &args )
			-> auto {
			fwRender->reset(context.width, context.height );
			fwRender->initialize( );
		}, CMD_FL_SYSTEM, "restart renderer" );
	}


	common->PrintWarnings( );
	common->ClearWarnings( "main loop" );
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
	if ( !com_editing.GetBool() )
		fwRender->shutdown();
	bgfxShutdown( &context );

	common->PrintWarnings( );
	imConsole->ClearLog( );

	sceneEditor->Shutdown( );
	eventLoop->Shutdown( );
	imConsole->Shutdown();
	common->Shutdown( );
	fileSystem->Shutdown( false );
	cvarSystem->Shutdown( );
	cmdSystem->Shutdown( );
	gltfParser->Shutdown();

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

/*
==================
idSysLocal::OpenURL
==================
*/
void idSysLocal::OpenURL( const char *url, bool doexit ) {
	static bool doexit_spamguard = false;
	HWND wnd;

	if ( doexit_spamguard ) {
		common->DPrintf( "OpenURL: already in an exit sequence, ignoring %s\n", url );
		return;
	}

	common->Printf( "Open URL: %s\n", url );

	if ( !ShellExecute( NULL, "open", url, NULL, NULL, SW_RESTORE ) ) {
		common->Error( "Could not open url: '%s' ", url );
		return;
	}

	wnd = GetForegroundWindow( );
	if ( wnd ) {
		ShowWindow( wnd, SW_MAXIMIZE );
	}

	if ( doexit ) {
		doexit_spamguard = true;
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}
}

/*
==================
idSysLocal::StartProcess
==================
*/
void idSysLocal::StartProcess( const char *exePath, bool doexit ) {
	TCHAR				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof( si ) );
	si.cb = sizeof( si );

	strncpy( szPathOrig, exePath, _MAX_PATH );

	if ( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
		common->Error( "Could not start process: '%s' ", szPathOrig );
		return;
	}

	if ( doexit ) {
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}
}

/*
==============
Sys_DebugPrintf
==============
*/
#define MAXPRINTMSG 4096
void Sys_DebugPrintf( const char *fmt, ... ) {
	char msg[MAXPRINTMSG];

	va_list argptr;
	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, MAXPRINTMSG - 1, fmt, argptr );
	msg[sizeof( msg ) - 1] = '\0';
	va_end( argptr );

	printf( "%s", msg );

	OutputDebugString( msg );
}


/*
==============
Sys_DebugVPrintf
==============
*/
void Sys_DebugVPrintf( const char *fmt, va_list arg ) {
	char msg[MAXPRINTMSG];

	idStr::vsnPrintf( msg, MAXPRINTMSG - 1, fmt, arg );
	msg[sizeof( msg ) - 1] = '\0';

	printf( "%s", msg );

	OutputDebugString( msg );
}


/*
=====================
Sys_DLL_Load
=====================
*/
uintptr_t Sys_DLL_Load( const char *dllName ) {
	HINSTANCE	libHandle;
	libHandle = LoadLibrary( dllName );
	if ( libHandle ) {
		// since we can't have LoadLibrary load only from the specified path, check it did the right thing
		char loadedPath[MAX_OSPATH];
		GetModuleFileName( libHandle, loadedPath, sizeof( loadedPath ) - 1 );
		if ( idStr::IcmpPath( dllName, loadedPath ) ) {
			Sys_Printf( "ERROR: LoadLibrary '%s' wants to load '%s'\n", dllName, loadedPath );
			Sys_DLL_Unload( ( uintptr_t ) libHandle );
			return 0;
		}
	}
	return ( uintptr_t ) libHandle;
}

/*
=====================
Sys_DLL_GetProcAddress
=====================
*/
void *Sys_DLL_GetProcAddress( uintptr_t dllHandle, const char *procName ) {
	return ( void * ) GetProcAddress( ( HINSTANCE ) dllHandle, procName );
}

/*
=====================
Sys_DLL_Unload
=====================
*/
void Sys_DLL_Unload( uintptr_t dllHandle ) {
	if ( !dllHandle ) {
		return;
	}
	if ( FreeLibrary( ( HINSTANCE ) dllHandle ) == 0 ) {
		int lastError = GetLastError( );
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL,
			lastError,
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // Default language
			( LPTSTR ) &lpMsgBuf,
			0,
			NULL
		);
		Sys_Error( "Sys_DLL_Unload: FreeLibrary failed - %s (%d)", lpMsgBuf, lastError );
	}
}


/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void Sys_Error( const char *error, ... ) {
	va_list		argptr;
	char		text[4096];
	MSG        msg;

	//if ( !Sys_IsMainThread( ) ) {
	//	// to avoid deadlocks we mustn't call Conbuf_AppendText() etc if not in main thread!
	//	va_start( argptr, error );
	//	vsprintf( errorText, error, argptr );
	//	va_end( argptr );

	//	printf( "%s", errorText );
	//	OutputDebugString( errorText );

	//	hadError = true;
	//	return;
	//}

	//va_start( argptr, error );
	//vsprintf( text, error, argptr );
	//va_end( argptr );

	//if ( !hadError ) {
	//	// if we had an error in another thread, printf() and OutputDebugString() has already been called for this
	//	printf( "%s", text );
	//	OutputDebugString( text );
	//}

	//Conbuf_AppendText( text );
	//Conbuf_AppendText( "\n" );

	//Win_SetErrorText( text );
	//Sys_ShowConsole( 1, true );

	//timeEndPeriod( 1 );

	//Sys_ShutdownInput( );

	//GLimp_Shutdown( );

	//// wait for the user to quit
	//while ( 1 ) {
	//	if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
	//		common->Quit( );
	//	}
	//	TranslateMessage( &msg );
	//	DispatchMessage( &msg );
	//}

	//Sys_DestroyConsole( );

	exit( 1 );
}


/*
================
Sys_Init

The cvar system must already be setup
================
*/
void Sys_Init( void ) {

	//CoInitialize( NULL );

	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution
	//timeBeginPeriod( 1 );

	// get WM_TIMER messages pumped every millisecond
	//	SetTimer( NULL, 0, 100, NULL );

#ifdef DEBUG
	cmdSystem->AddCommand( "createResourceIDs", CreateResourceIDs_f, CMD_FL_TOOL, "assigns resource IDs in _resouce.h files" );
#endif
#if 0
	cmdSystem->AddCommand( "setAsyncSound", Sys_SetAsyncSound_f, CMD_FL_SYSTEM, "set the async sound option" );
#endif

	//
	// Windows version
	//
	win32.osversion.dwOSVersionInfoSize = sizeof( win32.osversion );

	if ( !GetVersionEx( ( LPOSVERSIONINFO ) &win32.osversion ) )
		Sys_Error( "Couldn't get OS info" );

	if ( win32.osversion.dwMajorVersion < 4 ) {
		Sys_Error( GAME_NAME " requires Windows version 4 (NT) or greater" );
	}
	if ( win32.osversion.dwPlatformId == VER_PLATFORM_WIN32s ) {
		Sys_Error( GAME_NAME " doesn't run on Win32s" );
	}

	common->Printf( "%d MB System Memory\n", Sys_GetSystemRam( ) );
}