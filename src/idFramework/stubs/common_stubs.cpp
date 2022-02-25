#include "common_stubs.hpp"
#include "idImGui/idImConsole.h"
#include "UsercmdGen.h"
#include "EventLoop.h"
#include <idFramework/KeyInput.h>

#define	MAX_PRINT_MSG_SIZE	4096
#define MAX_WARNING_LIST	256
#define MAX_ERROR_LIST		256


idCVar com_timestampPrints( "com_timestampPrints", "1", CVAR_SYSTEM, "print time with each console print, 1 = msec, 2 = sec", 0, 2, idCmdSystem::ArgCompletion_Integer<0, 2> );
idCVar com_speeds( "com_speeds", "0", CVAR_BOOL | CVAR_SYSTEM | CVAR_NOCHEAT, "show engine timings" );
idCVar com_showFPS( "com_showFPS", "0", CVAR_BOOL | CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_NOCHEAT, "show frames rendered per second" );
idCVar com_timescale( "timescale", "1", CVAR_SYSTEM | CVAR_FLOAT, "scales the time", 0.1f, 10.0f );
idCVar com_preciseTic( "com_preciseTic", "1", CVAR_BOOL | CVAR_SYSTEM, "run one game tick every async thread update" );
idCVar com_asyncInput( "com_asyncInput", "1", CVAR_BOOL | CVAR_SYSTEM, "sample input from the async thread" );
//Compilers
// 2 different approaches:
// 1:	Create dscene/GLTF scene compiler, in the same spirit as dmap/map compiler. 
//		Use GLTF scenes, als map files, and allow runAAS and runReach to run on an GLTF Scene.
// Pro:		Dont have to support all d3's map options like brushes, entities, primitives
//			Focus on primitives first.
// Con:		Have to rebuild each map option for scene
// 2:	Convert an GLTF scene to map
//		Build an custom editor with imgui to view and edit gltf scenes, then allow save as map. 
// Pro:		Only have to implement renderer fucntionality that i want, can start with static primitives only
//			.Proc .CM .AAS only have to be implemented and can be generated with legacy tools
// Con:		what about entities? brushes? dont want to build a gazillion GLTF extensions..
//			do maps support limited data sets? 
//			do i want .map to still exist? entityies, classes and so on? all those properties will not stay in their current form.
// 
// 2 sounds like the least amount of work. but, i cant reconcile with it. it has to many features which all have to be checked, and are a risk.
// 1 sounds like the safest option, the most easy way to create standalone functionality and keep it clean.
// 1 also allowes to take it step by step for each GLTF property.

// plan.
// 1. copy tools/dmap 
// 2. gltf scene viewer.
// 4a. add tools/dscene 
// 4b. dscene command and start from dmap main.

idCommonLocal		commonLocal;
idCommon *common = &commonLocal;
// com_speeds times
int				time_gameFrame;
int				time_gameDraw;
int				time_frontend;			// renderSystem frontend time
int				time_backend;			// renderSystem backend time

int				com_frameTime;			// time for the current frame in milliseconds
int				com_frameNumber;		// variable frame number
volatile int	com_ticNumber;			// 60 hz tics
int				com_editors;			// currently opened editor(s)
bool			com_editorActive;		//  true if an editor has focus

bool			com_debuggerSupported;	// only set to true when the updateDebugger function is set. see GetAdditionalFunction()

#ifdef UINTPTR_MAX // DG: make sure D3_SIZEOFPTR is consistent with reality

#if ID_SIZEOFPTR == 4
  #if UINTPTR_MAX != 0xFFFFFFFFUL
    #error "CMake assumes that we're building for a 32bit architecture, but UINTPTR_MAX doesn't match!"
  #endif
#elif ID_SIZEOFPTR == 8
  #if UINTPTR_MAX != 18446744073709551615ULL
    #error "CMake assumes that we're building for a 64bit architecture, but UINTPTR_MAX doesn't match!"
  #endif
