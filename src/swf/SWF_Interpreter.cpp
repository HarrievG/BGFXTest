#include "atom.h"
#include "atom-inlines.h"
#include "SWF_Bitstream.h"
#include "SWF_Enums.h"
#include "SWF_Abc.h"
#include "SWF_ScriptObject.h"
#include "SWF_ScriptFunction.h"

using namespace avmplus;
Atom gAtom;


inline int32 readS24( const byte *pc ) {
	return ((uint16_t*)pc)[0] | ((int8_t*)pc)[2]<<16;
}

inline uint32 readU30( const uint8_t *&pc ) {
	uint32 result = 0;
	for ( int i = 0; i < 5; i++ ) {
		byte b = *pc++;
		result |= ( b & 0x7F ) << ( 7 * i );
		if ( ( b & 0x80 ) == 0 ) {
			return result;
		}
	}
	return result;
}

const AbcOpcodeInfo opcodeInfo[] = {
	// For stack movement ("stk") only constant movement is accounted for; variable movement,
	// as for arguments to CALL, CONSTRUCT, APPLYTYPE, et al, and for run-time parts of
	// names, must be handled separately.

#define ABC_OP(operandCount, canThrow, stack, internalOnly, nameToken)        { operandCount, canThrow, stack , OP_##nameToken	, #nameToken },
#define ABC_UNUSED_OP(operandCount, canThrow, stack, internalOnly, nameToken) { operandCount, canThrow, stack ,	0				, #nameToken },

#include "opcodes.tbl"

#undef ABC_OP
#undef ABC_UNUSED_OP
#undef ABC_OP_F
};

void debugfile(SWF_AbcFile * file,  idSWFBitStream &bitstream  )
{
	common->Printf(" debugfile %s\n",file->constant_pool.utf8Strings[bitstream.ReadEncodedU32()].c_str());
}

void debugline(SWF_AbcFile * file,  idSWFBitStream &bitstream  )
{
	common->Printf(" debugline %i\n",(int)bitstream.ReadEncodedU32());
}

