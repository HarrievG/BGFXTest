#pragma once
#include "idFramework/Common.h"
#include "idFramework/idlib/containers/StrList.h"
#include <functional>
#include "gltfProperties.h"

/////////////////////////////////////////////////////////////////////////////
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
	GLTFCACHEITEM( Texture, textures )
	GLTFCACHEITEM( Accessor, accessors )
	GLTFCACHEITEM( ExtensionsUsed, extensionsUsed )
private:
	idList<gltfImage*>				images;
	idList<gltfData*>				assetData;
	idList<gltfSampler*>			samplers;
	idList<gltfBufferView *>		bufferViews;
	idList<gltfBuffer*>				buffers;
	idList<gltfTexture*>			textures;
	idList<gltfAccessor *>			accessors;
	idList<gltfExtensionsUsed *>	extensionsUsed;
};
extern gltfCache *gltfAssetCache;
#undef GLTFCACHEITEM
/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////
//gltf data types
// gltfItem  - string type
class gltfItem : public parsable, public parseType<idStr>
{
public:
	gltfItem( idStr Name) : name( Name ){ item = nullptr; }
	virtual void parse( idToken &token ) { *item = token; };
	virtual idStr &Name( ) {return name;}
private:
	idStr name;
};

class gltfItem_uri : public parsable, public parseType<idStr> {
public:
	gltfItem_uri( idStr Name ) : name( Name ) { item = nullptr; }
	virtual void parse( idToken &token ) { *item = token; Convert(); };
	virtual idStr &Name( ) { return name; }
	void Set( idStr *type,int * targetBufferview,gltfData* dataDestination ) { parseType::Set(type); bufferView = targetBufferview; data = dataDestination; }
	// read data from uri file, and push it at end of current data buffer for this GLTF File
	// bufferView will be set accordingly to the generated buffer.
	bool Convert( );
private:
	idStr name;
	int * bufferView;
	gltfData *  data;
};

//helper macro to define more gltf data types
#define gltfItemClass(className,type,function) \
class gltfItem_##className : public parsable, public parseType<type>				\
{public:																	\
	gltfItem_##className( idStr Name ) : name( Name ){ item = nullptr; }			\
	virtual void parse( idToken &token ) { function }						\
	virtual idStr &Name( ) { return name; }									\
private:																	\
	idStr name;}

////
gltfItemClass(integer, int, *item = token.GetIntValue( ); );
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
	gltfPropertyArray( idLexer *Parser,bool AoS = true );
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
	bool isArrayOfStructs;
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
	
	static void ResolveUri( const idStr &uri );

	GLTF_Parser();
	void Init();
	bool Parse();
	bool Load(idStr filename );
	bool loadGLB(idStr filename );

	//current/last loaded gltf asset 
	gltfData *currentAsset;
private:
	void CreateTextures( );
	
	
	idLexer	parser;
	idToken	token;
	idStr currentFile;
};