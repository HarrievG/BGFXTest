#include "gltfParser.h"
#include <FileSystem.h>
#include "bgfx-stubs/bgfxRender.h"
#include "bgfx-stubs/bgfxImage.h"
static const unsigned int gltfChunk_Type_JSON =  0x4E4F534A; //1313821514
static const unsigned int gltfChunk_Type_BIN =  0x004E4942; //5130562

gltf_mesh_attribute_map s_meshAttributeMap[] = {
	"POSITION",		bgfx::Attrib::Position,	3,
	"NORMAL",		bgfx::Attrib::Normal,	3,
	"TANGENT",		bgfx::Attrib::Tangent,	3,
	"TEXCOORD_0",	bgfx::Attrib::TexCoord0,2,
	"TEXCOORD_1",	bgfx::Attrib::TexCoord1,2,
	"TEXCOORD_2",	bgfx::Attrib::TexCoord2,2,
	"TEXCOORD_3",	bgfx::Attrib::TexCoord3,2,
	"TEXCOORD_4",	bgfx::Attrib::TexCoord4,2,
	"TEXCOORD_5",	bgfx::Attrib::TexCoord5,2,
	"TEXCOORD_6",	bgfx::Attrib::TexCoord6,2,
	"TEXCOORD_7",	bgfx::Attrib::TexCoord7,2,
	"COLOR_0",		bgfx::Attrib::Color0,	4,
	"COLOR_1",		bgfx::Attrib::Color1,	4,
	"COLOR_2",		bgfx::Attrib::Color2,	4,
	"COLOR_3",		bgfx::Attrib::Color3,	4,
	"WEIGHTS_0",	bgfx::Attrib::Weight,	1,
	"",				bgfx::Attrib::Count
};
bgfx::Attrib::Enum GetAttributeEnum( const char *str , uint * elementSize = nullptr) {
	   int i = -1;
	   while ( s_meshAttributeMap[++i].attib != bgfx::Attrib::Count )
			if ( !idStr::Icmp( s_meshAttributeMap[i].stringID, str ) )
			{
				if (elementSize != nullptr)
					*elementSize = s_meshAttributeMap[i].elementSize;
				return s_meshAttributeMap[i].attib;
			}
	   
	   return bgfx::Attrib::Count;
}

//https://github.com/KhronosGroup/glTF/issues/832
gltf_accessor_component_type_map s_componentTypeMap[] = {
	"signed byte",		5120,	bgfx::AttribType::Count, 1 ,
	"unsigned byte",	5121,	bgfx::AttribType::Uint8, 1 ,
	"signed short",		5122,	bgfx::AttribType::Int16, 2 ,
	"unsigned short",	5123,	bgfx::AttribType::Count, 2 ,
	"unsigned int",		5125,	bgfx::AttribType::Count, 4 , 
	"float",			5126,	bgfx::AttribType::Float, 4 ,
	"double",			5130,	bgfx::AttribType::Float, 8 ,
	"",					0,		bgfx::AttribType::Count, 0
};

bgfx::AttribType::Enum GetComponentTypeEnum( int id  , uint * sizeInBytes = nullptr) {
	int i = -1;
	while ( s_componentTypeMap[++i].id != 0)
		if ( s_componentTypeMap[i].id == id ) {
			if (sizeInBytes != nullptr )
				*sizeInBytes = s_componentTypeMap[i].sizeInBytes;

			return s_componentTypeMap[i].type;
		}
	
	return bgfx::AttribType::Count;
}


//some arbitrary amount for now.
#define GLTF_MAX_CHUNKS 32

//Helper macros for gltf data deserialize
//NOTE: gltfItems that deviate from the default SET(T*) function cannot be handled with itemref macro.
// target must be an gltfArrayItem.
// type with name will be added to the array.
#define GLTFARRAYITEM(target,name,type) auto name = new type (#name); target.AddItemDef((parsable*)name)
// name must point to an existing valid entry
// name->Set		(&target->name);
#define GLTFARRAYITEMREF(target,name) name->Set(&target->name)

//void gltfCache::clear()
//{
//	images.DeleteContents(true);
//}

idList<gltfData *> gltfData::dataList;

idCVar gltf_parseVerbose( "gltf_parseVerbose", "1", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "print gltf json data while parsing" );

void gltfPropertyArray::Iterator::operator ++( ) {

	//check if by modification, we are iterating again.
	//custom not AOS things can do this since it is not nicely guarded by braces
	if ( array->dirty && (!array->iterating &&  !array->isArrayOfStructs) ) {
		array->iterating = array->parser->PeekTokenString( "," );
		if ( array->iterating ) {
			array->properties.AssureSizeAlloc( array->properties.Num( ) + 1, idListNewElement<gltfPropertyItem> );
			array->parser->ExpectTokenString( "," );
		} 
	}

	if ( array->iterating )
	{
		p = array->properties[array->properties.Num( ) - 1];
		p->array = array;
		if (array->isArrayOfStructs )
			array->parser->ParseBracedSection( p->item );
		else
		{
			idToken token;
			array->parser->ExpectAnyToken( &token );
			p->item = token;
		}
		array->iterating = array->parser->PeekTokenString( "," );
		if ( array->iterating ) {
			array->properties.AssureSizeAlloc( array->properties.Num( ) + 1, idListNewElement<gltfPropertyItem> );
			array->parser->ExpectTokenString( "," );
		} 
	}else
	{
		if ( array->dirty ){
			p = array->endPtr;
			array->dirty = false;
		}
		else if ( array->index + 1 < array->properties.Num() )
			p = array->properties[++array->index];
		else
			p = array->endPtr;
	}
}
gltfPropertyArray::~gltfPropertyArray()
{
	delete endPtr;
	properties.DeleteContents(true);
}
gltfPropertyArray::gltfPropertyArray( idLexer *Parser, bool AoS/* = true */)
	: parser( Parser ), iterating( true ), dirty( true ), index( 0 ), isArrayOfStructs(AoS)
{
	properties.AssureSizeAlloc(32, idListNewElement<gltfPropertyItem> );
	properties.SetNum(0);
	endPtr = new gltfPropertyItem();
	endPtr->array = this;
}

