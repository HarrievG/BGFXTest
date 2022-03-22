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
void swf_InterpStream(idSWFBitStream & bitstream)
{
	//(case.*OP_)([A-Za-z0-9]*_?[A-Za-z0-9]*)(.*\n)(.*)
	idStr type;
	Atom a1, a2, a3;
	Atom *a2p;
	intptr_t i1, i2;
	uintptr_t u1, u2;
	uintptr_t u1t, u2t, u3t;   // private to the generic arithmetic macros
	double d1, d2;
	bool b1;
	uint8_t ub2;
	idSWFScriptObject * o1;
	const swfMultiname *multiname;
	//MethodEnv *f; // -----> this should be function? 
	///Traits *t1;  // -----> not sure..
	uint16_t uh2l;              // not register - its address /can/ be taken
	int32_t i32l;               // ditto
	float f2l;                  // ditto
	double d2l;                 // ditto
#ifdef SWF_FLOAT
	float4_t f4l;               // ditto
#endif
	Atom a1l;                   // ditto
	uint32_t tmp_u30;
	const byte *tmp_pc;
	const byte *pc = bitstream.Ptr( );
	const byte* volatile codeStart = bitstream.Ptr();
#  define VERBOSE(op) common->Printf( "%s\n", #op );
#  define INSTR(op) case OP_##op: VERBOSE(op);

#  define NEXT						continue
#  define U30ARG					(tmp_pc=pc, tmp_u30 = uint32_t(readU30(tmp_pc)), pc = tmp_pc, tmp_u30)
#  define U8ARG						(*pc++)
#  define S24ARG					(pc+=3, readS24(pc-3))
#  define SAVE_EXPC					expc = pc-1-codeStart
#  define SAVE_EXPC_TARGET(off)		expc = pc + (off) - codeStart

	//uint8 opCode = bitstream.ReadU8();
	int byteCnt = bitstream.Length();
	for ( ;; )
	{
#define DoWordCode( n ) case OP_##n: type = #n; break;
#define ExecWordCode( n ) case OP_##n: type = #n;
		switch ( *pc++ ) {
			INSTR( returnvoid ) { a1 = undefinedAtom;;}
			INSTR( returnvalue ) { }
			INSTR( bkpt ) NEXT;
			ExecWordCode( nop ) NEXT;
			DoWordCode( throw );
			DoWordCode( getsuper );
			DoWordCode( setsuper );
			DoWordCode( dxns );
			DoWordCode( dxnslate );
			DoWordCode( kill );
			ExecWordCode( label ) NEXT;
			DoWordCode( ifnlt );
			DoWordCode( ifnle );
			DoWordCode( ifngt );
			DoWordCode( ifnge );
			DoWordCode( jump );
			DoWordCode( iftrue );
			DoWordCode( iffalse );
			DoWordCode( ifeq );
			DoWordCode( ifne );
			DoWordCode( iflt );
			DoWordCode( ifle );
			DoWordCode( ifgt );
			DoWordCode( ifge );
			DoWordCode( ifstricteq );
			DoWordCode( ifstrictne );
			DoWordCode( lookupswitch );
			DoWordCode( pushwith );
			DoWordCode( popscope );
			DoWordCode( nextname );
			DoWordCode( hasnext );
			DoWordCode( pushnull );
			DoWordCode( pushundefined );
			DoWordCode( nextvalue );
			DoWordCode( pushbyte );
			DoWordCode( pushshort );
			DoWordCode( pushtrue );
			DoWordCode( pushfalse );
			DoWordCode( pushnan );
			DoWordCode( pop );
			DoWordCode( dup );
			DoWordCode( swap );
			DoWordCode( pushstring );
			DoWordCode( pushint );
			DoWordCode( pushuint );
			DoWordCode( pushdouble );
			DoWordCode( pushscope );
			DoWordCode( pushnamespace );
			DoWordCode( hasnext2 );
			DoWordCode( lix8 );
			DoWordCode( lix16 );
			DoWordCode( li8 );
			DoWordCode( li16 );
			DoWordCode( li32 );
			DoWordCode( lf32 );
			DoWordCode( lf64 );
			DoWordCode( si8 );
			DoWordCode( si16 );
			DoWordCode( si32 );
			DoWordCode( sf32 );
			DoWordCode( sf64 );
			DoWordCode( newfunction );
			DoWordCode( call );
			DoWordCode( construct );
			DoWordCode( callmethod );
			DoWordCode( callstatic );
			DoWordCode( callsuper );
			DoWordCode( callproperty );
			DoWordCode( constructsuper );
			DoWordCode( constructprop );
			DoWordCode( callsuperid );
			DoWordCode( callproplex );
			DoWordCode( callinterface );
			DoWordCode( callsupervoid );
			DoWordCode( callpropvoid );
			DoWordCode( sxi1 );
			DoWordCode( sxi8 );
			DoWordCode( sxi16 );
			DoWordCode( applytype );
			DoWordCode( newobject );
			DoWordCode( newarray );
			DoWordCode( newactivation );
			DoWordCode( newclass );
			DoWordCode( getdescendants );
			DoWordCode( newcatch );
			DoWordCode( findpropglobalstrict );
			DoWordCode( findpropglobal );
			DoWordCode( findpropstrict );
			DoWordCode( findproperty );
			DoWordCode( finddef );
			DoWordCode( getlex );
			DoWordCode( setproperty );
			DoWordCode( getlocal );
			DoWordCode( setlocal );
			DoWordCode( getglobalscope );
			DoWordCode( getscopeobject );
			DoWordCode( getproperty );
			DoWordCode( getouterscope );
			DoWordCode( initproperty );
			DoWordCode( deleteproperty );
			DoWordCode( getslot );
			DoWordCode( setslot );
			DoWordCode( getglobalslot );
			DoWordCode( setglobalslot );
			DoWordCode( convert_s );
			DoWordCode( esc_xelem );
			DoWordCode( esc_xattr );
			DoWordCode( convert_i );
			DoWordCode( convert_u );
			DoWordCode( convert_d );
			DoWordCode( convert_b );
			DoWordCode( convert_o );
			DoWordCode( checkfilter );
			DoWordCode( coerce );
			DoWordCode( coerce_b );
			DoWordCode( coerce_a );
			DoWordCode( coerce_i );
			DoWordCode( coerce_d );
			DoWordCode( coerce_s );
			DoWordCode( astype );
			DoWordCode( astypelate );
			DoWordCode( coerce_u );
			DoWordCode( coerce_o );
			DoWordCode( negate );
			DoWordCode( increment );
			DoWordCode( inclocal );
			DoWordCode( decrement );
			DoWordCode( declocal );
			DoWordCode( typeof );
			DoWordCode( not );
			DoWordCode( bitnot );
			DoWordCode( add );
			DoWordCode( subtract );
			DoWordCode( multiply );
			DoWordCode( divide );
			DoWordCode( modulo );
			DoWordCode( lshift );
			DoWordCode( rshift );
			DoWordCode( urshift );
			DoWordCode( bitand );
			DoWordCode( bitor );
			DoWordCode( bitxor );
			DoWordCode( equals );
			DoWordCode( strictequals );
			DoWordCode( lessthan );
			DoWordCode( lessequals );
			DoWordCode( greaterthan );
			DoWordCode( greaterequals );
			DoWordCode( instanceof );
			DoWordCode( istype );
			DoWordCode( istypelate );
			DoWordCode( in );
			DoWordCode( increment_i );
			DoWordCode( decrement_i );
			DoWordCode( inclocal_i );
			DoWordCode( declocal_i );
			DoWordCode( negate_i );
			DoWordCode( add_i );
			DoWordCode( subtract_i );
			DoWordCode( multiply_i );
			DoWordCode( getlocal0 );
			DoWordCode( getlocal1 );
			DoWordCode( getlocal2 );
			DoWordCode( getlocal3 );
			DoWordCode( setlocal0 );
			DoWordCode( setlocal1 );
			DoWordCode( setlocal2 );
			DoWordCode( setlocal3 );
			DoWordCode( debug );
			DoWordCode( debugline );
			DoWordCode( debugfile );
			ExecWordCode( bkptline ){ (void)U30ARG; NEXT;}
			ExecWordCode( timestamp ) NEXT;
			DoWordCode( restargc );
			DoWordCode( restarg );
		case OP_end_of_op_codes:
			break;
		default:
			break;
		}
		common->Printf( "%s\n", type.c_str( ) );
		//if (byteCnt-- >=0)
		//	opCode = bitstream.ReadU8();
	}
}