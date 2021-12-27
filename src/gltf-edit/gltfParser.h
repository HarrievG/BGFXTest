#pragma once
#include "idFramework/Common.h"
#include "idFramework/idlib/containers/StrList.h"
#include <functional>

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
	BUFFERS,
	ANIMATIONS,
	SKINS,
	EXTENSIONS_USED
};

class gltfImage {
public:
	gltfImage( ) : bufferView(-1){}
	idStr	uri;
	idStr	mimeType;
	int		bufferView;
	idStr	name;
	idStr	extensions;
	idStr	extras;
};


class gltfCache {
public:
	gltfCache( ) { }
	void clear(){
		images.DeleteContents( true );
	}
	idList<gltfImage*> images;
};
extern gltfCache * gltfAssetCache;

struct parsable {
public:
	virtual void parse(idToken & token )=0;
	virtual idStr &Name( ) = 0;
};

class gltfItem : public parsable
{
public:
	gltfItem(idStr Name, idStr * type ) : name(Name),item( type ) {}
	virtual void parse( idToken &token ) { *item = token; };
	virtual idStr &Name( ) {return name;}
private:
	idStr name;
	idStr *item;
};

class gltfItemInt : public parsable
{
public:
	gltfItemInt( idStr Name, int * type ) : name( Name ), item( type ) { }
	virtual void parse( idToken &token ) { *item = token.GetIntValue(); };
	virtual idStr &Name( ) { return name; }
private:
	idStr name;
	int *item;
};

class gltfItemArray
{
public:
	gltfItemArray( idLexer & Lexer) : lexer(Lexer){ };
	void AddItemDef( parsable * item ) {/*items.Alloc() = item; */P.Alloc() = item;}
	void Parse( );
private:
	idLexer & lexer;
	idList<parsable*> P;
};

class gltfPropertyArray;
class gltfPropertyItem 
{
public:
	gltfPropertyItem( ) : array(nullptr){ }
	gltfPropertyArray * array;
	idStr item; 
};

class gltfPropertyArray
{
public:
	gltfPropertyArray( idLexer *Parser );
	~gltfPropertyArray( );
	struct Iterator {
		gltfPropertyArray * array;
		gltfPropertyItem *p;
		gltfPropertyItem &operator*( ) {return *p;}
		bool operator != ( Iterator &rhs ) { 
			return p != rhs.p; 
		}
		void operator ++( );
	};	
	auto begin( );
	auto end( );
	bool iterating;
	bool dirty;
	int index;
	idLexer * parser;
	idList<gltfPropertyItem*> properties;
	gltfPropertyItem * endPtr;
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
	void Parse_ANIMATIONS( idToken & token );
	void Parse_SKINS( idToken & token );
	void Parse_EXTENSIONS_USED( idToken &token );
	bool PropertyIsAOS( );
	gltfProperty ParseProp( idToken &token );
	gltfProperty ResolveProp( idToken &token );

	GLTF_Parser();
	void Init();
	bool Parse();
	bool Load(idStr filename );
	bool loadGLB(idStr filename );
private:
	idLexer	parser;
	idToken	token;
	idStr currentFile;
};