auto gltfPropertyArray::begin( )  {
	if ( iterating )
	{
		if ( isArrayOfStructs && !parser->PeekTokenString( "{" ) ) 	{
			if ( !parser->ExpectTokenString( "[" ) && parser->PeekTokenString( "{" ) )
				common->FatalError( "Malformed gltf array" );
		}else if ( !isArrayOfStructs && !parser->ExpectTokenString( "[") )
			common->FatalError( "Malformed gltf array" );

		properties.AssureSizeAlloc( properties.Num() + 1,idListNewElement<gltfPropertyItem> );
		gltfPropertyItem *start = properties[0];
		start->array = this; 
		if (isArrayOfStructs )
			parser->ParseBracedSection( start->item );
		else
		{
			idToken token;
			parser->ExpectAnyToken(&token);
			start->item = token;
		}
		iterating = parser->PeekTokenString( "," );
		if ( iterating )
		{
			properties.AssureSizeAlloc( properties.Num() + 1,idListNewElement<gltfPropertyItem> );
			parser->ExpectTokenString( "," );
		} 

		return Iterator{ this , start };
	}
	index = 0;
	return Iterator{ this ,properties[index]};
}
auto gltfPropertyArray::end( )  { 
	return Iterator{ this , endPtr};
}

void gltfItemArray::Parse(idLexer * lexer) {
	idToken token;
	bool parsing = true;
	lexer->ExpectTokenString( "{" );
	while ( parsing && lexer->ExpectAnyToken( &token ) ) 	{
		lexer->ExpectTokenString( ":" );
		for ( auto item : items ) 		{
			if ( item->Name( ) == token ) 			{
				lexer->ExpectAnyToken( &token );
				item->parse( token );
				break;
			}

		}
		parsing = lexer->PeekTokenString( "," );
		if ( parsing )
			lexer->ExpectTokenString( "," );
	}
	lexer->ExpectTokenString( "}" );
}

byte * gltfData::AddData(int size, int * bufferID/*=nullptr*/)
{
	if (totalChunks == -1 )
	{
		json  =  (byte*) Mem_ClearedAlloc(size);
		totalChunks++;
		jsonDataLength = size;
		return json;
	}

	int id = totalChunks;

	if (data == nullptr )
		data = ( byte ** ) Mem_ClearedAlloc( GLTF_MAX_CHUNKS * sizeof( byte * ) );
	data[totalChunks++] = (byte*) Mem_ClearedAlloc(size);
	
	if ( bufferID )
		*bufferID = id;

	return data[id];
}

bool gltfItem_uri::Convert( ) {
	//HVG_TODO
	// uri cache.
	//read data
	int length = fileSystem->ReadFile( item->c_str( ), NULL );
	idFile *file = fileSystem->OpenFileRead( item->c_str() );

	//create buffer
	gltfBuffer *buffer = data->Buffer( );
	buffer->parent = data;
	buffer->name = item->c_str( );
	buffer->byteLength = length;
	int bufferID = -1;
	byte * dest = data->AddData( length, &bufferID );

	if (file->Read( dest, length ) != length)
		common->FatalError("Could not read %s",item->c_str() );
	
	if (gltf_parseVerbose.GetBool() )
		common->Warning("gltf Uri %s loaded into buffer[ %i ]",buffer->name.c_str(),bufferID );

	//create bufferview
	//if bufferview is not set, this is an buffer.uri.
	//A bufferview should aready be defined if the buffer is used.
	if (bufferView != nullptr )
	{
		*bufferView = data->BufferViewList().Num();
		gltfBufferView * newBufferView = data->BufferView( );
		newBufferView->buffer = bufferID;
		newBufferView->byteLength = length;
		newBufferView->parent = data;
	}

	fileSystem->CloseFile( file );

	return false;
}