void swf_PrintStream(SWF_AbcFile * file, idSWFBitStream &bitstream ) {
	//(case.*OP_)([A-Za-z0-9]*_?[A-Za-z0-9]*)(.*\n)(.*)
	idSWFStack stack;
	static idList<AbcOpcodeInfo*> codeMap;
	idStr type;
	const AbcOpcodeInfo * info = nullptr;
	while ( bitstream.Tell() < bitstream.Length() ) {
#define DoWordCode( n ) case OP_##n: type = #n; info = &opcodeInfo[opCode]; break;
#define ExecWordCode( n ) case OP_##n: type = #n; info = &opcodeInfo[opCode]; n(file,bitstream); continue;
		SWFAbcOpcode opCode = (SWFAbcOpcode) bitstream.ReadU8();
		switch ( opCode ) {
		DoWordCode ( bkpt );
		DoWordCode ( nop );
		DoWordCode ( throw );
		DoWordCode ( getsuper );
		DoWordCode ( setsuper );
		DoWordCode ( dxns );
		DoWordCode ( dxnslate );
		DoWordCode ( kill );
		DoWordCode ( label );
		DoWordCode ( ifnlt );
		DoWordCode ( ifnle );
		DoWordCode ( ifngt );
		DoWordCode ( ifnge );
		DoWordCode ( jump );
		DoWordCode ( iftrue );
		DoWordCode ( iffalse );
		DoWordCode ( ifeq );
		DoWordCode ( ifne );
		DoWordCode ( iflt );
		DoWordCode ( ifle );
		DoWordCode ( ifgt );
		DoWordCode ( ifge );
		DoWordCode ( ifstricteq );
		DoWordCode ( ifstrictne );
		DoWordCode ( lookupswitch );
		DoWordCode ( pushwith );
		DoWordCode ( popscope );
		DoWordCode ( nextname );
		DoWordCode ( hasnext );
		DoWordCode ( pushnull );
		DoWordCode ( pushundefined );
		DoWordCode ( DISABLED_pushfloat );
		DoWordCode ( nextvalue );
		DoWordCode ( pushbyte );
		DoWordCode ( pushshort );
		DoWordCode ( pushtrue );
		DoWordCode ( pushfalse );
		DoWordCode ( pushnan );
		DoWordCode ( pop );
		DoWordCode ( dup );
		DoWordCode ( swap );
		DoWordCode ( pushstring );
		DoWordCode ( pushint );
		DoWordCode ( pushuint );
		DoWordCode ( pushdouble );
		DoWordCode ( pushscope );
		DoWordCode ( pushnamespace );
		DoWordCode ( hasnext2 );
		DoWordCode ( lix8 );
		DoWordCode ( lix16 );
		DoWordCode ( li8 );
		DoWordCode ( li16 );
		DoWordCode ( li32 );
		DoWordCode ( lf32 );
		DoWordCode ( lf64 );
		DoWordCode ( si8 );
		DoWordCode ( si16 );
		DoWordCode ( si32 );
		DoWordCode ( sf32 );
		DoWordCode ( sf64 );
		DoWordCode ( newfunction );
		DoWordCode ( call );
		DoWordCode ( construct );
		DoWordCode ( callmethod );
		DoWordCode ( callstatic );
		DoWordCode ( callsuper );
		DoWordCode ( callproperty );
		DoWordCode ( returnvoid );
		DoWordCode ( returnvalue );
		DoWordCode ( constructsuper );
		DoWordCode ( constructprop );
		DoWordCode ( callsuperid );
		DoWordCode ( callproplex );
		DoWordCode ( callinterface );
		DoWordCode ( callsupervoid );
		DoWordCode ( callpropvoid );
		DoWordCode ( sxi1 );
		DoWordCode ( sxi8 );
		DoWordCode ( sxi16 );
		DoWordCode ( applytype );
		DoWordCode ( DISABLED_pushfloat4 );
		DoWordCode ( newobject );
		DoWordCode ( newarray );
		DoWordCode ( newactivation );
		DoWordCode ( newclass );
		DoWordCode ( getdescendants );
		DoWordCode ( newcatch );
		DoWordCode ( findpropglobalstrict );
		DoWordCode ( findpropglobal );
		DoWordCode ( findpropstrict );
		DoWordCode ( findproperty );
		DoWordCode ( finddef );
		DoWordCode ( getlex );
		DoWordCode ( setproperty );
		DoWordCode ( getlocal );
		DoWordCode ( setlocal );
		DoWordCode ( getglobalscope );
		DoWordCode ( getscopeobject );
		DoWordCode ( getproperty );
		DoWordCode ( getouterscope );
		DoWordCode ( initproperty );
		DoWordCode( deleteproperty );
		DoWordCode ( getslot );
		DoWordCode ( setslot );
		DoWordCode ( getglobalslot );
		DoWordCode ( setglobalslot );
		DoWordCode ( convert_s );
		DoWordCode ( esc_xelem );
		DoWordCode ( esc_xattr );
		DoWordCode ( convert_i );
		DoWordCode ( convert_u );
		DoWordCode ( convert_d );
		DoWordCode ( convert_b );
		DoWordCode ( convert_o );
		DoWordCode ( checkfilter );
		//DoWordCode ( DISABLED_convert );
		//DoWordCode ( DISABLED_unplus );
		//DoWordCode ( DISABLED_convert );
		DoWordCode ( coerce );
		DoWordCode ( coerce_b );
		DoWordCode ( coerce_a );
		DoWordCode ( coerce_i );
		DoWordCode ( coerce_d );
		DoWordCode ( coerce_s );
		DoWordCode ( astype );
		DoWordCode ( astypelate );
		DoWordCode ( coerce_u );
		DoWordCode ( coerce_o );
		DoWordCode ( negate );
		DoWordCode ( increment );
		DoWordCode ( inclocal );
		DoWordCode ( decrement );
		DoWordCode ( declocal );
		DoWordCode ( typeof );
		DoWordCode ( not );
		DoWordCode ( bitnot );
		DoWordCode ( add );
		DoWordCode ( subtract );
		DoWordCode ( multiply );
		DoWordCode ( divide );
		DoWordCode ( modulo );
		DoWordCode ( lshift );
		DoWordCode ( rshift );
		DoWordCode ( urshift );
		DoWordCode ( bitand );
		DoWordCode ( bitor );
		DoWordCode ( bitxor );
		DoWordCode ( equals );
		DoWordCode ( strictequals );
		DoWordCode ( lessthan );
		DoWordCode ( lessequals );
		DoWordCode ( greaterthan );
		DoWordCode ( greaterequals );
		DoWordCode ( instanceof );
		DoWordCode ( istype );
		DoWordCode ( istypelate );
		DoWordCode ( in );
		DoWordCode ( increment_i );
		DoWordCode ( decrement_i );
		DoWordCode ( inclocal_i );
		DoWordCode ( declocal_i );
		DoWordCode ( negate_i );
		DoWordCode ( add_i );
		DoWordCode ( subtract_i );
		DoWordCode ( multiply_i );
		DoWordCode ( getlocal0 );
		DoWordCode ( getlocal1 );
		DoWordCode ( getlocal2 );
		DoWordCode ( getlocal3 );
		DoWordCode ( setlocal0 );
		DoWordCode ( setlocal1 );
		DoWordCode ( setlocal2 );
		DoWordCode ( setlocal3 );
		DoWordCode ( debug );
		ExecWordCode ( debugline );
		ExecWordCode ( debugfile );
		DoWordCode ( bkptline );
		DoWordCode ( timestamp );
		DoWordCode ( restargc );
		DoWordCode ( restarg );
		default:
			common->Printf( "default %s %s\n", type.c_str( ) ,info ? info->name:"Empty");
		}
		static const char * tabs[] = { " ","  ","   ","    ","      ","       ","        ","         ","          ","           ","            ","             ","              ","               ","                "};
		if ( info && info->operandCount > 0 )
			bitstream.ReadData(info->operandCount);
		common->Printf( " %s %s o %s%i  \t s %s%i \n" ,
			info ? info->name:type.c_str( ),
			tabs[int(18 - (int(idStr::Length(info->name))))],
			info->operandCount > 0 ? "^2" : "^1" ,
			info->operandCount,
			info->stack < 0 ? "^2" : "^1",
			info->stack
		);
	}
	bitstream.Rewind();
#undef DoWordCode
#undef ExecWordCode
}

