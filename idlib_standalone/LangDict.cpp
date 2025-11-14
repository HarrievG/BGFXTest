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

#include "Lexer.h"

#include "LangDict.h"

#include <cctype>

#define STRTABLE_ID				"#str_"
#define STRTABLE_ID_LENGTH		5
idLangDict	idLocalization::languageDict;

/*
========================
idLocalization::ClearDictionary
========================
*/
void idLocalization::ClearDictionary() {
	languageDict.Clear();
}

/*
========================
idLocalization::LoadDictionary
========================
*/
bool idLocalization::LoadDictionary( const byte * data, int dataLen, const char * fileName ) {
	return languageDict.Load( data, dataLen, fileName );
}

/*
========================
idLocalization::GetString
========================
*/
const char * idLocalization::GetString( const char * inString ) {
	return languageDict.GetString( inString );
}

/*
========================
idLocalization::FindString
========================
*/
const char * idLocalization::FindString( const char * inString ) {
	return languageDict.FindString( inString );
}

/*
========================
idLocalization::VerifyUTF8
========================
*/
utf8Encoding_t idLocalization::VerifyUTF8( const uint8_t * buffer, const int bufferLen, const char * name ) {
	utf8Encoding_t encoding;
	idStr::IsValidUTF8( buffer, bufferLen, encoding );
	if ( encoding == UTF8_INVALID ) {
	} else if ( encoding == UTF8_INVALID_BOM ) {
	} else if ( encoding == UTF8_ENCODED_NO_BOM ) {
	} else if ( encoding != UTF8_ENCODED_BOM && encoding != UTF8_PURE_ASCII ) {
	}
	return encoding;
}

// string entries can refer to other string entries,
// recursing up to this many times before we decided someone did something stupid
const char * idLangDict::KEY_PREFIX = "#str_";	// all keys should be prefixed with this for redirection to work
const int idLangDict::KEY_PREFIX_LEN = idStr::Length( KEY_PREFIX );

/*
============
idLangDict::idLangDict
============
*/
idLangDict::idLangDict( void ) {
	args.SetGranularity( 256 );
	hash.SetGranularity( 256 );
	hash.Clear( 4096, 8192 );
	baseID = 0;
}

/*
============
idLangDict::~idLangDict
============
*/
idLangDict::~idLangDict( void ) {
	Clear();
}

/*
============
idLangDict::Clear
============
*/
void idLangDict::Clear( void ) {
	args.Clear();
	hash.Clear();
}

/*
============
idLangDict::Load
============
*/
bool idLangDict::Load( const char *fileName, bool clear /* _D3XP */ ) {
	return false;
}

