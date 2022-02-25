/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").  

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#ifndef	__SWF_PARMLIST_H__
#define	__SWF_PARMLIST_H__
#include "idlib/containers/StaticList.h"
#include "SWF_ScriptVar.h"

// static list for script parameters
static const int SWF_MAX_PARMS = 16;

/*
================================================
idSWFParmList 

A static list for script parameters that reduces the number of SWF allocations dramatically.
================================================
*/
class idSWFParmList : public idStaticList< idSWFScriptVar, SWF_MAX_PARMS > {
public:
					idSWFParmList() {
					}
	explicit		idSWFParmList( const int num_ ) {
						SetNum( num_ );
					}

	void	Append( const idSWFScriptVar & other );
	void	Append( idSWFScriptObject * o );
	void	Append( idSWFScriptFunction * f );
	void	Append( const char * s );
	void	Append( const idStr & s );
	void	Append( idSWFScriptString * s );
	void	Append( const float f );
	void	Append( const int32 i );
	void	Append( const bool b );
};

#endif	// __SWF_PARMLIST_H__ 
