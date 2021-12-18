#pragma once
#include "idFramework/sys/platform.h"
#include "idFramework/sys/sys_local.h"
#include "Common.h"
#include <idFramework/idlib/containers/StrList.h>
#include <SDL.h>
/*
==============================================================

	idCommon

==============================================================
*/

#define STDIO_PRINT( pre, post )	\
	va_list argptr;					\
	va_start( argptr, fmt );		\
	printf( pre );					\
	vprintf( fmt, argptr );			\
	printf( post );					\
	va_end( argptr )


class idCommonLocal : public idCommon {
public:
	idCommonLocal( void ) { ClearWarnings( "Intialization" ); }

	virtual void			Init( int argc, char **argv );
	virtual void			Shutdown( void );
	virtual void			Quit( void ) { }
	virtual bool			IsInitialized( void ) const { return com_fullyInitialized; }
	virtual void			Frame( void );
	virtual void			GUIFrame( bool execCmd, bool network ) { }
	virtual void			Async( void );
	virtual void			StartupVariable( const char *match, bool once ) { }
	virtual void			InitTool( const toolFlag_t tool, const idDict *dict ) { }
	virtual void			ActivateTool( bool active ) { }
	virtual void			WriteConfigToFile( const char *filename ) { }
	virtual void			WriteFlaggedCVarsToFile( const char *filename, int flags, const char *setCmd ) { }
	virtual void			BeginRedirect( char *buffer, int buffersize, void ( *flush )( const char * ) ) { }
	virtual void			EndRedirect( void ) { }
	virtual void			SetRefreshOnPrint( bool set ) { }
	virtual void			Printf( const char *fmt, ... );
	virtual void			VPrintf( const char *fmt, va_list arg );
	virtual void			DPrintf( const char *fmt, ... );
	virtual void			Warning( const char *fmt, ... );
	virtual void			VWarning( const char *fmt, va_list arg );
	virtual void			DWarning( const char *fmt, ... );
	virtual void			PrintWarnings( void );
	virtual void			ClearWarnings( const char *reason );
	virtual void			Error( const char *fmt, ... );
	virtual void			FatalError( const char *fmt, ... );
	virtual const idLangDict *GetLanguageDict( ) { return NULL; }
	virtual const char *	KeysFromBinding( const char *bind ) { return NULL; }
	virtual const char *	BindingFromKey( const char *key ) { return NULL; }
	virtual int				ButtonState( int key ) { return 0; }
	virtual int				KeyState( int key ) { return 0; }
	virtual bool			SetCallback( CallbackType cbt, FunctionPointer cb, void *userArg ) { return false; }
	virtual bool			GetAdditionalFunction( FunctionType ft, FunctionPointer *out_fnptr, void **out_userArg ) { return false; }

#define		MAX_CONSOLE_LINES	32
	int			com_numConsoleLines;
	idCmdArgs	com_consoleLines[MAX_CONSOLE_LINES];

	/*
	==================
	idCommonLocal::ParseCommandLine
	==================
	*/
	void ParseCommandLine( int argc, char **argv ) {
		int i;

		com_numConsoleLines = 0;
		// API says no program path
		for ( i = 0; i < argc; i++ ) {
			if ( argv[i][0] == '+' ) {
				com_numConsoleLines++;
				com_consoleLines[com_numConsoleLines - 1].AppendArg( argv[i] + 1 );
			} else {
				if ( !com_numConsoleLines ) {
					com_numConsoleLines++;
				}
				com_consoleLines[com_numConsoleLines - 1].AppendArg( argv[i] );
			}
		}
	}
	bool AddStartupCommands( void ) {
		int		i;
		bool	added;

		added = false;
		// quote every token, so args with semicolons can work
		for ( i = 0; i < com_numConsoleLines; i++ ) {
			if ( !com_consoleLines[i].Argc( ) ) {
				continue;
			}

			// set commands won't override menu startup
			if ( idStr::Icmpn( com_consoleLines[i].Argv( 0 ), "set", 3 ) ) {
				added = true;
			}
			// directly as tokenized so nothing gets screwed
			cmdSystem->BufferCommandArgs( CMD_EXEC_NOW, com_consoleLines[i] );
		}

		return added;
	}

private:
	void						SingleAsyncTic( void );
	bool						com_fullyInitialized;
	bool						com_refreshOnPrint;		// update the screen every print for dmap
	int							com_errorEntered;		// 0, ERP_DROP, etc
	idStr						warningCaption;
	idStrList					warningList;
	idStrList					errorList;
	SDL_TimerID					async_timer;
};
