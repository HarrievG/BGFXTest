#pragma once
/*
==============================================================

	idSys

==============================================================
*/
#include "idlib/containers/StrList.h"

void			Sys_Mkdir( const char *path ) { }
ID_TIME_T		Sys_FileTimeStamp( FILE *fp ) { return 0; }

#ifdef _WIN32

#include <io.h>
#include <direct.h>

/*
================
Sys_GetEvent
================
*/
sysEvent_t Sys_GetEvent( ) { return sysEvent_t( ); }
/*
================
Sys_Milliseconds
================
*/
unsigned int Sys_Milliseconds( ) {
	return SDL_GetTicks( );
}

const char *Sys_Cwd( void ) {
	static char cwd[1024];

	_getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[sizeof( cwd ) - 1] = 0;

	return cwd;
}

bool Sys_GetPath( sysPath_t type, idStr &path ) {
	switch ( type ) {
	case PATH_BASE:
		path = Sys_Cwd( );
		return true;

	case PATH_SAVE:
		path = cvarSystem->GetCVarString( "fs_basepath" );
		return true;

	case PATH_EXE:
		return false;
	}

	return false;
}

int Sys_ListFiles( const char *directory, const char *extension, idStrList &list ) {
	idStr		search;
	struct _finddata_t findinfo;
	intptr_t	findhandle;
	int			flag;

	if ( !extension ) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		flag = 0;
	} else {
		flag = _A_SUBDIR;
	}

	sprintf( search, "%s\\*%s", directory, extension );

	// search
	list.Clear( );

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		return -1;
	}

	do {
		if ( flag ^ ( findinfo.attrib & _A_SUBDIR ) ) {
			list.Append( findinfo.name );
		}
	} while ( _findnext( findhandle, &findinfo ) != -1 );

	_findclose( findhandle );

	return list.Num( );
}

#else

bool Sys_GetPath( sysPath_t, idStr & ) {
	return false;
}

int				Sys_ListFiles( const char *directory, const char *extension, idStrList &list ) { return 0; }

#endif

void			Sys_CreateThread( xthread_t function, void *parms, xthreadInfo &info, const char *name ) { }
void			Sys_DestroyThread( xthreadInfo &info ) { }

void			Sys_EnterCriticalSection( int index ) { }
void			Sys_LeaveCriticalSection( int index ) { }

void			Sys_WaitForEvent( int index ) { }
void			Sys_TriggerEvent( int index ) { }

/*
==============
idSysLocal stub
==============
*/
void			idSysLocal::DebugPrintf( const char *fmt, ... ) { }
void			idSysLocal::DebugVPrintf( const char *fmt, va_list arg ) { }

//double			idSysLocal::GetClockTicks( void ) { return 0.0; }
//double			idSysLocal::ClockTicksPerSecond( void ) { return 1.0; }
int				idSysLocal::GetProcessorId( void ) { return 0; }
void			idSysLocal::FPU_SetFTZ( bool enable ) { }
void			idSysLocal::FPU_SetDAZ( bool enable ) { }

bool			idSysLocal::LockMemory( void *ptr, int bytes ) { return false; }
bool			idSysLocal::UnlockMemory( void *ptr, int bytes ) { return false; }

uintptr_t		idSysLocal::DLL_Load( const char *dllName ) { return 0; }
void *idSysLocal::DLL_GetProcAddress( uintptr_t dllHandle, const char *procName ) { return NULL; }
void			idSysLocal::DLL_Unload( uintptr_t dllHandle ) { }
void			idSysLocal::DLL_GetFileName( const char *baseName, char *dllName, int maxLength ) { }

sysEvent_t		idSysLocal::GenerateMouseButtonEvent( int button, bool down ) { sysEvent_t ev; memset( &ev, 0, sizeof( ev ) ); return ev; }
sysEvent_t		idSysLocal::GenerateMouseMoveEvent( int deltax, int deltay ) { sysEvent_t ev; memset( &ev, 0, sizeof( ev ) ); return ev; }

void			idSysLocal::OpenURL( const char *url, bool quit ) { }
void			idSysLocal::StartProcess( const char *exeName, bool quit ) { }

unsigned int	idSysLocal::GetMilliseconds( void ) { return 0; }