/*
========================
idLangDict::Load
========================
*/
bool idLangDict::Load( const byte * buffer, const int bufferLen, const char *name ) {

	if ( buffer == NULL || bufferLen <= 0 ) {
		// let whoever called us deal with the failure (so sys_lang can be reset)
		return false;
	}

	bool utf8 = false;

	// in all but retail builds, ensure that the byte-order mark is NOT MISSING so that
	// we can avoid debugging UTF-8 code
#ifndef ID_RETAIL
	utf8Encoding_t encoding = idLocalization::VerifyUTF8( buffer, bufferLen, name );
	if ( encoding == UTF8_ENCODED_BOM ) {
		utf8 = true;
	} else if ( encoding == UTF8_PURE_ASCII ) {
		utf8 = false;
	} else {
		assert( false );	// this should have been handled in VerifyUTF8 with a FatalError
		return false;
	}
#else
	// in release we just check the BOM so we're not scanning the lang file twice on startup
	if ( bufferLen > 3 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF ) {
		utf8 = true;
	}
#endif

	if ( utf8 ) {
	} else {
	}

	idStr tempKey;
	idStr tempVal;

	int line = 0;
	int numStrings = 0;

	int i = 0;
	while ( i < bufferLen ) {
		uint32_t c = buffer[i++];
		if ( c == '/' ) { // comment, read until new line
			while ( i < bufferLen ) {
				c = buffer[i++];
				if ( c == '\n' ) {
					line++;
					break;
				}
			}
		} else if ( c == '}' ) {
			break;
		} else if ( c == '\n' ) {
			line++;
		} else if ( c == '\"' ) {
			int keyStart = i;
			int keyEnd = -1;
			while ( i < bufferLen ) {
				c = buffer[i++];
				if ( c == '\"' ) {
					keyEnd = i - 1;
					break;
				}
			}
			if ( keyEnd < keyStart ) {
			}
			tempKey.CopyRange( (char *)buffer, keyStart, keyEnd );

			int valStart = -1;
			while ( i < bufferLen ) {
				c = buffer[i++];
				if ( c == '\"' ) {
					valStart = i;
					break;
				}
			}
			if ( valStart < 0 ) {
			}
			int valEnd = -1;
			tempVal.CapLength( 0 );
			while ( i < bufferLen ) {
				uint32_t c = utf8 ? idStr::UTF8Char( buffer, i ) : buffer[i++];
				if ( !utf8 && c >= 0x80 ) {
					// this is a serious error and we must check this to avoid accidentally shipping a file where someone squased UTF-8 encodings
				}
				if ( c == '\"' ) {
					valEnd = i - 1;
					continue;
				}
				if ( c == '\n' ) {
					line++;
					break;
				}
				if ( c == '\r' ) {
					continue;
				}
				if ( c == '\\' ) {
					c = utf8 ? idStr::UTF8Char( buffer, i ) : buffer[i++];
					if ( c == 'n' ) {
						c = '\n';
					} else if ( c == 't' ) {
						c = '\t';
					} else if ( c == '\"' ) {
						c = '\"';
					} else if ( c == '\\' ) {
						c = '\\';
					} else {
						// idLib::Warning( "Unknown escape sequence %x at line %d", c, line );
					}
				}
				tempVal.AppendUTF8Char( c );
			}
			if ( valEnd < valStart ) {
			}
			//if ( lang_maskLocalizedStrings.GetBool() && tempVal.Length() > 0 && tempKey.Find( "#font_" ) == -1 ) {
			//	int len = tempVal.Length();
			//	if ( len > 0 ) {
			//		tempVal.Fill( 'W', len - 1 );
			//	} else {
			//		tempVal.Empty();
			//	}
			//	tempVal.Append( 'X' );
			//}
			AddKeyVal( tempKey, tempVal );
			numStrings++;
		}
	}


	// get rid of any waste due to geometric list growth
	//mem.PushHeap();
	args.Condense();
	//mem.PopHeap();

	return true;
}

/*
========================
idLangDict::Save
========================
*/
bool idLangDict::Save( const char * fileName ) {
	return false;
}



/*
========================
idLangDict::FindStringIndex
========================
*/
int idLangDict::FindStringIndex( const char *str ) const {
	if ( str == NULL ) {
		return -1;
	}
	int xhash = idStr::IHash( str );
	for ( int i = hash.GetFirst( xhash ); i >= 0; i = hash.GetNext( i ) ) {
		if ( idStr::Icmp( str, args[i].key ) == 0 ) {
			return i;
		}
	}
	return -1;
}
/*
========================
idLangDict::IsStringId
========================
*/
bool idLangDict::IsStringId( const char *str ) {
	return idStr::Icmpn( str, KEY_PREFIX, KEY_PREFIX_LEN ) == 0;
}

/*
========================
idLangDict::FindString_r
========================
*/
const char *idLangDict::FindString_r( const char *str, int &depth ) const {
	depth++;
	if ( depth > MAX_REDIRECTION_DEPTH ) {
		// This isn't an error because we assume the error will be obvious somewhere in a GUI or something,
		// and the whole point of tracking the depth is to avoid a crash.
		// idLib::Warning( "String '%s', indirection depth > %d", str, MAX_REDIRECTION_DEPTH );
		return NULL;
	}

	if ( str == NULL || str[0] == '\0' ) {
		return NULL;
	}

	int index = FindStringIndex( str );
	if ( index < 0 ) {
		return NULL;
	}
	const char *value = args[index].value;
	if ( value == NULL ) {
		return NULL;
	}
	if ( IsStringId( value ) ) {
		// this string is re-directed to another entry
		return FindString_r( value, depth );
	}
	return value;
}

/*
========================
idLangDict::FindString
========================
*/
const char * idLangDict::FindString( const char * str ) const {
	int depth = 0;
	return FindString_r( str, depth );
}
/*
============
idLangDict::GetString
============
*/
const char *idLangDict::GetString( const char *str ) const {

	if ( str == NULL || str[0] == '\0' ) {
		return "";
	}

	if ( idStr::Cmpn( str, STRTABLE_ID, STRTABLE_ID_LENGTH ) != 0 ) {
		return str;
	}

	int hashKey = GetHashKey( str );
	for ( int i = hash.First( hashKey ); i != -1; i = hash.Next( i ) ) {
		if ( args[i].key.Cmp( str ) == 0 ) {
			return args[i].value;
		}
	}

	// idLib::common->Warning( "Unknown string id %s", str );
	return str;
}