#else
  // Hello future person with a 128bit(?) CPU, I hope the future doesn't suck too much and that you don't still use CMake.
  // Also, please adapt this check and send a pull request (or whatever way we have to send patches in the future)
  #error "ID_SIZEOFPTR should really be 4 (for 32bit targets) or 8 (for 64bit targets), what kind of machine is this?!"
#endif

#endif // UINTPTR_MAX defined
/*
==================
idCommonLocal::VPrintf

A raw string should NEVER be passed as fmt, because of "%f" type crashes.
==================
*/
void idCommonLocal::VPrintf( const char *fmt, va_list args ) {
	char		msg[MAX_PRINT_MSG_SIZE];
	int			timeLength;

	// if the cvar system is not initialized
	if ( !cvarSystem->IsInitialized( ) ) {
		return;
	}

	// optionally put a timestamp at the beginning of each print,
	// so we can see how long different init sections are taking
	if ( com_timestampPrints.GetInteger( ) ) {
		int	t = Sys_Milliseconds( );
		if ( com_timestampPrints.GetInteger( ) == 2 ) {
			t /= 1000;
		}
		sprintf( msg, "^7[%i]", t );
		timeLength = strlen( msg );
	} else {
		timeLength = 0;
	}

	// don't overflow
	if ( idStr::vsnPrintf( msg + timeLength, MAX_PRINT_MSG_SIZE - timeLength - 1, fmt, args ) < 0 ) {
		msg[sizeof( msg ) - 2] = '\n'; msg[sizeof( msg ) - 1] = '\0'; // avoid output garbling
		Sys_Printf( "idCommon::VPrintf: truncated to %zd characters\n", strlen( msg ) - 1 );
	}

	//if ( rd_buffer ) {
	//	if ( ( int ) ( strlen( msg ) + strlen( rd_buffer ) ) > ( rd_buffersize - 1 ) ) {
	//		rd_flush( rd_buffer );
	//		*rd_buffer = 0;
	//	}
	//	strcat( rd_buffer, msg );
	//	return;
	//}

	// echo to console buffer
	imConsole->AddLog( msg );

	// remove any color codes
	idStr::RemoveColors( msg );

	//if ( com_enableDebuggerServer.GetBool( ) ) {
	//	// print to script debugger server
	//	if ( com_editors & EDITOR_DEBUGGER )
	//		DebuggerServerPrint( msg );
	//	else
	//		// only echo to dedicated console and early console when debugger is not running so no 
	//		// deadlocks occur if engine functions called from the debuggerthread trace stuff..
	//		Sys_Printf( "%s", msg );
	//} else {
		Sys_Printf( "%s", msg );
	//}
#if 0	// !@#
#if defined(_DEBUG) && defined(WIN32)
	if ( strlen( msg ) < 512 ) {
		TRACE( msg );
	}
#endif
#endif

	//// don't trigger any updates if we are in the process of doing a fatal error
	//if ( com_errorEntered != ERP_FATAL ) {
	//	// update the console if we are in a long-running command, like dmap
	//	if ( com_refreshOnPrint ) {
	//		session->UpdateScreen( );
	//	}

	//	// let session redraw the animated loading screen if necessary
	//	session->PacifierUpdate( );
	//}

//#ifdef _WIN32
//
//	if ( com_outputMsg ) {
//		if ( com_msgID == -1 ) {
//			com_msgID = ::RegisterWindowMessage( DMAP_MSGID );
//			if ( !FindEditor( ) ) {
//				com_outputMsg = false;
//			} else {
//				Sys_ShowWindow( false );
//			}
//		}
//		if ( com_hwndMsg ) {
//			ATOM atom = ::GlobalAddAtom( msg );
//			::PostMessage( com_hwndMsg, com_msgID, 0, static_cast< LPARAM >( atom ) );
//		}
//	}
//
//#endif
}

