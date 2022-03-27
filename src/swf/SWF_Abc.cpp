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

#include "swf.h"
#include "../idFramework/Font.h"

#include  "quickJS/quickjs.h"
#include  "quickJS/quickjs-libc.h"

#include "SWF_Abc.h"


idCVar swf_abc_verbose( "swf_abc_verbose", "1", CVAR_BOOL, "writes out all data read" );

#pragma warning( disable: 4189 ) // local variable is initialized but not referenced

void trace( const char *fmt, ... ) {
	if ( swf_abc_verbose.GetBool( ) )
	{
		common->PrintPrefix("[SWF]");
		va_list argptr;
		va_start( argptr, fmt );
		common->VPrintf( fmt, argptr );
		va_end( argptr );
		common->PrintPrefix("");
	}
}

void traceMN( const char *name, swfMultiname *mn, swfConstant_pool_info &constant_pool )
{
	idStr type;
#define switchTrace( n ) case n: type = #n;break;
	switch ( mn->type ) 	{
		switchTrace( unused_0x00 );
		switchTrace( Utf8 );
		switchTrace( Int );
		switchTrace( UInt );
		switchTrace( PrivateNs );
		switchTrace( Double );
		switchTrace( Qname );
		switchTrace( Namespace );
		switchTrace( False );
		switchTrace( True );
		switchTrace( Null );
		switchTrace( QnameA );
		switchTrace( RTQname );
		switchTrace( RTQnameA );
		switchTrace( RTQnameL );
		switchTrace( RTQnameLA );
		switchTrace( Multiname );
		switchTrace( MultinameA );
		switchTrace( MultinameL );
		switchTrace( MultinameLA );
		switchTrace( PackageNamespace );
		switchTrace( PackageInternalNs );
		switchTrace( ProtectedNamespace );
		switchTrace( ExplicitNamespace );
		switchTrace( StaticProtectedNs );
	}
	trace( "%s %s\t%s \n",name, type.c_str(),constant_pool.utf8Strings[mn->nameIndex].c_str( ) );
#undef  switchTrace
}

