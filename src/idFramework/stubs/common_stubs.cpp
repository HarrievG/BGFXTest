#include "common_stubs.hpp"
#include "idImGui/idImConsole.h"

#define	MAX_PRINT_MSG_SIZE	4096
#define MAX_WARNING_LIST	256


idCVar com_timestampPrints( "com_timestampPrints", "1", CVAR_SYSTEM, "print time with each console print, 1 = msec, 2 = sec", 0, 2, idCmdSystem::ArgCompletion_Integer<0, 2> );
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
		sprintf( msg, "[%i]", t );
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

	Printf( S_COLOR_RED"%s", msg );

	com_refreshOnPrint = temp;
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

	Printf( S_COLOR_YELLOW"WARNING: %s\n", msg );
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
		return;
	}

	warningList.Sort( );

	Printf( "----- Warnings -----\n" );
	Printf( "during %s...\n", warningCaption.c_str( ) );

	for ( i = 0; i < warningList.Num( ); i++ ) {
		Printf( S_COLOR_YELLOW "WARNING: " S_COLOR_RED "%s\n", warningList[i].c_str( ) );
	}
	if ( warningList.Num( ) ) {
		if ( warningList.Num( ) >= MAX_WARNING_LIST ) {
			Printf( "more than %d warnings\n", MAX_WARNING_LIST );
		} else {
			Printf( "%d warnings\n", warningList.Num( ) );
		}
	}
}
