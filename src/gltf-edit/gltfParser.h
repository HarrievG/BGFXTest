#pragma once
#include "idFramework/Common.h"

enum gltfProperty {
	INVALID, 
	ASSET,
	SCENE,
	SCENES,
	NODES,
	MATERIALS,
	MESHES,
	TEXTURES,
	IMAGES,
	ACCESSORS,
	BUFFERVIEWS,
	SAMPLERS,
	BUFFERS
};

class GLTF_Parser 
{
public:
	void Parse_ASSET( idToken &token ) ;
	void Parse_SCENE( idToken &token ) ;
	void Parse_SCENES( idToken &token ) ;
	void Parse_NODES( idToken &token ) ;
	void Parse_MATERIALS( idToken &token ) ;
	void Parse_MESHES( idToken &token ) ;
	void Parse_TEXTURES( idToken &token ) ;
	void Parse_IMAGES( idToken &token ) ;
	void Parse_ACCESSORS( idToken &token ) ;
	void Parse_BUFFERVIEWS( idToken &token ) ;
	void Parse_SAMPLERS( idToken &token ) ;
	void Parse_BUFFERS( idToken &token ) ;

	bool PropertyIsAOS( );
	gltfProperty ParseProp( idToken &token );
	gltfProperty ResolveProp( idToken &token );

	GLTF_Parser();
	void Init();
	bool Load(idStr filename );
private:
	idLexer	parser;
	idToken	token;
};