void idSWFScriptFunction_Script::findpropstrict( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream ) 
{
	const auto &cp = file->constant_pool;
	const auto &mn = file->constant_pool.multinameInfos[bitstream.ReadEncodedU32( )];
	const idStrPtr propName = ( idStrPtr ) &cp.utf8Strings[mn.nameIndex];
	//search up scope stack.
	for ( int i = scope.Num( ) - 1; i >= 0; i-- ) {
		auto *s = scope[i];
		while ( s )
			if ( s->HasProperty( propName->c_str( ) ) ) 			{
				stack.Alloc( ) = s->Get( propName->c_str( ) );
				return;
			} else if ( s->GetPrototype( ) && s->GetPrototype( )->GetPrototype( ) )
				s = s->GetPrototype( )->GetPrototype( );
			else
				s = NULL;
	}
	common->FatalError("cant find %s",propName->c_str());
	//search method closure, which is the stack up until a method call?
}

void idSWFScriptFunction_Script::getlex( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream ) 
{
	const auto &cp = file->constant_pool;
	const auto &mn = file->constant_pool.multinameInfos[bitstream.ReadEncodedU32( )];
	const idStrPtr propName = ( idStrPtr ) &cp.utf8Strings[mn.nameIndex];
	//search up scope stack.
	for ( int i = scope.Num( ) - 1; i >= 0; i-- ) {
		auto *s = scope[i];
		while ( s )
			if ( s->HasProperty( propName->c_str( ) ))
			{
				stack.Alloc( ) = s->Get( propName->c_str( ) );
				return;
			}else if (s->GetPrototype( ) && s->GetPrototype( )->GetPrototype() )
				s = s->GetPrototype( )->GetPrototype();
			else
				s = NULL;
	}
}

void idSWFScriptFunction_Script::getscopeobject( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream ) {
	uint8 index = bitstream.ReadEncodedU32();
	stack.Alloc() = scope[scope.Num() - index];
}

void idSWFScriptFunction_Script::pushscope( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream ) {
	if (stack.Num() > 0)
	{ 
		if ( stack.A().IsObject() )
			scope.Alloc() = stack.A().GetObject();
		else
			common->DWarning( "tried to push a non object onto scope");
		stack.Pop(1);
	}
}

void idSWFScriptFunction_Script::popscope( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream ) {
	scope.SetNum( scope.Num() - 1, false );
}

