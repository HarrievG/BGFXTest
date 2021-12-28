#include "gltfParser.h"
#include <FileSystem.h>


static const unsigned int gltfChunk_Type_JSON =  0x4E4F534A; //1313821514
static const unsigned int gltfChunk_Type_BIN =  0x004E4942; //5130562

static gltfCache localCache;
gltfCache * gltfAssetCache = &localCache;

//void gltfCache::clear()
//{
//	images.DeleteContents(true);
//}


idCVar gltf_parseVerbose( "gltf_parseVerbose", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "print gltf json data while parsing" );

void gltfPropertyArray::Iterator::operator ++( ) {
	if ( array->iterating )
	{
		p = array->properties[array->properties.Num( ) - 1];
		p->array = array;
		array->parser->ParseBracedSection( p->item );
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
gltfPropertyArray::gltfPropertyArray( idLexer *Parser ) 
	: parser( Parser ), iterating( true ), dirty( true ), index( 0 )
{
	properties.AssureSizeAlloc(1024, idListNewElement<gltfPropertyItem> );
	properties.SetNum(0);
	endPtr = new gltfPropertyItem();
	endPtr->array = this;
}
auto gltfPropertyArray::begin( )  {
	if ( iterating )
	{
		if ( !parser->PeekTokenString( "{" ) ) 	{
			if ( !parser->ExpectTokenString( "[" ) && parser->PeekTokenString( "{" ) )
				common->FatalError( "Malformed gltf array" );
		}

		properties.AssureSizeAlloc( properties.Num() + 1,idListNewElement<gltfPropertyItem> );
		gltfPropertyItem *start = properties[0];
		start->array = this; 
		parser->ParseBracedSection( start->item );
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

GLTF_Parser::GLTF_Parser()
	: parser( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES ) { }

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
	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
		common->Printf( "%s", prop.item.c_str( ) );
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
	gltfPropertyArray array = gltfPropertyArray( &parser );

	gltfItemArray propItems;
	auto uri		= new gltfItem		("uri");		propItems.AddItemDef((parsable*)uri); 
	auto mimeType	= new gltfItem		("mimeType");	propItems.AddItemDef((parsable*)mimeType);
	auto bufferView = new gltfItemInt	("bufferView");	propItems.AddItemDef((parsable*)bufferView );
	auto name		= new gltfItem		("name");		propItems.AddItemDef((parsable*)name);
	auto extensions = new gltfItem		("extensions");	propItems.AddItemDef((parsable*)extensions);
	auto extras		= new gltfItem		("extras");		propItems.AddItemDef((parsable*)extras);

	for ( auto &prop : array )
	{
		idLexer lexer( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		lexer.LoadMemory(prop.item.c_str(),prop.item.Size(),"gltfImage",0);

		gltfImage *image = gltfAssetCache->Image();
		image = new gltfImage();
		uri->Set		(&image->uri);
		mimeType->Set	(&image->mimeType);
		bufferView->Set	(&image->bufferView);
		name->Set		(&image->name);
		extensions->Set	(&image->extensions);
		extras->Set		(&image->extras);
		propItems.Parse	(&lexer);
		if ( gltf_parseVerbose.GetBool( ) )
			common->Printf( "%s", prop.item.c_str( ) );
	}
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_ACCESSORS( idToken &token ) 
{
	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto & prop : array )
	{
		common->Printf( "%s", prop.item.c_str( ) );
	}
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_BUFFERVIEWS( idToken &token ) 
{
	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
		common->Printf( "%s", prop.item.c_str( ) );
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_SAMPLERS( idToken &token ) 
{
	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
		common->Printf( "%s", prop.item.c_str( ) );
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_BUFFERS( idToken &token )
{
	gltfPropertyArray array = gltfPropertyArray( &parser );
	for ( auto &prop : array )
		common->Printf( "%s", prop.item.c_str( ) );
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
	parser.ExpectTokenString( "[" );
	idStrList exts;
	idToken item;
	bool parsing = true;
	while (parsing && parser.ExpectAnyToken(&item))
	{
		if ( item.type != TT_STRING )
			common->FatalError( "malformed extensions_used array" );
		idStr &extension = exts.Alloc( );
		extension = item.c_str( );
		parsing = parser.PeekTokenString(",");
		if ( parsing )
			parser.ExpectTokenString(",");
	}
	parser.ExpectTokenString( "]" );
	for (auto & out : exts )
		common->Printf("%s",out.c_str() );
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
			Parse_BUFFERVIEWS( token );
			break;
		case SAMPLERS:
			Parse_SAMPLERS( token );
			break;
		case BUFFERS:
			Parse_BUFFERS( token );
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

bool GLTF_Parser::PropertyIsAOS()
{
	if (!parser.PeekTokenString( "{" ))
	{
		if (!parser.ExpectTokenString( "[" ) && parser.PeekTokenString( "{" ))
			common->FatalError("Malformed gltf" );
		return true;
	}
	return false;
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
	byte * data;
	gltfData *dataCache = gltfAssetCache->Data( );

	int chunkCount = 0;
	while ( length ) {
		unsigned int prev_length = chunk_length;
		length -= file->ReadUnsignedInt( chunk_length );
		length -= file->ReadUnsignedInt( chunk_type );
		
		if (chunkCount)
			dataCache->json = data;

		data = new byte[chunk_length];

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
			dataCache->data = data;
		}
		if (chunkCount++ && length )
			common->FatalError("corrupt glb file." );
	}

	Parse();
	return true;
}

bool GLTF_Parser::Parse()
{
	bool parsing = true;
	parser.ExpectTokenString( "{" );
	while ( parsing && parser.ExpectAnyToken( &token ) ) {
		if ( token.type != TT_STRING )
			common->FatalError( "Expected an \"string\" " );

		common->Printf( token.c_str( ) );
		ParseProp( token );
		common->Printf("\n" );
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
	else {
		if ( !parser.LoadFile( filename ) )
			return false;
		Parse();
	}

	parser.Reset();
	parser.FreeSource();
	common->SetRefreshOnPrint( false );
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

//gltfData::gltfData( size_t jsonSize, size_t datasize ) {
//	data = new byte[datasize];
//	json = new byte[jsonSize];
//}


gltfData::~gltfData() {
	if (data)
		delete[] data;
	if (json)
		delete[] json;
}