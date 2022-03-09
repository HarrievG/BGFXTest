#include "swf.h"
#include "../idFramework/Font.h"

#include  "quickJS/quickjs.h"
#include  "quickJS/quickjs-libc.h"

#include "SWF_Abc.h"


idCVar swf_abc_verbose( "swf_abc_verbose", "1", CVAR_BOOL, "writes out all data read" );

//trace( "hello" )
//trace( "world" )
//var clickCount = 0;
//function playAnimation( event:MouseEvent ) :void {
//	clickCount++;
//	trace( text1.text )
//		text1.text = "clicked me " + clickCount;
//}
//// Register the function as a listener with the button. 
//addEventListener( MouseEvent.CLICK, playAnimation );

#pragma warning( disable: 4189 ) // local variable is initialized but not referenced

void ReadMultiName( idSWFBitStream & bitstream ,swfMultiname & target )
{
	 target.type = (swfConstantKind_t )bitstream.ReadU8();
	 target.index = 0;
	 target.nameIndex = 0;
	 switch ( target.type ) 	
	 {
	 case RTQNameL:
	 case RTQNameLA:
		 //0,0
		 break;
	 case QName:
	 case QNameA:
		 target.index = bitstream.ReadEncodedU32();
		 target.nameIndex = bitstream.ReadEncodedU32();
		 break;

	 case RTQName:
	 case RTQNameA:
		 target.nameIndex = bitstream.ReadEncodedU32();
		 break;

	 case Multiname:
	 case MultinameA:
		 target.nameIndex = bitstream.ReadEncodedU32( );
		 target.index = bitstream.ReadEncodedU32( );
		 break;

	 case MultinameL:
	 case MultinameLA:
		 target.index = bitstream.ReadEncodedU32( );
		 break;
	 default:
		 common->FatalError("Invalid Multiname type");
		 break;
	 }
}

void ReadConstantPoolInfo( idSWFBitStream & bitstream ,  swfConstant_pool_info & target )
{
	/*cpool_info{}*/
	uint32 int_count = bitstream.ReadEncodedU32( );
	for ( uint i = 1; i < int_count; i++ )
		target.integers.Alloc( ) = bitstream.ReadS32( );

	uint32 uint_count = bitstream.ReadEncodedU32( ); 
	for ( uint i = 1; i < uint_count; i++ )
		target.uIntegers.Alloc( ) = bitstream.ReadU32( );

	uint32 double_count = bitstream.ReadEncodedU32( );
	for ( uint i = 1; i < double_count; i++ )
		target.doubles.Alloc( ) = bitstream.ReadDouble( );

	uint32 string_count = bitstream.ReadEncodedU32( ); 
	for ( uint i = 1; i < string_count; i++ ) {
		uint32 str_len = bitstream.ReadEncodedU32( );
		target.utf8Strings.Alloc( ).Append( ( char * ) bitstream.ReadData( str_len ),str_len );
	}

	uint32 namespace_count = bitstream.ReadEncodedU32( );
	for ( uint i = 1; i < namespace_count; i++ ) {
		target.namespaceKinds.Alloc() = (swfConstantKind_t)bitstream.ReadU8();
		uint32 str_idx = bitstream.ReadEncodedU32( );
		target.namespaceNames.Alloc( ) = &target.utf8Strings[(int)str_idx];
	}

	uint32 namespace_set_count = bitstream.ReadEncodedU32( );
	for ( uint i = 1; i < namespace_set_count; i++ ) {
		uint32 count = bitstream.ReadEncodedU32( );
		auto & newSet = target.namespaceSets.Alloc();
		for (uint j = 1; i < count; j++)
		{
			uint32 idx = bitstream.ReadEncodedU32( );
			newSet.Alloc() = target.namespaceNames[(int)idx];
		}
	}

	uint32 multiname_count = bitstream.ReadEncodedU32( );
	for ( uint i = 1; i < multiname_count; i++ ) {
		auto &newMn = target.multinameInfos.Alloc( );
		ReadMultiName(bitstream,newMn);
	}
}

