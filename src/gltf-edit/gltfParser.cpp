#include "gltfParser.h"


void gltfPropertyArray::Iterator::operator ++( ) {
	if ( array->dirty )
	{
		auto next = array->properties.Alloc( );
		array->parser.ParseBracedSection( next.item );
		array->dirty = array->parser.PeekTokenString( "," );
		if ( array->dirty )
			array->parser.ExpectTokenString( "," );
	}else
		++p;
}

auto gltfPropertyArray::begin( )  { 
	if ( dirty )
	{
		if ( !parser.PeekTokenString( "{" ) ) 	{
			if ( !parser.ExpectTokenString( "[" ) && parser.PeekTokenString( "{" ) )
				common->FatalError( "Malformed gltf array" );
		}else
			common->FatalError( "Malformed gltf array" );

		auto & start = properties.Alloc( );
		start.array = this;
		parser.ParseBracedSection( start.item );
		dirty = parser.PeekTokenString( "," );
		if ( dirty )
		{
			parser.ExpectTokenString( "," );
			auto next = properties.Alloc( );
			return Iterator{ this , &next };
		}else
			return Iterator{ this , &start };
	}
	return Iterator{ this ,properties.begin( ).p};
}

auto gltfPropertyArray::end( )  { 
	return Iterator{ this ,properties.end( ).p };
}

GLTF_Parser::GLTF_Parser()
	: parser( LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS )
{

}

void GLTF_Parser::Parse_ASSET( idToken &token )
{
	parser.ExpectTokenString( ":" );
	idStr section;
	parser.ParseBracedSection( section );
	common->Printf( section.c_str( ) );
}
void GLTF_Parser::Parse_SCENE( idToken &token )
{
	parser.ExpectTokenString( ":" );
	common->Printf( " ^1 SCENE ^6 : ^8 %i",parser.ParseInt());
}
void GLTF_Parser::Parse_SCENES( idToken &token )
{
	parser.ExpectTokenString( ":" );
	gltfPropertyArray array = gltfPropertyArray(parser);
	for (auto & prop : array )
		common->Printf("%s", prop.item.c_str() );
	parser.ExpectTokenString( "]" );
}
void GLTF_Parser::Parse_NODES( idToken &token ) { }
void GLTF_Parser::Parse_MATERIALS( idToken &token ) { }
void GLTF_Parser::Parse_MESHES( idToken &token ) { }
void GLTF_Parser::Parse_TEXTURES( idToken &token ) { }
void GLTF_Parser::Parse_IMAGES( idToken &token ) { }
void GLTF_Parser::Parse_ACCESSORS( idToken &token ) { }
void GLTF_Parser::Parse_BUFFERVIEWS( idToken &token ) { }
void GLTF_Parser::Parse_SAMPLERS( idToken &token ) { }
void GLTF_Parser::Parse_BUFFERS( idToken &token ) { }

gltfProperty GLTF_Parser::ParseProp( idToken & token )
{
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

bool GLTF_Parser::Load(idStr filename )
{
	if ( !parser.LoadFile( filename ) ) {
		return false;
	}

	parser.ExpectTokenString("{");
	while (parser.ExpectAnyToken(&token))
	{
		if (token.type != TT_STRING )
			common->FatalError("Expected an \"string\" " );
		common->Printf( token.c_str( ) );
		ParseProp( token );
		parser.ExpectTokenString( "," );
	}
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