void idSWFScriptFunction_Script::getlocal0(SWF_AbcFile* file, idSWFStack &stack, idSWFBitStream &bitstream ) {
	stack.Alloc() = registers[0];
}

//The class's static initializer will be run when the newclass instruction 
// is executed on the class_info entry for the class.
void idSWFScriptFunction_Script::newclass( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream ) {
	const auto &ci = file->classes[bitstream.ReadEncodedU32( )];
	idSWFScriptVar  base = stack.A(); stack.Pop(1);
	idSWFScriptVar* thisObj = &registers[0];
	//find static intializer.
	int  a = 1;
}

void idSWFScriptFunction_Script::callpropvoid( SWF_AbcFile *file, idSWFStack &stack, idSWFBitStream &bitstream ) {
	const auto &cp = file->constant_pool;
	const auto &mn = file->constant_pool.multinameInfos[bitstream.ReadEncodedU32( )];
	const idStrPtr funcName = ( idStrPtr ) &cp.utf8Strings[mn.nameIndex];
	uint32 arg_count = bitstream.ReadEncodedU32();

	idSWFParmList parms(arg_count);

	for ( int i = 0; i < parms.Num( ); i++ ) {
		parms[i] = stack.A( );
		stack.Pop( 1 );
	}
	idSWFScriptVar & item = stack.A();
	if (item.IsFunction())
		item.GetFunction()->Call(registers[0].GetObject(),parms);
	else if( item.IsObject() )
	{
		auto func = item.GetObject()->Get( funcName->c_str() );
		if ( func.IsFunction( ) )
		{
			((idSWFScriptFunction_Script*)func.GetFunction())->SetScope(*GetScope());
			func.GetFunction( )->Call( item.GetObject(), parms );
		}
	}
	stack.Pop(1);
}

