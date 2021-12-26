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

class gltfItem
{
public:
	template<typename T>
	gltfItem(idStr name, T * type ) : item( nullptr ) { item = static_cast< void * >( new T ); }
	template <typename T>
	void Set(T * item ) { item = static_cast<void*>(item); }
	template <typename T>
	T* Get( ) { return static_cast< T * >( item ); }
	idStr name;
	void * item;
};

class gltfItemArray
{
public:
	gltfItemArray( ) { };
	void AddItemDef( gltfItem * item ) {items.Alloc() = item; }
	bool iterating;
	bool dirty;
	int index;
	idLexer * parser;
	idList<gltfItem*> items;
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

	
	template<typename T,typename ... A>
	int GetAttribs(idList<T> & idList, A ... attribs  )
	{

	}
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

	template<typename T, typename ... A>
	int internalGetAttribs( idList<T> *idList, std::tuple<idStr,A...> &tuple ) 
	{

	}
	template<typename T, typename ... A>
	int internalGetAttribs( idList<T> *idList, std::tuple<> &tuple ) 	{ }
	idLexer	parser;
	idToken	token;
	idStr currentFile;
};