inline void idCommonLocal::Init( int argc, char **argv ) {
	// in case UINTPTR_MAX isn't defined (or wrong), do a runtime check at startup
	if ( ID_SIZEOFPTR != sizeof( void * ) ) {
		common->FatalError( "Something went wrong in your build: CMake assumed that sizeof(void*) == %d but in reality it's %d!\n",
			( int ) ID_SIZEOFPTR, ( int ) sizeof( void * ) );
	}

	if ( SDL_Init( SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 ) {
		common->FatalError( "SDL could not initialize. SDL_Error: %s\n", SDL_GetError( ) );
	}

	Sys_InitThreads();

	ParseCommandLine( argc, argv );
	AddStartupCommands( );
	
	// init the user command input code
	usercmdGen->Init();

	idKeyInput::Init( );
	
	com_fullyInitialized = true;
	static auto * thisPtr = this;
	async_timer = SDL_AddTimer( USERCMD_MSEC, []( unsigned int interval, void * usr)
		-> unsigned int {
		thisPtr->Async( );
		Sys_TriggerEvent( TRIGGER_EVENT_ONE );

		// calculate the next interval to get as close to 60fps as possible
		unsigned int now = SDL_GetTicks( );
		unsigned int tick = com_ticNumber * USERCMD_MSEC;

		if ( now >= tick )
			return 1;

		return tick - now;
	}, NULL );

	if ( !async_timer )
		common->FatalError( "Error while starting the async timer: %s", SDL_GetError( ) );
}

void idCommonLocal::Shutdown( void ) {
	SDL_RemoveTimer( async_timer );
	usercmdGen->Shutdown();
	idKeyInput::Shutdown();
	com_fullyInitialized = false;
	warningCaption.Clear();
	warningList.Clear();
	errorList.Clear();

}

/*
==================
idCommonLocal::Printf

Both client and server can use this, and it will output to the appropriate place.

A raw string should NEVER be passed as fmt, because of "%f" type crashers.
==================
*/
void idCommonLocal::Printf( const char *fmt, ... ) {
	va_list argptr;
	va_start( argptr, fmt );
	VPrintf( fmt, argptr );
	va_end( argptr );
}

/*
==================
idCommonLocal::DPrintf

prints message that only shows up if the "developer" cvar is set
==================
*/
void idCommonLocal::DPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		msg[MAX_PRINT_MSG_SIZE];

	if ( !cvarSystem->IsInitialized( ) || !com_developer.GetBool( ) ) {
		return;			// don't confuse non-developers with techie stuff...
	}

	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );
	msg[sizeof( msg ) - 1] = '\0';

	// never refresh the screen, which could cause reentrency problems
	bool temp = com_refreshOnPrint;
	com_refreshOnPrint = false;

	Printf( S_COLOR_MAGENTA"%s", msg );

	com_refreshOnPrint = temp;
}
void idCommonLocal::DPrintfIf(bool expr, const char *fmt, ... ) {
	if (expr)
	{
		va_list		argptr;
		char		msg[MAX_PRINT_MSG_SIZE];

		if ( !cvarSystem->IsInitialized( ) || !com_developer.GetBool( ) ) {
			return;			// don't confuse non-developers with techie stuff...
		}

		va_start( argptr, fmt );
		idStr::vsnPrintf( msg, sizeof( msg ), fmt, argptr );
		va_end( argptr );
		msg[sizeof( msg ) - 1] = '\0';

		// never refresh the screen, which could cause reentrency problems
		bool temp = com_refreshOnPrint;
		com_refreshOnPrint = false;

		Printf( S_COLOR_MAGENTA"%s", msg );

		com_refreshOnPrint = temp;
	}
}
void idCommonLocal::VWarning( const char *fmt, va_list arg )
{
	char		msg[MAX_PRINT_MSG_SIZE];
	idStr::vsnPrintf( msg, sizeof( msg ), fmt, arg );

	msg[sizeof( msg ) - 1] = '\0';

	Printf( S_COLOR_YELLOW "WARNING: " S_COLOR_RED "%s\n", msg );

	if ( warningList.Num( ) < MAX_WARNING_LIST ) {
		warningList.AddUnique( msg );
	}

}

