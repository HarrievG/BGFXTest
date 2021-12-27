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
	EXTENSIONS_USED,
	EXTENSIONS_REQUIRED
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

template<class T>
class parseType {
public:
	void Set(T * type ) { item = type; }
	T* item;
};

class gltfItem : public parsable, public parseType<idStr>
{
public:
	gltfItem( idStr Name) : name( Name ){ }
	virtual void parse( idToken &token ) { *item = token; };
	virtual idStr &Name( ) {return name;}
private:
	idStr name;
};

class gltfItemInt : public parsable, public parseType<int>
{
public:
	gltfItemInt( idStr Name ) : name( Name ){ }
	virtual void parse( idToken &token ) { *item = token.GetIntValue(); };
	virtual idStr &Name( ) { return name; }
private:
	idStr name;
};

class gltfItemArray
{
public:
	~gltfItemArray( ) { items.DeleteContents(true); }
	gltfItemArray( ){ };
	void AddItemDef( parsable *item ) { items.Alloc( ) = item;}
	void Parse(idLexer * lexer );
private:
	idList<parsable*> items;
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
	void Parse_EXTENSIONS_REQUIRED( idToken &token );
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