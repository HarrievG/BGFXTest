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

class gltfBufferView {
public: 
	gltfBufferView( ) : buffer(-1),byteLength(-1),byteStride(-1),byteOffset(-1),target(-1){};
	int buffer;
	int byteLength;
	int byteStride;
	int byteOffset;
	int target;
	idStr name;
	idStr extensions;
	idStr extras;
};

class gltfBuffer {
public:
	gltfBuffer( ) : bytelength(-1) { };
	idStr uri;
	int bytelength;
	idStr name;
	idStr extensions;
	idStr extras;
};

class gltfSampler {
public:
	gltfSampler( ) : magFilter(-1),minFilter(-1) { };
	int magFilter;
	int minFilter;
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

class gltfData
{
	friend class gltfCache;
public:
	gltfData( ) : json( nullptr ), data( nullptr ) { };
	~gltfData();
	byte * json;
	byte * data;
};


//// For these to function you need to add an private idList<gltf{name}*> {target}
#define GLTFCACHEITEM(name,target) \
gltf##name * name ( ) { target.AssureSizeAlloc( target.Num()+1,idListNewElement<gltf##name>); return target[target.Num()-1];} \
const idList<gltf##name*> & Get##name##List() { return target; }
class gltfCache {
public:
	gltfCache( ) { }
	void clear(){
		images.DeleteContents( true );
	}
	GLTFCACHEITEM( Sampler, samplers )
	GLTFCACHEITEM( Buffer, buffers )
	GLTFCACHEITEM( BufferView, bufferViews )
	GLTFCACHEITEM( Data, assetData )
	GLTFCACHEITEM( Image, images )
private:
	idList<gltfImage*>			images;
	idList<gltfData*>			assetData;
	idList<gltfSampler*>		samplers;
	idList<gltfBufferView *>	bufferViews;
	idList<gltfBuffer*>			buffers;
};
#undef GLTFCACHEITEM

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

//gltf data types
// string type does not have suffix for easy of use
// all others should use the gltfItemClass macro.
class gltfItem : public parsable, public parseType<idStr>
{
public:
	gltfItem( idStr Name) : name( Name ){ item = nullptr; }
	virtual void parse( idToken &token ) { *item = token; };
	virtual idStr &Name( ) {return name;}
private:
	idStr name;
};

//helper macro to define more gltf data types
#define gltfItemClass(type,function) \
class gltfItem_##type : public parsable, public parseType<type>				\
{public:																	\
	gltfItem_##type( idStr Name ) : name( Name ){ item = nullptr; }			\
	virtual void parse( idToken &token ) { function }						\
	virtual idStr &Name( ) { return name; }									\
private:																	\
	idStr name;}

gltfItemClass(int, *item = token.GetIntValue( ); );
/////////////////////////////////////////////////////////////////////////////

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
	void CreateTextures( );
	idLexer	parser;
	idToken	token;
	idStr currentFile;
};