void gltfItem_mesh_primitive::parse( idToken &token )
{ 
	gltfItemArray prim;
	GLTFARRAYITEM( prim, attributes,	gltfItem_mesh_primitive_attribute );
	GLTFARRAYITEM( prim, indices,		gltfItem_integer );
	GLTFARRAYITEM( prim, material,		gltfItem_integer );
	GLTFARRAYITEM( prim, mode,			gltfItem_integer );
	GLTFARRAYITEM( prim, target,		gltfItem ); 
	GLTFARRAYITEM( prim, extensions,	gltfItem );
	GLTFARRAYITEM( prim, extras,		gltfItem );

	gltfPropertyArray array = gltfPropertyArray( parser );
	for ( auto &prop : array ) 	{
		idLexer lexer( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		lexer.LoadMemory( prop.item.c_str( ), prop.item.Size( ), "gltfItem_mesh_primitiveB", 0 );


		item->AssureSizeAlloc(item->Num() + 1,idListNewElement<gltfMesh_Primitive>);
		gltfMesh_Primitive *gltfMeshPrim =(*item)[item->Num() - 1];

		attributes->Set	( &gltfMeshPrim->attributes, &lexer );
		GLTFARRAYITEMREF( gltfMeshPrim, indices	);
		GLTFARRAYITEMREF( gltfMeshPrim, material );
		GLTFARRAYITEMREF( gltfMeshPrim, mode );
		GLTFARRAYITEMREF( gltfMeshPrim, target );
		GLTFARRAYITEMREF( gltfMeshPrim, extensions );
		GLTFARRAYITEMREF( gltfMeshPrim, extras );
		prim.Parse( &lexer );
		if ( gltf_parseVerbose.GetBool( ) )
			common->Printf( "%s", prop.item.c_str( ) );
	}
	parser->ExpectTokenString( "]" );
}

void gltfItem_mesh_primitive_attribute::parse( idToken &token ) {
	bool parsing = true;
	while ( parsing && parser->ExpectAnyToken( &token ) ) {
		
		item->AssureSizeAlloc( item->Num( ) + 1, idListNewElement<gltfMesh_Primitive_Attribute> );
		gltfMesh_Primitive_Attribute *attr = ( *item )[item->Num( ) - 1];
		parser->ExpectTokenString( ":" );
		attr->attributeSemantic = token;
		attr->bgfxType = GetAttributeEnum( attr->attributeSemantic.c_str(), &attr->elementSize );
		parser->ExpectAnyToken( &token );
		attr->accessorIndex = token.GetIntValue();
		parsing = parser->PeekTokenString( "," );
		if ( parsing )
			parser->ExpectTokenString( "," );
	}
	parser->ExpectTokenString( "}" );

	if ( gltf_parseVerbose.GetBool( ) )
		common->Printf( "%s", token.c_str( ) );
}

void gltfItem_integer_array::parse( idToken &token ) {
	
	parser->UnreadToken( &token );
	gltfPropertyArray array = gltfPropertyArray( parser, false );
	for ( auto &prop : array ) {
		idStr neg;
		int &value = item->Alloc( );
		value = prop.item.GetIntValue();

		if ( prop.item.type == TT_PUNCTUATION && prop.item == "-" ) {
			parser->ExpectTokenType( TT_NUMBER, 0, &prop.item );
			value = -( prop.item.GetIntValue( ) );
			neg = "-";
		} else if ( prop.item.type == TT_NUMBER ) {
			value = prop.item.GetIntValue( );
		} else
			common->FatalError( "parse error" );

		if ( gltf_parseVerbose.GetBool( ) )
			common->Printf( "%s%s", neg.c_str( ), prop.item.c_str( ) );
	}
	parser->ExpectTokenString( "]" );
}

void gltfItem_number_array::parse( idToken &token ) {

	parser->UnreadToken(&token);
	gltfPropertyArray array = gltfPropertyArray( parser, false );
	for ( auto &prop : array ) {
		idStr neg;
		double& value = item->Alloc( );
		if ( prop.item.type == TT_PUNCTUATION && prop.item == "-" ) {
			parser->ExpectTokenType( TT_NUMBER, 0, &prop.item );
			value = -(prop.item.GetDoubleValue());
			neg = "-";
		} else if ( prop.item.type == TT_NUMBER ) {
			value = prop.item.GetDoubleValue( );
		}else
			common->FatalError("parse error" );

		if ( gltf_parseVerbose.GetBool( ) )
			common->Printf( "%s%s", neg.c_str(),prop.item.c_str( ) );
	}
	parser->ExpectTokenString( "]" );
}

void gltfItem_accessor_sparse::parse( idToken &token ) {
	parser->Warning("%s is untested!", "gltfItem_accessor_sparse" );

	gltfItemArray sparse;
	GLTFARRAYITEM( sparse, count, gltfItem_integer );
	GLTFARRAYITEM( sparse, indices, gltfItem_accessor_sparse_indices);
	GLTFARRAYITEM( sparse, values, gltfItem_accessor_sparse_values );
	GLTFARRAYITEM( sparse, extensions, gltfItem );
	GLTFARRAYITEM( sparse, extras, gltfItem );

	GLTFARRAYITEMREF( item, count );
	indices->Set	( &item->indices, parser );
	values->Set		( &item->values, parser );
	GLTFARRAYITEMREF( item, extensions );
	GLTFARRAYITEMREF( item, extras );
	sparse.Parse( parser );

	if ( gltf_parseVerbose.GetBool( ) )
		common->Printf( "%s", token.c_str( ) );
}

void gltfItem_accessor_sparse_values::parse( idToken &token ) {
	parser->Warning( "%s is untested!", "gltfItem_accessor_sparse_values" );

	gltfItemArray values;
	GLTFARRAYITEM( values, bufferView, gltfItem_integer );
	GLTFARRAYITEM( values, byteOffset, gltfItem_integer );
	GLTFARRAYITEM( values, extensions, gltfItem );
	GLTFARRAYITEM( values, extras, gltfItem );

	GLTFARRAYITEMREF( item, bufferView );
	GLTFARRAYITEMREF( item, byteOffset );
	GLTFARRAYITEMREF( item, extensions );
	GLTFARRAYITEMREF( item, extras );

	if ( gltf_parseVerbose.GetBool( ) )
		common->Printf( "%s", token.c_str( ) );
}

void gltfItem_accessor_sparse_indices::parse( idToken &token ) {
	parser->Warning( "%s is untested!", "gltfItem_accessor_sparse_indices" );

	gltfItemArray indices;
	GLTFARRAYITEM( indices, bufferView, gltfItem_integer );
	GLTFARRAYITEM( indices, byteOffset, gltfItem_integer );
	GLTFARRAYITEM( indices, componentType, gltfItem_integer );
	GLTFARRAYITEM( indices, extensions, gltfItem );
	GLTFARRAYITEM( indices, extras, gltfItem );

	GLTFARRAYITEMREF( item, bufferView );
	GLTFARRAYITEMREF( item, byteOffset );
	GLTFARRAYITEMREF( item, componentType );
	GLTFARRAYITEMREF( item, extensions );
	GLTFARRAYITEMREF( item, extras );
	indices.Parse( parser );

	if ( gltf_parseVerbose.GetBool( ) )
		common->Printf( "%s", token.c_str( ) );
}

GLTF_Parser::GLTF_Parser()
	: parser( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES ) , buffersDone(false), bufferViewsDone( false ) { }

void GLTF_Parser::Parse_ASSET( idToken &token )
{
	idStr section;
	parser.ParseBracedSection( section );
	common->Printf( section.c_str( ) );
}
void GLTF_Parser::Parse_SCENE( idToken &token )
{
	common->Printf( " ^1 SCENE ^6 : ^8 %i",parser.ParseInt());
}
void GLTF_Parser::Parse_SCENES( idToken &token )
{
	gltfPropertyArray array = gltfPropertyArray(&parser);
	for (auto & prop : array )
		common->Printf("%s", prop.item.c_str() );
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_NODES( idToken &token ) 
{
	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
		common->Printf( "%s", prop.item.c_str( ) );
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_MATERIALS( idToken &token ) 
{
	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
		common->Printf( "%s", prop.item.c_str( ) );
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_MESHES( idToken &token ) 
{
	gltfItemArray mesh;
	GLTFARRAYITEM( mesh, primitives,	gltfItem_mesh_primitive ); // object
	GLTFARRAYITEM( mesh, weights,		gltfItem_number_array ); //number[1 - *]
	GLTFARRAYITEM( mesh, name,			gltfItem ); 
	GLTFARRAYITEM( mesh, extensions,	gltfItem );
	GLTFARRAYITEM( mesh, extras,		gltfItem );

	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array ) 	{
		idLexer lexer( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		lexer.LoadMemory( prop.item.c_str( ), prop.item.Size( ), "gltfMesh", 0 );

		gltfMesh *gltfMesh = currentAsset->Mesh( );

		primitives->Set	( &gltfMesh->primitives, &lexer);
		weights->Set	( &gltfMesh->weights, &lexer );
		GLTFARRAYITEMREF( gltfMesh,  name );
		GLTFARRAYITEMREF( gltfMesh,  extensions );
		GLTFARRAYITEMREF( gltfMesh,  extras );
		mesh.Parse( &lexer );
		if ( gltf_parseVerbose.GetBool( ) )
			common->Printf( "%s", prop.item.c_str( ) );
	}
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_TEXTURES( idToken &token )
{
	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
		common->Printf( "%s", prop.item.c_str( ) );
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_IMAGES( idToken &token )
{
	//reference impl
	gltfPropertyArray array = gltfPropertyArray( &parser );

	gltfItemArray propItems;
	auto uri		= new gltfItem_uri		("uri");		propItems.AddItemDef((parsable*)uri); 
	auto mimeType	= new gltfItem			("mimeType");	propItems.AddItemDef((parsable*)mimeType);
	auto bufferView = new gltfItem_integer	("bufferView");	propItems.AddItemDef((parsable*)bufferView );
	auto name		= new gltfItem			("name");		propItems.AddItemDef((parsable*)name);
	auto extensions = new gltfItem			("extensions");	propItems.AddItemDef((parsable*)extensions);
	auto extras		= new gltfItem			("extras");		propItems.AddItemDef((parsable*)extras);

	for ( auto &prop : array )
	{
		idLexer lexer( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		lexer.LoadMemory(prop.item.c_str(),prop.item.Size(),"gltfImage",0);

		gltfImage *image = currentAsset->Image();
		uri->Set		(&image->uri, &image->bufferView,currentAsset);
		mimeType->Set	(&image->mimeType);
		bufferView->Set	(&image->bufferView);
		name->Set		(&image->name);
		extensions->Set	(&image->extensions);
		extras->Set		(&image->extras);
		propItems.Parse	(&lexer);

		if ( gltf_parseVerbose.GetBool( ) )
			common->Printf( "%s", prop.item.c_str( ) );

		//automate..
		image->bgfxTexture.handle.idx = UINT16_MAX;
	}
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_ACCESSORS( idToken &token ) 
{
	gltfItemArray accessor;
	GLTFARRAYITEM( accessor, bufferView, gltfItem_integer );
	GLTFARRAYITEM( accessor, byteOffset, gltfItem_integer );
	GLTFARRAYITEM( accessor, componentType, gltfItem_integer );
	GLTFARRAYITEM( accessor, normalized, gltfItem_boolean );
	GLTFARRAYITEM( accessor, count, gltfItem_integer );
	GLTFARRAYITEM( accessor, type, gltfItem );
	GLTFARRAYITEM( accessor, max, gltfItem_number_array );
	GLTFARRAYITEM( accessor, min, gltfItem_number_array );
	GLTFARRAYITEM( accessor, sparse, gltfItem_accessor_sparse);
	GLTFARRAYITEM( accessor, name, gltfItem );
	GLTFARRAYITEM( accessor, extensions, gltfItem );
	GLTFARRAYITEM( accessor, extras, gltfItem );

	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto & prop : array )
	{
		idLexer lexer( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		lexer.LoadMemory( prop.item.c_str( ), prop.item.Size( ), "gltfAccessor", 0 );

		gltfAccessor * item = currentAsset->Accessor();
		GLTFARRAYITEMREF( item, bufferView);
		GLTFARRAYITEMREF( item, byteOffset);
		GLTFARRAYITEMREF( item, componentType);
		GLTFARRAYITEMREF( item, normalized);
		GLTFARRAYITEMREF( item, count);
		GLTFARRAYITEMREF( item, type );
		max->Set		( &item->max, &lexer );
		min->Set		( &item->min, &lexer );
		sparse->Set		( &item->sparse, &lexer );
		GLTFARRAYITEMREF( item, name);
		GLTFARRAYITEMREF( item, extensions);
		GLTFARRAYITEMREF( item, extras);
		accessor.Parse( &lexer );

		item->bgfxType = GetComponentTypeEnum( item->componentType, &item->typeSize );

		if ( gltf_parseVerbose.GetBool( ) )
			common->Printf( "%s", prop.item.c_str( ) );
	}
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_BUFFERVIEWS( idToken &token ) 
{	
	gltfItemArray bv;
	GLTFARRAYITEM( bv, buffer,		gltfItem_integer );
	GLTFARRAYITEM( bv, byteLength,	gltfItem_integer );
	GLTFARRAYITEM( bv, byteStride,	gltfItem_integer );
	GLTFARRAYITEM( bv, byteOffset,	gltfItem_integer );
	GLTFARRAYITEM( bv, target,		gltfItem_integer );
	GLTFARRAYITEM( bv, name,		gltfItem );
	GLTFARRAYITEM( bv, extensions,	gltfItem );
	GLTFARRAYITEM( bv, extras,		gltfItem );

	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
	{
		idLexer lexer( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		lexer.LoadMemory( prop.item.c_str( ), prop.item.Size( ), "gltfBufferView", 0 );

		gltfBufferView * gltfBV = currentAsset->BufferView();
		GLTFARRAYITEMREF( gltfBV, buffer	);
		GLTFARRAYITEMREF( gltfBV, byteLength);
		GLTFARRAYITEMREF( gltfBV, byteStride);
		GLTFARRAYITEMREF( gltfBV, byteOffset);
		GLTFARRAYITEMREF( gltfBV, target	);
		GLTFARRAYITEMREF( gltfBV, name		);
		GLTFARRAYITEMREF( gltfBV, extensions);
		GLTFARRAYITEMREF( gltfBV, extras	);
		bv.Parse(&lexer); 
		gltfBV->parent = currentAsset;

		if (gltf_parseVerbose.GetBool())
			common->Printf( "%s", prop.item.c_str( ) );
	}
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_SAMPLERS( idToken &token ) 
{
	gltfItemArray sampl;
	GLTFARRAYITEM( sampl, magFilter,	gltfItem_integer );
	GLTFARRAYITEM( sampl, minFilter,	gltfItem_integer );
	GLTFARRAYITEM( sampl, wrapS,		gltfItem_integer );
	GLTFARRAYITEM( sampl, wrapT,		gltfItem_integer );
	GLTFARRAYITEM( sampl, name,			gltfItem );
	GLTFARRAYITEM( sampl, extensions,	gltfItem );
	GLTFARRAYITEM( sampl, extras,		gltfItem );

	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
	{
		idLexer lexer( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		lexer.LoadMemory( prop.item.c_str( ), prop.item.Size( ), "gltfSampler", 0 );

		gltfSampler * gltfSampl = currentAsset->Sampler();
		GLTFARRAYITEMREF( gltfSampl, magFilter );
		GLTFARRAYITEMREF( gltfSampl, minFilter );
		GLTFARRAYITEMREF( gltfSampl, wrapS );
		GLTFARRAYITEMREF( gltfSampl, wrapT );
		GLTFARRAYITEMREF( gltfSampl, name		);
		GLTFARRAYITEMREF( gltfSampl, extensions);
		GLTFARRAYITEMREF( gltfSampl, extras	);
		sampl.Parse(&lexer);

		if (gltf_parseVerbose.GetBool())
			common->Printf( "%s", prop.item.c_str( ) );
	}
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_BUFFERS( idToken &token )
{
	gltfItemArray buf;
	GLTFARRAYITEM( buf, uri,		gltfItem_uri );
	GLTFARRAYITEM( buf, byteLength, gltfItem_integer );
	GLTFARRAYITEM( buf, name,		gltfItem );
	GLTFARRAYITEM( buf, extensions, gltfItem );
	GLTFARRAYITEM( buf, extras,		gltfItem );

	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array ) 	{
		idLexer lexer( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		lexer.LoadMemory( prop.item.c_str( ), prop.item.Size( ), "gltfBuffer", 0 );

		gltfBuffer *gltfBuf = currentAsset->Buffer( );
		gltfBuf->parent = currentAsset;

		uri->Set( &gltfBuf->uri, nullptr , currentAsset );
		GLTFARRAYITEMREF( gltfBuf, byteLength );
		GLTFARRAYITEMREF( gltfBuf, name );
		GLTFARRAYITEMREF( gltfBuf, extensions );
		GLTFARRAYITEMREF( gltfBuf, extras );
		buf.Parse( &lexer );
		if ( gltf_parseVerbose.GetBool( ) )
			common->Printf( "%s", prop.item.c_str( ) );
	}
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_ANIMATIONS( idToken &token )
{
	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
		common->Printf( "%s", prop.item.c_str( ) );
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_SKINS( idToken &token ) {
	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
		common->Printf( "%s \n", prop.item.c_str( ) );
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_EXTENSIONS_USED( idToken &token )
{
	gltfPropertyArray array = gltfPropertyArray( &parser,false );
	for ( auto &prop : array ) {
		gltfExtensionsUsed * ext = currentAsset->ExtensionsUsed( );
		ext->extension = prop.item;
		if ( gltf_parseVerbose.GetBool( ) )
			common->Printf( "%s", prop.item.c_str( ) );
	}
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_EXTENSIONS_REQUIRED( idToken &token ) {
	parser.ExpectTokenString( "[" );
	idStrList exts;
	idToken item;
	bool parsing = true;
	while ( parsing && parser.ExpectAnyToken( &item ) ) {
		if ( item.type != TT_STRING )
			common->FatalError( "malformed extensions_used array" );
		idStr &extension = exts.Alloc( );
		extension = item.c_str( );
		parsing = parser.PeekTokenString( "," );
		if ( parsing )
			parser.ExpectTokenString( "," );
	}
	parser.ExpectTokenString( "]" );
	for ( auto &out : exts )
		common->Printf( "%s", out.c_str( ) );
}

gltfProperty GLTF_Parser::ParseProp( idToken & token )
{
	parser.ExpectTokenString( ":" );
	gltfProperty prop = ResolveProp( token );

	bool skipping = false;
	if (!buffersDone || !bufferViewsDone)
	{
		if (prop == BUFFERS && !buffersDone)
		{
			Parse_BUFFERS( token );
			return prop;
		}
		if ( prop == BUFFERVIEWS && !bufferViewsDone ) {
			Parse_BUFFERVIEWS( token );
			return prop;
		}
		skipping = true;
		common->DPrintf( "Searching for buffer tag. Skipping %s.", token.c_str());
	}else 
	{
		if (( prop == BUFFERS && buffersDone)  || (prop == BUFFERVIEWS && bufferViewsDone ))
		{
			skipping = true;
			common->DPrintf( "Skipping %s , already done.", token.c_str( ) );
		}
	}

	if (skipping )
	{
		//1. search for {} scope.
		//2. search for [] scope.
		//3. singele token.

		idToken skipTok;
		int sectionsSkipped = 0;
		if (parser.PeekTokenString("{"))
			parser.SkipBracedSection( true,BRSKIP_BRACE, &sectionsSkipped );
		if ( !sectionsSkipped && parser.PeekTokenString( "[" ) )
			parser.SkipBracedSection( true, BRSKIP_BRACKET, &sectionsSkipped );
		if ( !sectionsSkipped )
			parser.ExpectAnyToken( &skipTok );

		return gltfProperty::INVALID;
	}
		
	switch ( prop )
	{
		case ASSET:
			Parse_ASSET( token );
			break;
		case SCENE:
			Parse_SCENE( token );
			break;
		case SCENES:
			Parse_SCENES( token );
			break;
		case NODES:
			Parse_NODES( token );
			break;
		case MATERIALS:
			Parse_MATERIALS( token );
			break;
		case MESHES:
			Parse_MESHES( token );
			break;
		case TEXTURES:
			Parse_TEXTURES( token );
			break;
		case IMAGES:
			Parse_IMAGES( token );
			break;
		case ACCESSORS:
			Parse_ACCESSORS( token );
			break;
		case BUFFERVIEWS:
			//Parse_BUFFERVIEWS( token );
			if ( !bufferViewsDone )
				common->FatalError( "Bufferviews should already be parsed!" );
			break;
		case SAMPLERS:
			Parse_SAMPLERS( token );
			break;
		case BUFFERS:
			if (!buffersDone)
				common->FatalError( "Buffers should already be parsed!" );
			break;
		case ANIMATIONS:
			Parse_ANIMATIONS( token );
			break;
		case SKINS:
			Parse_SKINS( token );
			break;
		case EXTENSIONS_USED:
			Parse_EXTENSIONS_USED( token );
			break;
		case EXTENSIONS_REQUIRED:
			Parse_EXTENSIONS_REQUIRED( token );
			break;
		default:
			common->FatalError("UnImplemented GLTF property : %s",token.c_str());
	}
	return prop;
}
gltfProperty GLTF_Parser::ResolveProp( idToken & token )
{
		if ( !idStr::Icmp( token.c_str( ), "asset"       ) )
		return gltfProperty::ASSET;
	else if ( !idStr::Icmp( token.c_str( ), "scene"       ) )
		return gltfProperty::SCENE;
	else if ( !idStr::Icmp( token.c_str( ), "scenes"      ) )
		return gltfProperty::SCENES;
	else if ( !idStr::Icmp( token.c_str( ), "nodes"       ) )
		return gltfProperty::NODES;
	else if ( !idStr::Icmp( token.c_str( ), "materials"   ) )
		return gltfProperty::MATERIALS;
	else if ( !idStr::Icmp( token.c_str( ), "meshes"      ) )
		return gltfProperty::MESHES;
	else if ( !idStr::Icmp( token.c_str( ), "textures"    ) )
		return gltfProperty::TEXTURES;
	else if ( !idStr::Icmp( token.c_str( ), "images"      ) )
		return gltfProperty::IMAGES;
	else if ( !idStr::Icmp( token.c_str( ), "accessors"   ) )
		return gltfProperty::ACCESSORS;
	else if ( !idStr::Icmp( token.c_str( ), "bufferViews" ) )
		return gltfProperty::BUFFERVIEWS;
	else if ( !idStr::Icmp( token.c_str( ), "samplers"    ) )
		return gltfProperty::SAMPLERS;
	else if ( !idStr::Icmp( token.c_str( ), "buffers"     ) )
		return gltfProperty::BUFFERS;
	else if ( !idStr::Icmp( token.c_str( ), "animations"  ) )
		return gltfProperty::ANIMATIONS;
	else if ( !idStr::Icmp( token.c_str( ), "skins" ) )
		return gltfProperty::SKINS;
	else if ( !idStr::Icmp( token.c_str( ), "extensionsused" ) )
		return gltfProperty::EXTENSIONS_USED;
	else if ( !idStr::Icmp( token.c_str( ), "extensionsrequired" ) )
		return gltfProperty::EXTENSIONS_REQUIRED;

	return gltfProperty::INVALID;
}

bool GLTF_Parser::loadGLB(idStr filename )
{
	idFile * file = fileSystem->OpenFileRead( filename );

	if ( file->Length() < 20 ) {
		common->FatalError("Too short data size for glTF Binary." );
		return false;
	}
	idStr gltfMagic("glTF");
	unsigned char fileMagic[5];
	
	file->Read((void*)&fileMagic,4 );
	fileMagic[4]=0;
	if (gltfMagic.Icmp( (const char*)&fileMagic ) == 0)
	{
		common->Printf("reading %s",filename.c_str() );
	} else {
		common->Error( "invalid magic" );
		return false;
	}

	unsigned int version = 0;	// 4 bytes
	unsigned int length = 0;	// 4 bytes

	//HVG_TODO
	//handle 0 bin chunk -> size is chunk[0].size  + 20;
	file->ReadUnsignedInt( version );
	file->ReadUnsignedInt( length );
	length -= 12; // header size

	unsigned int chunk_type=0;	// 4 bytes
	unsigned int chunk_length=0;	// 4 bytes
	byte * data = nullptr;
	gltfData *dataCache = gltfData::Data( );
	currentAsset = dataCache;

	int chunkCount = 0;
	while ( length ) {
		unsigned int prev_length = chunk_length;
		length -= file->ReadUnsignedInt( chunk_length );
		length -= file->ReadUnsignedInt( chunk_type );
		
		data = dataCache->AddData(chunk_length);
		dataCache->FileName(filename);

		int read = file->Read((void*)data, chunk_length );
		if (read != chunk_length)
			common->FatalError("Could not read full chunk (%i bytes) in file %s",chunk_length, filename );
		length -= read;
		if (chunk_type == gltfChunk_Type_JSON)
		{
			currentFile = filename + " [JSON CHUNK]";
			parser.LoadMemory((const char*)data,chunk_length,"gltfJson",0);
		}else if ( !chunkCount )
			common->FatalError("first chunk was not a json chunk");
		else {
			common->Printf("BINCHUNK i", chunk_length );
		}
		if (chunkCount++ && length )
			common->FatalError("corrupt glb file." );
	}

	Parse();
	return true;
}

bool GLTF_Parser::Parse( ) {
	bool parsing = true;
	parser.ExpectTokenString( "{" );
	while ( parsing && parser.ExpectAnyToken( &token ) ) {
		if ( token.type != TT_STRING )
			common->FatalError( "Expected an \"string\" " );

		common->Printf( token.c_str( ) );
		gltfProperty prop = ParseProp( token );

		if (( prop == BUFFERS && !buffersDone ))
		{
			parser.Reset();
			parser.ExpectTokenString( "{" );
			buffersDone = true;
			continue;
		}
		if ((prop == BUFFERVIEWS && !bufferViewsDone)) 
		{
			parser.Reset( );
			parser.ExpectTokenString( "{" );
			bufferViewsDone = true;
			continue;
		}
		common->Printf( "\n" );
		parsing = parser.PeekTokenString( "," );
		if ( parsing )
			parser.ExpectTokenString( "," );
		else
			parser.ExpectTokenString( "}" );
	}
	//parser should be at end.
	parser.ReadToken( &token );
	if ( parser.EndOfFile( ) )
		common->Printf( "%s ^2loaded", currentFile.c_str( ) );
	else
		common->FatalError( "%s not fully loaded.", currentFile );
	
	buffersDone = false;
	bufferViewsDone = false;
	return true;
}
	
bool GLTF_Parser::Load(idStr filename )
{
	//next line still has to be fixed.
	//gfx is not updated on command
	common->SetRefreshOnPrint( true );

	
	currentFile = filename;
 	if ( filename.CheckExtension( ".glb" ) ) {
		if ( !loadGLB( filename ) )
			return false;
	}
	else if ( filename.CheckExtension( ".gltf" ) ) {
		int length = fileSystem->ReadFile(filename,NULL);
		gltfData * data = gltfData::Data();
		data->FileName(filename);
		byte* dataBuff = data->AddData(length);
		currentAsset = data;
		
		idFile* file = fileSystem->OpenFileRead( filename );
		if ( file->Read(dataBuff,length)!=length)
			common->FatalError("Cannot read file, %s",filename.c_str() );

		fileSystem->CloseFile(file);

		if ( !parser.LoadMemory((const char*)dataBuff,length,"GLTF_ASCII_JSON",0))
			return false;
		
		Parse();

	}else
		return false;

	parser.Reset();
	parser.FreeSource();
	common->SetRefreshOnPrint( false );
	CreateBgfxData();
	return true;
}

void GLTF_Parser::Init( ) {
	static auto * thisPtr = this;
	cmdSystem->AddCommand( "gltf_XloadFile", []( const idCmdArgs &args )
		-> auto {
		if ( args.Argc( ) != 2 )
			common->Printf( "use: gltf_XloadFile <file>" );
		else
			thisPtr->Load( args.Argv( 1 ) );
	}, CMD_FL_SYSTEM, "Loads an gltf file", idCmdSystem::ArgCompletion_GltfName );

}

void GLTF_Parser::CreateBgfxData( )
{

	//buffers
	for ( auto mesh : currentAsset->MeshList( ) ) 
	{
		for ( auto prim : mesh->primitives ) 
		{
			//vertex indices accessor
			gltfAccessor * accessor = currentAsset->AccessorList( )[prim->indices];
			gltfBufferView *bv = currentAsset->BufferViewList( )[accessor->bufferView];
			gltfData *data = bv->parent;

			gltfBuffer *buff = data->BufferList( )[bv->buffer];
			uint idxDataSize = sizeof( uint ) * accessor->count;
			uint * indices = ( uint *)Mem_ClearedAlloc( idxDataSize );
			idFile_Memory idxBin = idFile_Memory( "gltfChunkIndices", ( const char * ) ((data->GetData( bv->buffer ) + bv->byteOffset + accessor->byteOffset )), bv->byteLength );
			for ( int i = 0; i < accessor->count; i++ ) {
				idxBin.Read( ( void * ) ( &indices[i] ), accessor->typeSize );
				if ( bv->byteStride )
					idxBin.Seek( bv->byteStride - accessor->typeSize, FS_SEEK_CUR );
			}
			prim->indexBufferHandle = bgfx::createIndexBuffer(bgfx::copy( indices,idxDataSize ),BGFX_BUFFER_INDEX32);

			Mem_Free( indices );

			//vertex attribs  
			PosColorVertex * vtxData = NULL;
			uint vtxDataSize = 0;
			bgfx::VertexLayout vtxLayout;
			vtxLayout.begin( );
			for ( auto & attrib : prim->attributes )
			{
				gltfAccessor * attrAcc = currentAsset->AccessorList( )[attrib->accessorIndex];
				gltfBufferView *attrBv = currentAsset->BufferViewList( )[attrAcc->bufferView];
				gltfData *attrData = attrBv->parent;
				gltfBuffer *attrbuff = attrData->BufferList( )[attrBv->buffer];

				idFile_Memory bin = idFile_Memory( "gltfChunkPosition", ( const char * )(( attrData->GetData( attrBv->buffer ) + attrBv->byteOffset + attrAcc->byteOffset )) , attrBv->byteLength );

				if ( vtxData == nullptr ) {
					vtxDataSize = sizeof( PosColorVertex ) * attrAcc->count;
					vtxData = ( PosColorVertex * ) Mem_ClearedAlloc( vtxDataSize );
				}

				if ( attrib->bgfxType == bgfx::Attrib::Enum::Position ) {
					for ( int i = 0; i < attrAcc->count; i++ ) {
						bin.Read( ( void * ) ( &vtxData[i].x ), attrAcc->typeSize );
						bin.Read( ( void * ) ( &vtxData[i].y ), attrAcc->typeSize );
						bin.Read( ( void * ) ( &vtxData[i].z ), attrAcc->typeSize );
						if ( attrBv->byteStride )
							bin.Seek( attrBv->byteStride - (3 * attrAcc->typeSize), FS_SEEK_CUR );

						idRandom rnd(i);
						int r = rnd.RandomInt(255), g = rnd.RandomInt(255), b = rnd.RandomInt(255);

						vtxData[i].abgr = 0xff000000 + ( b << 16 ) + ( g << 8 ) + r;
					}
					vtxLayout.add( attrib->bgfxType, attrib->elementSize,bgfx::AttribType::Float, attrAcc->normalized );
				}
			}

			vtxLayout.add( bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true );
			vtxLayout.end();
			if ( vtxData != NULL ) {
				prim->vertexBufferHandle = bgfx::createVertexBuffer(
					bgfx::copy( vtxData, vtxDataSize ), vtxLayout );

				Mem_Free( vtxData );
			}else
				common->FatalError("Failed to read vertex info" );
		}
	}
	//textures
	for ( auto &image : currentAsset->ImageList( ) ) 
	{
		if(image->bgfxTexture.handle.idx == UINT16_MAX )
		{
			gltfBufferView *bv = currentAsset->BufferViewList( )[image->bufferView];
			gltfData *data = bv->parent;
			gltfBuffer *buff = data->BufferList( )[bv->buffer];
			
			image->bgfxTexture = bgfxImageLoad(data->GetData(bv->buffer) + bv->byteOffset,bv->byteLength );
		}
	}
}

gltfData::~gltfData() {
	//hvg_todo
	//delete data, not only pointer
	common->Warning("GLTF DATA NOT FREED" );
	if (data)
		delete[] data;
}

#undef GLTFARRAYITEM
#undef GLTFARRAYITEMREF