/*
==================
idCommonLocal::DWarning

prints warning message in yellow that only shows up if the "developer" cvar is set
==================
*/
void idCommonLocal::DWarning( const char *fmt, ... ) {
	va_list		argptr;
	char		msg[MAX_PRINT_MSG_SIZE];

	if ( !com_developer.GetBool( ) ) {
		return;			// don't confuse non-developers with techie stuff...
	}

	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );
	msg[sizeof( msg ) - 1] = '\0';

	Printf( S_COLOR_YELLOW "WARNING: " S_COLOR_MAGENTA "%s\n", msg );

	if ( warningList.Num( ) < MAX_WARNING_LIST ) {
		warningList.AddUnique( msg );
	}
}
/*
==================
idCommonLocal::Warning

prints WARNING %s and adds the warning message to a queue to be printed later on
==================
*/
void idCommonLocal::Warning( const char *fmt, ... ) {
	va_list		argptr;
	char		msg[MAX_PRINT_MSG_SIZE];

	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );
	msg[sizeof( msg ) - 1] = 0;

	Printf( S_COLOR_YELLOW "WARNING: " S_COLOR_RED "%s\n", msg );

	if ( warningList.Num( ) < MAX_WARNING_LIST ) {
		warningList.AddUnique( msg );
	}
}

/*
==================
idCommonLocal::PrintWarnings
==================
*/
void idCommonLocal::PrintWarnings( void ) {
	int i;
	if ( !warningList.Num( ) ) {
		Printf( "^2 0 warnings during %s ...\n", warningCaption.c_str( ) );
		return;
	}

	Printf( "^7----- ^3Warnings ^7-----\n" );
	warningList.Sort( );
	Printf( "^3-[^7during %s^3]-\n", warningCaption.c_str( ) );

	for ( i = 0; i < warningList.Num( ); i++ ) {
		Printf( S_COLOR_YELLOW "WARNING: " S_COLOR_RED "%s", warningList[i].c_str( ) );
	}
	if ( warningList.Num( ) ) {
		if ( warningList.Num( ) >= MAX_WARNING_LIST ) {
			Printf( S_COLOR_RED "^3-[^7more than %d warnings^3]-\n", MAX_WARNING_LIST );
		} else {
			Printf("^3-[^1%d ^7warnings^3]-\n", warningList.Num( ) );
		}
	}
}

void idCommonLocal::ClearWarnings( const char *reason ) {
	warningCaption = reason;
	warningList.Clear( );
}

void idCommonLocal::Error( const char *fmt, ... ) 
{
	va_list		argptr;
	char		msg[MAX_PRINT_MSG_SIZE];

	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );
	msg[sizeof( msg ) - 1] = 0;

	Printf( S_COLOR_RED "ERROR:: " S_COLOR_YELLOW "%s\n", msg );

	if ( errorList.Num( ) < MAX_ERROR_LIST ) {
		errorList.AddUnique( msg );
	}
}

void idCommonLocal::FatalError( const char *fmt, ... ) 
{
	va_list		argptr;
	char		msg[MAX_PRINT_MSG_SIZE];

	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );
	msg[sizeof( msg ) - 1] = 0;

	Printf( S_COLOR_RED "FATAL ERROR:: " S_COLOR_YELLOW "%s\n", msg );

	common->Shutdown();
	exit( 0 ); 
}


/*
=================
idCommonLocal::Frame
=================
*/
void idCommonLocal::Frame( void ) {
	try {

		// pump all the events
		Sys_GenerateEvents();

		//if ( com_enableDebuggerServer.IsModified() ) {
		//	if ( com_enableDebuggerServer.GetBool() ) {
		//		DebuggerServerInit();
		//	} else {
		//		DebuggerServerShutdown();
		//	}
		//}

		eventLoop->RunEventLoop();

		com_frameTime = com_ticNumber * USERCMD_MSEC;


	//	if ( idAsyncNetwork::IsActive() ) {
	//		if ( idAsyncNetwork::serverDedicated.GetInteger() != 1 ) {
	//			session->GuiFrameEvents();
	//			session->UpdateScreen( false );
	//		}
	//	} else {
	//		session->Frame();

			// normal, in-sequence screen update
	//		session->UpdateScreen( false );
	//	}

	//	// report timing information
		if ( com_speeds.GetBool() ) {
			static int	lastTime;
			int		nowTime = Sys_Milliseconds();
			int		com_frameMsec = nowTime - lastTime;
			lastTime = nowTime;
			Printf( "frame:%i all:%3i gfr:%3i rf:%3i bk:%3i\n", com_frameNumber, com_frameMsec, time_gameFrame, time_frontend, time_backend );
			time_gameFrame = 0;
			time_gameDraw = 0;
		}

		com_frameNumber++;

	//	// set idLib frame number for frame based memory dumps
		idLib::frameNumber = com_frameNumber;
	}

	catch( idException & ) {
		return;			// an ERP_DROP was thrown
	}
}