/*
========================
idSWFScriptFunction_Script::RunAbc bytecode
========================
*/
idSWFScriptVar idSWFScriptFunction_Script::RunAbc( idSWFScriptObject *thisObject, idSWFStack &stack, idSWFBitStream &bitstream ) {
	static int abcCallstackLevel = -1;
	idSWFSpriteInstance *thisSprite = thisObject->GetSprite( );
	idSWFSpriteInstance *currentTarget = thisSprite;
	assert( abcFile );
	//return idSWFScriptVar( );
	abcCallstackLevel++;


	if ( currentTarget == NULL ) {
		thisSprite = currentTarget = defaultSprite;
	}

	while ( bitstream.Tell( ) < bitstream.Length( ) ) {
#define ExecWordCode( n ) case OP_##n: n(abcFile,stack,bitstream); continue;
#define InlineWordCode( n ) case OP_##n:
		SWFAbcOpcode opCode = (SWFAbcOpcode) bitstream.ReadU8();

		switch ( opCode ) 	{
			//ExecWordCode ( bkpt );
			//ExecWordCode ( nop );
			//ExecWordCode ( throw );
			//ExecWordCode ( getsuper );
			//ExecWordCode ( setsuper );
			//ExecWordCode ( dxns );
			//ExecWordCode ( dxnslate );
			//ExecWordCode ( kill );
			//ExecWordCode ( label );
			//ExecWordCode ( ifnlt );
			//ExecWordCode ( ifnle );
			//ExecWordCode ( ifngt );
			//ExecWordCode ( ifnge );
			//ExecWordCode ( jump );
			//ExecWordCode ( iftrue );
			//ExecWordCode ( iffalse );
			//ExecWordCode ( ifeq );
			//ExecWordCode ( ifne );
			//ExecWordCode ( iflt );
			//ExecWordCode ( ifle );
			//ExecWordCode ( ifgt );
			//ExecWordCode ( ifge );
			//ExecWordCode ( ifstricteq );
			//ExecWordCode ( ifstrictne );
			//ExecWordCode ( lookupswitch );
			//ExecWordCode ( pushwith );
			ExecWordCode ( popscope );
			//ExecWordCode ( nextname );
			//ExecWordCode ( hasnext );
			InlineWordCode( pushnull )
				stack.Append( idSWFScriptVar( NULL ) );
			continue;
			//ExecWordCode ( pushundefined );
			InlineWordCode( pushundefined )
				stack.Append( idSWFScriptVar( ) );
			continue;
			//ExecWordCode ( nextvalue );
			InlineWordCode ( pushbyte )
				stack.Append( idSWFScriptVar((int)bitstream.ReadU8()));
				continue;
			InlineWordCode ( pushshort )
				stack.Append( idSWFScriptVar((int)bitstream.ReadEncodedU32( )));
			continue;
			InlineWordCode ( pushtrue )
				stack.Append( idSWFScriptVar( true ) );
				continue;
			InlineWordCode ( pushfalse )
				stack.Append( idSWFScriptVar( false ) );
				continue;
			//ExecWordCode ( pushnan );
			InlineWordCode ( pop )
				stack.Pop(1);
			continue;
			InlineWordCode ( dup )
				stack.Alloc() = idSWFScriptVar(stack.A());
			continue;
			InlineWordCode( swap )
				common->FatalError("swap not implemented");
			continue;
			InlineWordCode ( pushstring )
			{
				const auto &cp = abcFile->constant_pool.utf8Strings;
				const auto &mn = cp[bitstream.ReadEncodedU32( )];
				stack.Append( idSWFScriptString(mn.c_str()));
				continue;
			}
			//ExecWordCode ( pushint );
			//ExecWordCode ( pushuint );
			//ExecWordCode ( pushdouble );
			ExecWordCode ( pushscope );
			//ExecWordCode ( pushnamespace );
			//ExecWordCode ( hasnext2 );
			//ExecWordCode ( lix8 );
			//ExecWordCode ( lix16 );
			//ExecWordCode ( li8 );
			//ExecWordCode ( li16 );
			//ExecWordCode ( li32 );
			//ExecWordCode ( lf32 );
			//ExecWordCode ( lf64 );
			//ExecWordCode ( si8 );
			//ExecWordCode ( si8 );
			//ExecWordCode ( si16 );
			//ExecWordCode ( si32 );
			//ExecWordCode ( sf32 );
			//ExecWordCode ( sf64 );
			//ExecWordCode ( newfunction );
			//ExecWordCode ( call );
			//ExecWordCode ( construct );
			//ExecWordCode ( callmethod );
			//ExecWordCode ( callstatic );
			InlineWordCode ( callsuper )
			{
				common->FatalError("callsuper not implemented");
				continue;
			}
			InlineWordCode ( callproperty )
			{//fold this with callpropvoid.
				const auto &cp = abcFile->constant_pool;
				const auto &mn = abcFile->constant_pool.multinameInfos[bitstream.ReadEncodedU32( )];
				const idStrPtr funcName = ( idStrPtr ) &cp.utf8Strings[mn.nameIndex];
				uint32 arg_count = bitstream.ReadEncodedU32( );

				idSWFParmList parms( arg_count );

				for ( int i = 0; i < parms.Num( ); i++ ) {
					parms[i] = stack.A( );
					stack.Pop( 1 );
				}
				idSWFScriptVar &item = stack.A( );
				if ( item.IsFunction( ) )
					stack.Alloc() = item.GetFunction( )->Call( registers[0].GetObject( ), parms );
				else if ( item.IsObject( ) ) 			{
					auto func = item.GetObject( )->Get( funcName->c_str( ) );
					if ( func.IsFunction( ) ) 				{
						( ( idSWFScriptFunction_Script * ) func.GetFunction( ) )->SetScope( *GetScope( ) );
						stack.Alloc() = func.GetFunction( )->Call( item.GetObject( ), parms );
					}
				}
				continue;
			}
			InlineWordCode ( returnvoid )
			{
				if (scope[scope.Num()-1])
					scope[scope.Num()-1]->Release();

				scope.SetNum(scope.Num()-1);
				continue;
			}
			//ExecWordCode ( returnvalue );
			InlineWordCode ( constructsuper )
			{
				uint32 args = bitstream.ReadEncodedU32( );
				stack.Pop( args );
				continue;
			}
			InlineWordCode ( constructprop )
			{

				//no need to call constructors. 
				const auto &cp = abcFile->constant_pool;
				const auto &mn = abcFile->constant_pool.multinameInfos[bitstream.ReadEncodedU32( )];
				const idStrPtr propName = ( idStrPtr ) &cp.utf8Strings[mn.nameIndex];
				uint32 arg_count = bitstream.ReadEncodedU32( );
				stack.Pop( arg_count );
				continue;


				idSWFParmList parms( arg_count );

				for ( int i = 0; i < parms.Num( ); i++ ) {
					parms[i] = stack.A( );
					stack.Pop( 1 );
				}

				idSWFScriptVar prop = stack.A();//scope[0]->Get(propName->c_str());
				if (!prop.IsUndefined() && prop.IsObject())
				{
					idSWFScriptObject * newProp = idSWFScriptObject::Alloc();
					stack.Alloc() = newProp;
					newProp->DeepCopy(prop.GetObject());
					if ( newProp->HasProperty( "__constructor__" ) ) {
						common->DPrintf( "Calling constructor for %s%\n", propName->c_str( ) );
						idSWFScriptVar instanceInit = newProp->Get( "__constructor__" );
						( ( idSWFScriptFunction_Script * ) instanceInit.GetFunction( ) )->SetScope( *GetScope( ) );
						instanceInit.GetFunction( )->Call( newProp, parms );
					}
				}else
					common->Warning("Could not construct %s ",propName->c_str());

				continue;
			}
			//ExecWordCode ( callsuperid );
			//ExecWordCode ( callproplex );
			//ExecWordCode ( callinterface );
			//ExecWordCode ( callsupervoid );
			ExecWordCode ( callpropvoid );
			//ExecWordCode ( sxi1 );
			//ExecWordCode ( sxi8 );
			//ExecWordCode ( sxi16 );
			//ExecWordCode ( applytype );
			//ExecWordCode ( DISABLED_pushfloat4 );
			//ExecWordCode ( newobject );
			//ExecWordCode ( newarray );
			//ExecWordCode ( newactivation );
			ExecWordCode ( newclass );
			//ExecWordCode ( getdescendants );
			//ExecWordCode ( newcatch );
			//ExecWordCode ( findpropglobalstrict );
			//ExecWordCode ( findpropglobal );
			ExecWordCode ( findpropstrict );
			//ExecWordCode ( findproperty );
			//ExecWordCode ( finddef );
			ExecWordCode ( getlex );
			InlineWordCode ( setproperty );
			{
				const auto &cp = abcFile->constant_pool;
				const auto &mn = cp.multinameInfos[bitstream.ReadEncodedU32( )];
				const auto &n = cp.utf8Strings[mn.nameIndex];

				idSWFScriptVar value = stack.A( );
				stack.Pop( 1 );
				stack.A( ).GetObject( )->Set( n, value );
				continue;
			}
			InlineWordCode ( getlocal )
			{
				stack.Alloc() = registers[bitstream.ReadEncodedU32()];
				continue;
			}
			//ExecWordCode ( setlocal );
			//ExecWordCode ( getglobalscope );
			ExecWordCode ( getscopeobject );
			InlineWordCode( getproperty ) 		{

				const auto &cp = abcFile->constant_pool;
				const auto &mn = cp.multinameInfos[bitstream.ReadEncodedU32( )];
				const auto &n = cp.utf8Strings[mn.nameIndex];

				if (!stack.A().IsObject())
				{
					common->Warning("cant find property %s",n.c_str());
					stack.Pop(1);
					stack.Alloc().SetUndefined();
					continue;
				}
				auto * obj = stack.A().GetObject();
				stack.Pop(1);

				if ( obj->HasProperty(n.c_str()))
					stack.Alloc() = obj->Get(n.c_str());
				else
					stack.Alloc().SetUndefined();

				continue;
			}
			//ExecWordCode ( getouterscope );
			InlineWordCode ( initproperty )
			{
				const auto &cp = abcFile->constant_pool;
				const auto &mn = cp.multinameInfos[bitstream.ReadEncodedU32( )];
				const auto &n = cp.utf8Strings[mn.nameIndex];

				idSWFScriptVar value = stack.A( );
				stack.Pop( 1 );
				stack.A().GetObject()->Set( n,value);
				continue;
			}
			//ExecWordCode ( 0x69 );
			//ExecWordCode ( deleteproperty );
			//ExecWordCode ( 0x6B );
			//ExecWordCode ( getslot );
			//ExecWordCode ( setslot );
			//ExecWordCode ( getglobalslot );
			//ExecWordCode ( setglobalslot );
			//ExecWordCode ( convert_s );
			//ExecWordCode ( esc_xelem );
			//ExecWordCode ( esc_xattr );
			//ExecWordCode ( convert_i );
			//ExecWordCode ( convert_u );
			//ExecWordCode ( convert_d );
			//ExecWordCode ( convert_b );
			//ExecWordCode ( convert_o );
			//ExecWordCode ( checkfilter );
			//ExecWordCode ( DISABLED_convert );
			//ExecWordCode ( DISABLED_unplus );
			//ExecWordCode ( DISABLED_convert );
			//ExecWordCode ( coerce );
			//ExecWordCode ( coerce_b );
			//ExecWordCode ( coerce_a );
			//ExecWordCode ( coerce_i );
			//ExecWordCode ( coerce_d );
			//ExecWordCode ( coerce_s );
			//ExecWordCode ( astype );
			//ExecWordCode ( astypelate );
			//ExecWordCode ( coerce_u );
			//ExecWordCode ( coerce_o );
			//ExecWordCode ( negate );
			//ExecWordCode ( increment );
			//ExecWordCode ( inclocal );
			//ExecWordCode ( decrement );
			//ExecWordCode ( declocal );
			//ExecWordCode ( typeof );
			//ExecWordCode ( not );
			//ExecWordCode ( bitnot );
			InlineWordCode ( add )
			{
				auto & lH = stack.B();
				auto & rH = stack.A();
				idSWFScriptVar result;
				switch ( lH.GetType( ) ) {
				case idSWFScriptVar::SWF_VAR_STRING:
					result.SetString( lH.ToString( ) + rH.ToString( ) );
					break;
				case idSWFScriptVar::SWF_VAR_FLOAT:
					result.SetFloat( lH.ToFloat( ) + rH.ToFloat( ) );
					break;
				case idSWFScriptVar::SWF_VAR_INTEGER:
					result.SetInteger( lH.ToInteger( ) + rH.ToInteger( ) );
					break;
				case idSWFScriptVar::SWF_VAR_FUNCTION:
					result.SetString( lH.ToString( ) + rH.ToString( ) );
					break;
				default:
					common->Warning( " Tried to add incompatible types %s + %s", lH.TypeOf( ), rH.TypeOf( ) );
				}

				stack.Pop(2);
				stack.Alloc() = result;
				continue;
			}
			//ExecWordCode ( subtract );
			//ExecWordCode ( multiply );
			//ExecWordCode ( divide );
			//ExecWordCode ( modulo );
			//ExecWordCode ( lshift );
			//ExecWordCode ( rshift );
			//ExecWordCode ( urshift );
			//ExecWordCode ( bitand );
			//ExecWordCode ( bitor );
			//ExecWordCode ( bitxor );
			//ExecWordCode ( equals );
			//ExecWordCode ( strictequals );
			//ExecWordCode ( lessthan );
			//ExecWordCode ( lessequals );
			//ExecWordCode ( greaterthan );
			//ExecWordCode ( greaterequals );
			//ExecWordCode ( instanceof );
			//ExecWordCode ( istype );
			//ExecWordCode ( istypelate );
			//ExecWordCode ( in );
			//ExecWordCode ( increment_i );
			//ExecWordCode ( decrement_i );
			//ExecWordCode ( inclocal_i );
			//ExecWordCode ( declocal_i );
			//ExecWordCode ( negate_i );
			//ExecWordCode ( add_i );
			//ExecWordCode ( subtract_i );
			//ExecWordCode ( multiply_i );
			ExecWordCode ( getlocal0 );
			//ExecWordCode ( getlocal1 );
			//ExecWordCode ( getlocal2 );
			//ExecWordCode ( getlocal3 );
			//ExecWordCode ( setlocal0 );
			//ExecWordCode ( setlocal1 );
			//ExecWordCode ( setlocal2 );
			//ExecWordCode ( setlocal3 );
			InlineWordCode ( debug )
			{
				uint8 type = bitstream.ReadU8();
				uint32 index = bitstream.ReadEncodedU32();
				uint8 reg = bitstream.ReadU8();
				uint32 extra = bitstream.ReadEncodedU32();
			continue;
			}
			InlineWordCode ( debugline )
			InlineWordCode ( debugfile )
				uint32 nr = bitstream.ReadEncodedU32();
			continue;
			//ExecWordCode ( bkptline );
			//ExecWordCode ( timestamp );
			//ExecWordCode ( restargc );
			//ExecWordCode ( restarg );
			//ExecWordCode ( codes );

		}
	}
	abcCallstackLevel--;
	return idSWFScriptVar( );
}
