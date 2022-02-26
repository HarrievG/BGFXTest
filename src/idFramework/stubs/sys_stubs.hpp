#pragma once
/*
==============================================================

	idSys

==============================================================
*/
#include "idlib/containers/StrList.h"
#include "CVarSystem.h"

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
sysEvent_t Sys_GetEvent( );

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