void SWF_AbcFile::ReadOptionInfo( idSWFBitStream &bitstream, swfOption_info &newOption )
{
	newOption.option_count = bitstream.ReadEncodedU32( );
	for ( uint i = 0; i < newOption.option_count; i++ ) {
		auto &newItem = newOption.options.Alloc( );
		newItem.val = bitstream.ReadEncodedU32( );
		newItem.kind = constant_pool.namespaceKinds[bitstream.ReadEncodedU32( )];
	}
}

void SWF_AbcFile::ReadMetaDataInfo	( idSWFBitStream & bitstream , swfMetadata_info & newMetadata )
{
	newMetadata.name = &constant_pool.utf8Strings[bitstream.ReadEncodedU32()];
	uint32 item_count = bitstream.ReadEncodedU32();
	for (uint i = 0; i < item_count; i++ ) {
		auto & newItem = newMetadata.items.Alloc();
		newItem.key =  &constant_pool.utf8Strings[bitstream.ReadEncodedU32()];
		newItem.value =  &constant_pool.utf8Strings[bitstream.ReadEncodedU32()];
	}
}

void SWF_AbcFile::ReadTraitsInfo( idSWFBitStream &bitstream, swfTraits_info &newTraitsData ) 
{
	newTraitsData.name = &constant_pool.multinameInfos[bitstream.ReadEncodedU32( )];
	newTraitsData.kind = bitstream.ReadU8();

}

void SWF_AbcFile::ReadInstanceInfo( idSWFBitStream &bitstream, swfInstance_info &newInstancedata ) 
{
	newInstancedata.name = &constant_pool.multinameInfos[bitstream.ReadEncodedU32()];
	newInstancedata.super_name = &constant_pool.multinameInfos[bitstream.ReadEncodedU32()];
	newInstancedata.flags = (swfInstanceFlags_t)bitstream.ReadU8();
	if 	( (newInstancedata.flags & swfInstanceFlags_t::ClassProtectedNs) != 0 )
		newInstancedata.protectedNs = bitstream.ReadEncodedU32();
	uint32 interface_count = bitstream.ReadEncodedU32();
	for (uint i =0; i < interface_count; i++ )
		newInstancedata.interfaces.Alloc() = &constant_pool.multinameInfos[bitstream.ReadEncodedU32()];

	newInstancedata.iinit = &methods[bitstream.ReadEncodedU32()];

	uint32 trait_count = bitstream.ReadEncodedU32( );
	for (uint i = 0; i < trait_count; i++ ) {
		auto & newTrait = newInstancedata.traits.Alloc();
		ReadTraitsInfo(bitstream,newTrait);
	}

}

void SWF_AbcFile::ReadMethodInfo ( idSWFBitStream & bitstream , swfMethod_info & newMethod )
{
	uint32 idx = 0;
	newMethod.paramCount = bitstream.ReadEncodedU32();
	idx =  bitstream.ReadEncodedU32();
	newMethod.returnType =  &constant_pool.multinameInfos[idx];
	for (uint i = 0; i < newMethod.paramCount; i++ ) {
		idx = bitstream.ReadEncodedU32();
		newMethod.paramTypes.Alloc() = &constant_pool.multinameInfos[idx];
	}
	idx = bitstream.ReadEncodedU32();
	newMethod.name = &constant_pool.utf8Strings[idx];
	newMethod.flags = bitstream.ReadU8();
	newMethod.options.option_count = 0;
	if ( (newMethod.flags & swfMethod_info::HAS_OPTIONAL) != 0 )
		ReadOptionInfo(bitstream,newMethod.options);

	if ( (newMethod.flags & swfMethod_info::HAS_PARAM_NAMES) != 0 ) {
		for ( uint i = 0; i < newMethod.paramCount; i++ ) {
			idx = bitstream.ReadEncodedU32( );
			newMethod.paramNames.Alloc() = &constant_pool.utf8Strings[idx];
		}
	}

}

