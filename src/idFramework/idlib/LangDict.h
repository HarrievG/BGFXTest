/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __LANGDICT_H__
#define __LANGDICT_H__

#include "idlib/containers/List.h"
#include "idlib/containers/HashIndex.h"
#include "idlib/Str.h"

/*
===============================================================================

	Simple dictionary specifically for the localized string tables.

===============================================================================
*/

class idLangKeyValue {
public:
	idStr					key;
	idStr					value;
};

class idLangDict {
public:
							idLangDict( void );
							~idLangDict( void );

	void					Clear( void );
	bool					Load( const char *fileName, bool clear = true );
	void					Save( const char *fileName );

	const char *			AddString( const char *str );
	const char *			GetString( const char *str ) const;

							// adds the value and key as passed (doesn't generate a "#str_xxxxx" key or ensure the key/value pair is unique)
	void					AddKeyVal( const char *key, const char *val );

	int						GetNumKeyVals( void ) const;
	const idLangKeyValue *	GetKeyVal( int i ) const;

	void					SetBaseID(int id) { baseID = id; };

private:
	idList<idLangKeyValue>	args;
	idHashIndex				hash;

	bool					ExcludeString( const char *str ) const;
	int						GetNextId( void ) const;
	int						GetHashKey( const char *str ) const;

	int						baseID;
};


/*
================================================
idStrId represents a localized String as a String ID.
================================================
*/
class idStrId {
public:
	idStrId( ) : index( -1 ) { }
	idStrId( const idStrId &other ) : index( other.index ) { }

	explicit idStrId( int i ) : index( i ) { }
	explicit idStrId( const char *key ) { Set( key ); }
	explicit idStrId( const idStr &key ) { Set( key ); }

	void operator=( const char *key ) { Set( key ); }
	void operator=( const idStr &key ) { Set( key ); }
	void operator=( const idStrId &other ) { index = other.index; }

	bool operator==( const idStrId &other ) const { return index == other.index; }
	bool operator!=( const idStrId &other ) const { return index != other.index; }

	void			Set( const char *key );

	void			Empty( ) { index = -1; }
	bool			IsEmpty( ) const { return index < 0; }

	const char *GetKey( ) const;
	const char *GetLocalizedString( ) const;

	int				GetIndex( ) const { return index; }
	void			SetIndex( int i ) { index = i; }

private:
	int index;	// Index into the language dictionary
};

#endif /* !__LANGDICT_H__ */