void traceConstantPool( swfConstant_pool_info &constant_pool ) 
{
	int cnt = 0;
	
	trace("^8========================================================\n" );
	trace(" constant pool \n" );
	trace("^8========================================================\n" );

	trace("^8integers : ^7%i\n", constant_pool.integers.Num() );
	for (auto& t : constant_pool.integers)
		trace( "^8[^7%i^8]\t ^7%i \n", cnt++, t );
	trace( "^8uIntegers ^7: %i\n", constant_pool.uIntegers.Num( ) ); cnt = 0;
	for ( auto &t : constant_pool.uIntegers )
		trace( "^8[^7%i^8]\t ^7%i \n",cnt++, (int)t );
	trace( "^8doubles ^7: %i\n", constant_pool.doubles.Num( ) ); cnt = 0;
	for ( auto &t : constant_pool.doubles )
		trace( "^8[^7%i^8]\t ^7%f \n",cnt++, ( float ) t );
	trace( "^8utf8Strings ^7: %i\n", constant_pool.utf8Strings.Num( ) ); cnt = 0;
	for ( auto &t : constant_pool.utf8Strings )
		trace( "^8[^7%i^8]\t ^7%s \n",cnt++, t.c_str( ) );
	trace( "^8namespaceNames ^7: %i\n", constant_pool.namespaceNames.Num( ) ); cnt = 0;
	for ( auto &t : constant_pool.namespaceNames )
		trace( "^8[^7%i^8]\t ^7%s \n", cnt++, t->c_str( ) );

	trace( "^8namespaceSets ^7: %i\n", constant_pool.namespaceSets.Num( ) ); cnt = 0;
	for ( auto &t : constant_pool.namespaceSets )
	{
		for ( auto &ts : t )
			trace( "^8[^7%i^8]\t ^7%s \n", cnt++, ts->c_str( ) );
	}
	
	trace( "^8multinameInfos ^7: %i\n", constant_pool.multinameInfos.Num( ) ); cnt = 0;
	for ( auto &t : constant_pool.multinameInfos )
	{
		idStr pre="^8[^7";pre+=idStr(cnt++);pre+="^8]^7\t";
		traceMN(pre.c_str(),&t,constant_pool);
	}

	trace("^8========================================================\n" );
}
void ReadMultiName( idSWFBitStream & bitstream ,swfMultiname & target )
{
	 target.type = (swfConstantKind_t )bitstream.ReadU8();
	 target.index = 0;
	 target.nameIndex = 0;
	 switch ( target.type ) 	
	 {
	 case RTQnameL:
	 case RTQnameLA:
		 //0,0
		 break;
	 case Qname:
	 case QnameA:
		 target.index = bitstream.ReadEncodedU32();
		 target.nameIndex = bitstream.ReadEncodedU32();
		 break;

	 case RTQname:
	 case RTQnameA:
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
	uint32 int_count = bitstream.ReadEncodedU32( ) ;
	target.integers.Alloc( ) = 0;
	for ( uint i = 1; i < int_count; i++ )
		target.integers.Alloc( ) = bitstream.ReadEncoded<int32>( );

	uint32 uint_count = bitstream.ReadEncodedU32( ); 
	target.uIntegers.Alloc( ) = 0;
	for ( uint i = 1; i < uint_count; i++ )
		target.uIntegers.Alloc( ) = bitstream.ReadEncodedU32( );

	uint32 double_count = bitstream.ReadEncodedU32( );
	target.doubles.Alloc( ) = 0.0;
	for ( uint i = 1; i < double_count; i++ )
		target.doubles.Alloc( ) = bitstream.ReadDouble( );

	uint32 string_count = bitstream.ReadEncodedU32( ); 
	target.utf8Strings.Alloc( ).Append("*");
	for ( uint i = 1; i < string_count; i++ ) {
		uint32 str_len = bitstream.ReadEncodedU32( );
		target.utf8Strings.Alloc( ).Append( ( char * ) bitstream.ReadData( str_len ),str_len );
	}

	uint32 namespace_count = bitstream.ReadEncodedU32( );
	target.namespaceNames.Alloc( ) = &target.utf8Strings[0];
	for ( uint i = 1; i < namespace_count; i++ ) {
		target.namespaceKinds.Alloc() = (swfConstantKind_t)bitstream.ReadU8();
		uint32 str_idx = bitstream.ReadEncodedU32( );
		target.namespaceNames.Alloc( ) = &target.utf8Strings[(int)str_idx];
	}

	uint32 namespace_set_count = bitstream.ReadEncodedU32( );
	for ( uint i = 1; i < namespace_set_count; i++ ) {
		uint32 count = bitstream.ReadEncodedU32( );
		auto & newSet = target.namespaceSets.Alloc();
		for (uint j = 0; j < count; j++)
		{
			uint32 idx = bitstream.ReadEncodedU32( );
			newSet.Alloc() = target.namespaceNames[(int)idx-1];
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

void SWF_AbcFile::ReadTraitData( idSWFBitStream &bitstream, swfTraits_info &newTraitsData ) 
{
	swfTraits_info::Type kind = (swfTraits_info::Type)((newTraitsData.kind << 4) >> 4); // snip upper half, lower 4 bits should remain.
	uint8 attrib = (newTraitsData.kind >> 4); // snip lower half, upper 4 bits should remain.
	
	switch ( kind ) {
	case swfTraits_info::Trait_Slot:
	case swfTraits_info::Trait_Const: {
		newTraitsData.data = Mem_ClearedAlloc( sizeof( swfTrait_slot ) );
		swfTrait_slot &slot = *(( swfTrait_slot * )( newTraitsData.data ));
		slot.slot_id = bitstream.ReadEncodedU32();
		slot.type_name = &constant_pool.multinameInfos[bitstream.ReadEncodedU32()];
		slot.vindex = bitstream.ReadEncodedU32();
		if ( slot.vindex != 0 )
			slot.vkind = bitstream.ReadU8();
	}break;
	case swfTraits_info::Trait_Method:
	case swfTraits_info::Trait_Getter:
	case swfTraits_info::Trait_Setter:{
		newTraitsData.data = Mem_ClearedAlloc( sizeof( swfTrait_method ) );
		swfTrait_method &method = *( swfTrait_method * )( newTraitsData.data );
		method.disp_id = bitstream.ReadEncodedU32( );
		method.method = &methods[bitstream.ReadEncodedU32( )];
	}break;
	case swfTraits_info::Trait_Class:{
		newTraitsData.data = Mem_ClearedAlloc( sizeof( swfTrait_class ) );
		swfTrait_class &tclass = *( swfTrait_class * )( newTraitsData.data );
		tclass.slot_id = bitstream.ReadEncodedU32( );
		tclass.classi = &classes[bitstream.ReadEncodedU32( )];
	}break;
	case swfTraits_info::Trait_Function:{ 
		newTraitsData.data = Mem_ClearedAlloc( sizeof( swfTrait_function ) );
		swfTrait_function &func = *( swfTrait_function * ) ( newTraitsData.data );
		func.slot_id = bitstream.ReadEncodedU32( );
		func.func = &methods[bitstream.ReadEncodedU32( )];
	}break;

	default:
		common->FatalError("Unknown trait data");
		break;
	}
}

void SWF_AbcFile::ReadTraitsInfo( idSWFBitStream &bitstream, swfTraits_info &newTraitsData ) 
{
	newTraitsData.name = &constant_pool.multinameInfos[bitstream.ReadEncodedU32( )];
	//The kind field contains two four-bit fields. The lower four bits determine the kind of this trait. The 
	//	upper four bits comprise a bit vector providing attributes of the trait. See the following tables and 
	//	sections for full descriptions. 

	newTraitsData.kind = bitstream.ReadU8();
	ReadTraitData( bitstream, newTraitsData );
	uint8 attrib = ( newTraitsData.kind >> 4 ); // snip lower half, upper 4 bits should remain.
	if ((attrib & swfTraits_info::Attrib::Metadata) != 0 ) {
		uint32 meta_count = bitstream.ReadEncodedU32(); 
		for (uint i = 0 ; i < meta_count; i++) {
			newTraitsData.metadatas.Alloc() = &metadatas[bitstream.ReadEncodedU32()];
		}
	}
}

void SWF_AbcFile::ReadClassInfo( idSWFBitStream &bitstream, swfClass_info &newClassData ) 
{
	newClassData.cinit = &methods[bitstream.ReadEncodedU32()];
	uint32 trait_count  = bitstream.ReadEncodedU32();
	for ( uint i = 0; i < trait_count; i++ ) {
		auto &newTrait = newClassData.traits.Alloc( );
		ReadTraitsInfo( bitstream, newTrait );
	}
}

//The last entry in that array is the entry point for the ABC file; that is, the last entry’s
//initialization method contains the first bytecode that’s run when the ABC file is executed.
void SWF_AbcFile::ReadScriptInfo( idSWFBitStream &bitstream, swfScript_info &newScriptData ) 
{
	uint32 init = bitstream.ReadEncodedU32( );
	uint32 trait_count = bitstream.ReadEncodedU32( );
	newScriptData.init = &methods[init];
	trace("%s \n",newScriptData.init->name->c_str());
	for ( uint i = 0; i < trait_count; i++ ) {
		auto &newTrait = newScriptData.traits.Alloc( );
		ReadTraitsInfo( bitstream, newTrait );
	}
}

void SWF_AbcFile::ReadMethodBodyInfo( idSWFBitStream &bitstream, swfMethod_body_info &newMethodBody ) 
{
	newMethodBody.method = &methods[bitstream.ReadEncodedU32()];
	assert(newMethodBody.method->body==nullptr);
	newMethodBody.method->body = &newMethodBody;

	newMethodBody.max_stack = bitstream.ReadEncodedU32();
	newMethodBody.localCount = bitstream.ReadEncodedU32();
	newMethodBody.initScopeDepth = bitstream.ReadEncodedU32();
	newMethodBody.maxScopeDepth = bitstream.ReadEncodedU32();
	newMethodBody.codeLength = bitstream.ReadEncodedU32();
	newMethodBody.code.Load(bitstream.ReadData(newMethodBody.codeLength),newMethodBody.codeLength,true);// ( byte * ) Mem_ClearedAlloc( sizeof( byte ) * newMethodBody.codeLength );
	extern void swf_PrintStream(SWF_AbcFile * file ,idSWFBitStream & bitstream);
	if ( swf_abc_verbose.GetBool( ) )
	{
		common->Printf("============================\n");
		common->Printf("Method %s 's bytecode \n",newMethodBody.method->name->c_str());
		common->Printf("============================\n");
		swf_PrintStream(this,newMethodBody.code);
	}
	//memcpy(newMethodBody.code,bitstream.ReadData(newMethodBody.codeLength),newMethodBody.codeLength);
	uint32 exception_count = bitstream.ReadEncodedU32( );
	for ( uint i = 0; i < exception_count; i++ ) {
		auto &newExceptionInfo = newMethodBody.exceptions.Alloc( );
		ReadExceptionInfo( bitstream, newExceptionInfo );
	}
	uint32 trait_count = bitstream.ReadEncodedU32( );
	for ( uint i = 0; i < trait_count; i++ ) {
		auto &newTrait = newMethodBody.traits.Alloc( );
		ReadTraitsInfo( bitstream, newTrait );
	}
}

void SWF_AbcFile::ReadExceptionInfo( idSWFBitStream &bitstream, swfException_info &newException )
{
	newException.from = bitstream.ReadEncodedU32( );
	newException.to = bitstream.ReadEncodedU32();
	newException.target = bitstream.ReadEncodedU32();
	newException.exc_type = &constant_pool.utf8Strings[bitstream.ReadEncodedU32()];
	newException.var_name = &constant_pool.utf8Strings[bitstream.ReadEncodedU32()];
}

void SWF_AbcFile::ReadInstanceInfo( idSWFBitStream &bitstream, swfInstance_info &newInstancedata ) 
{
	newInstancedata.name = &constant_pool.multinameInfos[bitstream.ReadEncodedU32()];
	newInstancedata.super_name = &constant_pool.multinameInfos[bitstream.ReadEncodedU32()];

	traceMN( "newInstancedata.name",newInstancedata.name , constant_pool );
	traceMN( "newInstancedata.super_name", newInstancedata.super_name , constant_pool );

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

void SWF_AbcFile::RemoveAccessibility()
{
// look for all 
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

	trace( "newMethod.name %s \n", newMethod.name->c_str() );

	if ( (newMethod.flags & swfMethod_info::HAS_PARAM_NAMES) != 0 ) 
	{
		trace( "newMethod.params %i \n", (int)newMethod.paramCount);

		for ( uint i = 0; i < newMethod.paramCount; i++ ) {
			idx = bitstream.ReadEncodedU32( );
			newMethod.paramNames.Alloc() = &constant_pool.utf8Strings[idx];

			trace( "newMethod.param %s \n", constant_pool.utf8Strings[idx].c_str() );
		}
	}

}

//Remove Accessibility 

void idSWF::DoABC( idSWFBitStream & bitstream ) {

	SWF_AbcFile &newAbcFile = abcFile;
	int strmSize = bitstream.Length( ) + 6; // codeLength(uin16) + recordLength(uin32) 
	uint32 flags = bitstream.ReadU32();
	idStr name = bitstream.ReadString();	
	int dataSize = bitstream.Length( ) - name.Length( );
	common->Printf( "DoABC %s flags %i tagsize %i bytecode size %i \n", name.c_str( ), flags, strmSize, dataSize );
	
	bitstream.ReadLittle( newAbcFile.minor_version );
	bitstream.ReadLittle( newAbcFile.major_version );
	
	ReadConstantPoolInfo( bitstream, newAbcFile.constant_pool );
	traceConstantPool(newAbcFile.constant_pool);

	uint32 method_count = bitstream.ReadEncodedU32( ) ;
	trace("method_count %i \n", method_count );
	for ( uint i = 0; i < method_count; i++ ) {
		auto &newMethod = newAbcFile.methods.Alloc( );
		newAbcFile.ReadMethodInfo( bitstream, newMethod );
	}

	uint32 meta_count = bitstream.ReadEncodedU32( );
	trace( "meta_count %i \n", method_count );
	for ( uint i = 0; i < meta_count; i++ ) {
		auto &newMeta = newAbcFile.metadatas.Alloc( );
		newAbcFile.ReadMetaDataInfo( bitstream, newMeta );
	}

	newAbcFile.class_count = bitstream.ReadEncodedU32();
	trace( "class_count %i (classes) \n", method_count );
	for ( uint i = 0; i < newAbcFile.class_count ; i++ ) {
		auto &newInstance = newAbcFile.instances.Alloc( );
		newAbcFile.ReadInstanceInfo( bitstream, newInstance );
	}

	for ( uint i = 0; i < newAbcFile.class_count ; i++ ) {
		auto &newClass = newAbcFile.classes.Alloc( );
		newAbcFile.ReadClassInfo( bitstream, newClass );
	}

	uint32 script_count = bitstream.ReadEncodedU32( );
	trace( "script_count %i \n", script_count );
	for ( uint i = 0; i < script_count; i++ ) {
		auto &newScript = newAbcFile.scripts.Alloc( );
		newAbcFile.ReadScriptInfo( bitstream, newScript );
	}

	uint32 methBody_count = bitstream.ReadEncodedU32( );
	trace( "methBody_count %i \n", methBody_count );
	for ( uint i = 0; i < methBody_count; i++ ) {

		auto &newMethBody = newAbcFile.method_bodies.Alloc( );
		newAbcFile.ReadMethodBodyInfo( bitstream, newMethBody );
	}

	//Create trait objects
	//resolve subclass/superclass relations. 
}

void idSWF::SymbolClass( idSWFBitStream &bitstream ) {

	//Header		RECORDHEADER	Tag type = 76
	//NumSymbols	UI16			Number of symbols that will be associated by this tag.
	//Tag1			U16				The 16 - bit character tag ID for the symbol to associate
	//Name1			STRING			The fully - qualified name of the ActionScript 3.0 class with which to associate this symbol.The class must have already been declared by a DoABC tag.
	//... ... ...
	//TagN			U16				Tag ID for symbol N
	//NameN STRING Fully - qualified class name for symbol N
	SWF_SymbolClass swfSymbolClass;
	uint16 numSymbols = bitstream.ReadU16();
	for (uint i = 0 ; i < numSymbols; i++ )
	{
		auto & newSymbol = swfSymbolClass.symbols.Alloc();
		newSymbol.tag  = bitstream.ReadU16( );
		newSymbol.name = bitstream.ReadString( );
		common->Printf("SymbolClass ^5%i ^7tag ^5%i  ^2%s \n", i,newSymbol.tag,newSymbol.name.c_str());
	}

	//load bytecode
}