void idSWF::DoABC( idSWFBitStream & bitstream ) {
	SWF_AbcFile newAbcFile;

	uint32 flags = bitstream.ReadU32();
	idStr name = bitstream.ReadString();	
	int dataSize = bitstream.Length( ) - 4 - name.Length( ) - 1;
	common->Printf( "DoABC %s flags %i tagsize %i bytecode size %i \n", name.c_str( ), flags, bitstream.Length( ), dataSize );
	
	bitstream.ReadLittle( newAbcFile.minor_version );
	bitstream.ReadLittle( newAbcFile.major_version );
	
	ReadConstantPoolInfo( bitstream, newAbcFile.constant_pool );

	uint32 method_count = bitstream.ReadEncodedU32( );
	for ( uint i = 0; i < method_count; i++ ) {
		auto &newMethod = newAbcFile.methods.Alloc( );
		newAbcFile.ReadMethodInfo( bitstream, newMethod );
	}

	uint32 meta_count = bitstream.ReadEncodedU32( );
	for ( uint i = 0; i < meta_count; i++ ) {
		auto &newMeta = newAbcFile.metadatas.Alloc( );
		newAbcFile.ReadMetaDataInfo( bitstream, newMeta );
	}

	newAbcFile.class_count = bitstream.ReadEncodedU32();

	for ( uint i = 0; i < newAbcFile.class_count ; i++ ) {
		auto &newInstance = newAbcFile.instances.Alloc( );
		newAbcFile.ReadInstanceInfo( bitstream, newInstance );
	}

	JSRuntime *runtime = JS_NewRuntime( );
	if ( !runtime ) {
		common->FatalError( "line %d : JS_NewRuntime returned NULL\n", __LINE__ - 2 );
		//free( if_contents );

	}

	JSContext *ctx = JS_NewContext( runtime );
	if ( !ctx ) {
		common->FatalError( "line %d : JS_NewContext returned NULL\n", __LINE__ - 2 );
		free( runtime );
		//free( if_contents );
		//return 1;
	}
	//const byte *data = bitstream.ReadData( dataSize - 4 );	

	//js_std_eval_binary(ctx, (const unsigned char *)data, dataSize -4 , 1);
	//JSValue result = JS_Eval(ctx, (const char *)data, dataSize, "bitstream", JS_EVAL_TYPE_GLOBAL);
	//JSValue jsThis = JS_DupValue( ctx, result );
	//JSValue res = JS_Call( ctx, result, jsThis, 0, NULL );
	//common->Printf( JS_ToCString( ctx, result ) );
	//common->Printf( JS_ToCString( ctx, res ) );
	//JS_FreeValue( ctx, result );
	//JS_FreeValue( ctx, res );

	int a;
}

void idSWF::SymbolClass( idSWFBitStream &bitstream ) {

	//Header		RECORDHEADER	Tag type = 76
	//NumSymbols	UI16			Number of symbols that will be associated by this tag.
	//Tag1			U16				The 16 - bit character tag ID for the symbol to associate
	//Name1			STRING			The fully - qualified name of the ActionScript 3.0 class with which to associate this symbol.The class must have already been declared by a DoABC tag.
	//... ... ...
	//TagN			U16				Tag ID for symbol N
	//NameN STRING Fully - qualified class name for symbol N
	
	uint16 numSymbols = bitstream.ReadU16();
	for (uint i = 1 ; i < numSymbols; i++ )
	{
		uint16 tag1 = bitstream.ReadU16( );
		idStr name1 = bitstream.ReadString( );
		common->Printf("SymbolClass ^4%i tag ^5%i  ^2%s \n  ", i,tag1,name1.c_str());
	}

	//load bytecode
}