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
#ifndef __SWF_SCRIPTFUNCTION_H__
#define __SWF_SCRIPTFUNCTION_H__
#include "SWF_ScriptVar.h"
#include "SWF_ParmList.h"
#include "SWF_ScriptObject.h"
#include "SWF_Bitstream.h"
#include "SWF_Abc.h"
/*
========================
Interface for calling functions from script
========================
*/
class idSWFScriptFunction {
public:
	virtual ~idSWFScriptFunction() {};

	virtual idSWFScriptVar	Call( idSWFScriptObject * thisObject, const idSWFParmList & parms ){ return idSWFScriptVar(); }; // this should never be hit
	virtual void			AddRef(){};
	virtual void			Release(){};
	virtual idSWFScriptObject *GetPrototype() { return NULL; }
	virtual void			SetPrototype( idSWFScriptObject * _object ) { }
};

/*
========================
Interface for calling functions from script, implemented statically
========================
*/
class idSWFScriptFunction_Static : public idSWFScriptFunction {
public:
							idSWFScriptFunction_Static() { }
	virtual void			AddRef() { }
	virtual void			Release() { }
};

/*
========================
Interface for calling functions from script, implemented natively on a nested class object
========================
*/
template< typename T >
class idSWFScriptFunction_Nested : public idSWFScriptFunction {
protected:
	T * pThis;
public:
							idSWFScriptFunction_Nested() : pThis( NULL ) { }

	idSWFScriptFunction *	Bind( T * _pThis ) { pThis = _pThis; return this; }
	virtual void			AddRef() { }
	virtual void			Release() { }
};

/*
========================
Interface for calling functions from script, with reference counting
NOTE: The ref count starts at 0 here because it assumes you do an AddRef on the allocated
object.  The proper way is to start it at 1 and force the caller to Release, but that's
really kind of a pain in the ass.  It was made to be used like this:
object->Set( "myFunction", new idSWFScriptFunction_MyFunction() );
========================
*/
class idSWFScriptFunction_RefCounted : public idSWFScriptFunction {
public:
	idSWFScriptFunction_RefCounted() : refCount( 0 ) { }
	void AddRef() { refCount++; }
	void Release() { if ( --refCount <= 0 ) { delete this; } }
private:
	int refCount;
};

/*
========================
Action Scripts can define a pool of constants then push values from that pool
The documentation isn't very clear on the scope of these things, but from what
I've gathered by testing, pool is per-function and copied into the function
whenever that function is declared.
========================
*/
class idSWFConstantPool {
public:
						idSWFConstantPool();
						~idSWFConstantPool() { Clear(); }

	void				Clear();
	void				Copy( const idSWFConstantPool & other );
	idSWFScriptString * Get( int n ) { return pool[n]; }
	void				Append( idSWFScriptString * s ) { pool.Append( s ); }

private:
	idList< idSWFScriptString * > pool;
};

/*
========================
The idSWFStack class is just a helper routine for treating an idList like a stack
========================
*/
class idSWFStack : public idList< idSWFScriptVar > {
public:
	idSWFScriptVar & A() { return operator[]( Num() - 1 ); }
	idSWFScriptVar & B() { return operator[]( Num() - 2 ); }
	idSWFScriptVar & C() { return operator[]( Num() - 3 ); }
	idSWFScriptVar & D() { return operator[]( Num() - 4 ); }
	void Pop( int n )	{ SetNum( Num() - n ); }
};

/*
========================
idSWFScriptFunction_Script is a script function that's implemented in action script
========================
*/
class idSWFScriptFunction_Script : public idSWFScriptFunction {
public:
				idSWFScriptFunction_Script();
	virtual		~idSWFScriptFunction_Script();

	static idSWFScriptFunction_Script *	Alloc() { return new idSWFScriptFunction_Script; }
	void	AddRef() { refCount++; }
	void	Release() { if ( --refCount == 0 ) { delete this; } }

	
	// This could all be passed to Alloc (and was at one time) but in some places it's far more convenient to specify each separately
	void	SetFlags( uint16 _flags )								{ flags = _flags; }
	void	SetData( const byte * _data, uint32 _length )			{ data = _data; length = _length; }
	void	SetData( swfMethod_info * _method)						{ methodInfo = _method; }
	void	SetAbcFile ( SWF_AbcFile * _file )						{ abcFile = _file; }
	swfMethod_info * GetMethodInfo( )								{ return methodInfo;}
	void	SetScope( idList<idSWFScriptObject *> & scope );
	idList<idSWFScriptObject *> *	GetScope()						{ return &scope; }
	void	SetConstants( const idSWFConstantPool & _constants )	{ constants.Copy( _constants ); }
	void	SetDefaultSprite( idSWFSpriteInstance * _sprite )		{ defaultSprite = _sprite; }
	void	AllocRegisters( int numRegs	)							{ registers.SetNum( numRegs ); }
	void	AllocParameters( int numParms )							{ parameters.SetNum( numParms ); }
	void	SetParameter( uint8 n, uint8 r, const char * name )		{ parameters[n].reg = r; parameters[n].name = name; }

	idSWFScriptObject * GetPrototype()	    						{ return prototype; }
	void	SetPrototype( idSWFScriptObject * _prototype )			{ _prototype->AddRef(); assert( prototype == NULL ); prototype = _prototype; }

	virtual idSWFScriptVar	Call( idSWFScriptObject * thisObject, const idSWFParmList & parms );

	
private:
//////////////////////////////////////////////////////////////////////////
//////////////////////ABC Wordcode Interpretation/////////////////////////
//////////////////////////////////////////////////////////////////////////
	void findpropstrict( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream );
	void getlex( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream );
	void getscopeobject( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream );
	void pushscope( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream );
	void popscope( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream );
	void getlocal0( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream );
	void newclass( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream );
	void callpropvoid( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream );
	//////////////////////////////////////////////////////////////////////////

	idSWFScriptVar Run( idSWFScriptObject * thisObject, idSWFStack & stack, idSWFBitStream & bitstream );
	idSWFScriptVar RunAbc( idSWFScriptObject * thisObject, idSWFStack & stack, idSWFBitStream & bitstream );
	SWF_AbcFile *abcFile;
private:
	int					refCount;

	uint16				flags;
	const  byte *		data;
	uint32				length;
	idSWFScriptObject * prototype;

	idSWFSpriteInstance * defaultSprite;		// some actions have an implicit sprite they work off of (e.g. Action_GotoFrame outside of object scope)

	idList< idSWFScriptObject * > scope;

	idSWFConstantPool	constants;
	idList< idSWFScriptVar > registers;

	struct parmInfo_t {
		const char * name;
		uint8 reg;
	};
	idList< parmInfo_t > parameters;

	swfMethod_info *methodInfo;
};

#endif // !__SWF_SCRIPTFUNCTION_H__