/*
============
idLangDict::AddString
============
*/
const char *idLangDict::AddString( const char *str ) {

	if ( ExcludeString( str ) ) {
		return str;
	}

	int c = args.Num();
	for ( int j = 0; j < c; j++ ) {
		if ( idStr::Cmp( args[j].value, str ) == 0 ){
			return args[j].key;
		}
	}

	int id = GetNextId();
	idLangKeyValue kv;
	// _D3XP
	char buf[64];
	sprintf(buf, "#str_%08i", id);
	kv.key = buf;
	// kv.key = va( "#str_%05i", id );
	kv.value = str;
	c = args.Append( kv );
	assert( kv.key.Cmpn( STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 );
	hash.Add( GetHashKey( kv.key ), c );
	return args[c].key;
}

/*
============
idLangDict::GetNumKeyVals
============
*/
int idLangDict::GetNumKeyVals( void ) const {
	return args.Num();
}

/*
========================
idLangDict::GetLocalizedString
========================
*/

const char *idLangDict::GetLocalizedString( const idStrId *strId ) const {
	if ( strId->GetIndex( ) >= 0 && strId->GetIndex( ) < args.Num( ) ) {
		if ( args[strId->GetIndex( )].value == NULL ) {
			return args[strId->GetIndex( )].key;
		} else {
			return args[strId->GetIndex( )].value;
		}
	}
	return "";
}


/*
============
idLangDict::GetKeyVal
============
*/
const idLangKeyValue * idLangDict::GetKeyVal( int i ) const {
	return &args[i];
}

/*
============
idLangDict::AddKeyVal
============
*/
void idLangDict::AddKeyVal( const char *key, const char *val ) {
	idLangKeyValue kv;
	kv.key = key;
	kv.value = val;
	//assert( kv.key.Cmpn( STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 );
	hash.Add( GetHashKey( kv.key ), args.Append( kv ) );
}

/*
============
idLangDict::ExcludeString
============
*/
bool idLangDict::ExcludeString( const char *str ) const {
	if ( str == NULL ) {
		return true;
	}

	int c = strlen( str );
	if ( c <= 1 ) {
		return true;
	}

	if ( idStr::Cmpn( str, STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 ) {
		return true;
	}

	if ( idStr::Icmpn( str, "gui::", strlen( "gui::" ) ) == 0 ) {
		return true;
	}

	if ( str[0] == '$' ) {
		return true;
	}

	int i;
	for ( i = 0; i < c; i++ ) {
		if ( isalpha( str[i] ) ) {
			break;
		}
	}
	if ( i == c ) {
		return true;
	}

	return false;
}

/*
============
idLangDict::GetNextId
============
*/
int idLangDict::GetNextId( void ) const {
	int c = args.Num();

	//Let and external user supply the base id for this dictionary
	int id = baseID;

	if ( c == 0 ) {
		return id;
	}

	idStr work;
	for ( int j = 0; j < c; j++ ) {
		work = args[j].key;
		work.StripLeading( STRTABLE_ID );
		int test = atoi( work );
		if ( test > id ) {
			id = test;
		}
	}
	return id + 1;
}

/*
============
idLangDict::GetHashKey
============
*/
int idLangDict::GetHashKey( const char *str ) const {
	int hashKey = 0;

	return idStr::IHash(str);
	// DG: Replace assertion for invalid entries with a warning that's shown only once
	//     (for D3LE mod that seems to have lots of entries like #str_adil_exis_pda_01_audio_info)
	const char* strbk = str;
	//static bool warnedAboutInvalidKey = !lang_warn_invalid_key.GetBool();
	for ( str += STRTABLE_ID_LENGTH; str[0] != '\0'; str++ ) {
		// assert( str[0] >= '0' && str[0] <= '9' );
		//if(!warnedAboutInvalidKey && (str[0] < '0' || str[0] > '9')) {
			// The "hash" code here very obviously expects numbers, but apparently it still somehow works,
			// so just warn about it and otherwise accept those entries, seems to work for D3LE?
			//idLib::common->Warning( "We have at least one invalid key in a language dict: %s\n"
			//                        " (might still work, but Doom3 really wants #str_01234, i.e. only a number after '#str_')\n", strbk );
			//warnedAboutInvalidKey = true;
		//}
		// DG end
		hashKey = hashKey * 10 + str[0] - '0';
	}
	return hashKey;
}

/*
========================
idStrId::GetLocalizedString
========================
*/
const char *idStrId::GetLocalizedString( ) const {
	return idLocalization::languageDict.GetLocalizedString( this );
}