/*
=================
idCommonLocal::SingleAsyncTic

The system will asyncronously call this function 60 times a second to
handle the time-critical functions that we don't want limited to
the frame rate:

sound mixing
user input generation (conditioned by com_asyncInput)
packet server operation
packet client operation

We are not using thread safe libraries, so any functionality put here must
be VERY VERY careful about what it calls.
=================
*/

typedef struct {
	int				milliseconds;			// should always be incremeting by 60hz
	int				deltaMsec;				// should always be 16
	int				timeConsumed;			// msec spent in Com_AsyncThread()
	int				clientPacketsReceived;
	int				serverPacketsReceived;
	int				mostRecentServerPacketSequence;
} asyncStats_t;

static const int MAX_ASYNC_STATS = 1024;
asyncStats_t	com_asyncStats[MAX_ASYNC_STATS];		// indexed by com_ticNumber
int prevAsyncMsec;
int	lastTicMsec;
void idCommonLocal::SingleAsyncTic( void ) {
	// main thread code can prevent this from happening while modifying
	// critical data structures
	Sys_EnterCriticalSection();

	asyncStats_t *stat = &com_asyncStats[com_ticNumber & (MAX_ASYNC_STATS-1)];
	memset( stat, 0, sizeof( *stat ) );
	stat->milliseconds = Sys_Milliseconds();
	stat->deltaMsec = stat->milliseconds - com_asyncStats[(com_ticNumber - 1) & (MAX_ASYNC_STATS-1)].milliseconds;

	if ( usercmdGen && com_asyncInput.GetBool() ) {
		usercmdGen->UsercmdInterrupt();
	}

	//switch ( com_asyncSound.GetInteger() ) {
	//	case 1:
	//		soundSystem->AsyncUpdate( stat->milliseconds );
	//		break;
	//	case 3:
	//		soundSystem->AsyncUpdateWrite( stat->milliseconds );
	//		break;
	//}

	// we update com_ticNumber after all the background tasks
	// have completed their work for this tic
	com_ticNumber++;

	stat->timeConsumed = Sys_Milliseconds() - stat->milliseconds;

	Sys_LeaveCriticalSection();
}

/*
=================
idCommonLocal::Async
=================
*/
void idCommonLocal::Async( void ) {
	int	msec = Sys_Milliseconds();
	if ( !lastTicMsec ) {
		lastTicMsec = msec - USERCMD_MSEC;
	}

	if ( !com_preciseTic.GetBool() ) {
		// just run a single tic, even if the exact msec isn't precise
		SingleAsyncTic();
		return;
	}

	int ticMsec = USERCMD_MSEC;

	// the number of msec per tic can be varies with the timescale cvar
	float timescale = com_timescale.GetFloat();
	if ( timescale != 1.0f ) {
		ticMsec /= timescale;
		if ( ticMsec < 1 ) {
			ticMsec = 1;
		}
	}

	// don't skip too many
	if ( timescale == 1.0f ) {
		if ( lastTicMsec + 10 * USERCMD_MSEC < msec ) {
			lastTicMsec = msec - 10*USERCMD_MSEC;
		}
	}

	while ( lastTicMsec + ticMsec <= msec ) {
		SingleAsyncTic();
		lastTicMsec += ticMsec;
	}
}