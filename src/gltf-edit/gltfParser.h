#pragma once
#include "idFramework/Common.h"
#include "idFramework/idlib/containers/StrList.h"
#include <functional>
#include "gltfProperties.h"

#pragma region GLTF Types parsing

#pragma region Parser interfaces
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
#pragma endregion 

#pragma region helper macro to define gltf data types with extra parsing context forced to be implemented externally
#define gltfItemClassParser(className,ptype)											\
class gltfItem_##className : public parsable, public parseType<ptype>					\
{public:																				\
	gltfItem_##className( idStr Name ) : name( Name ){ item = nullptr; }				\
	virtual void parse( idToken &token );												\
	virtual idStr &Name( ) { return name; }												\
	void Set( ptype *type, idLexer *lexer ) { parseType::Set( type ); parser = lexer; }	\
private:																				\
	idStr name;																			\
	idLexer *parser;}
#pragma endregion 

gltfItemClassParser( mesh_primitive,				idList<gltfMesh_Primitive *>);
gltfItemClassParser( mesh_primitive_attribute,		idList<gltfMesh_Primitive_Attribute *> );
gltfItemClassParser( integer_array,					idList<int>);
gltfItemClassParser( number_array,					idList<double>);
gltfItemClassParser( accessor_sparse,				gltfAccessor_Sparse );
gltfItemClassParser( accessor_sparse_indices,		gltfAccessor_Sparse_Indices );
gltfItemClassParser( accessor_sparse_values,		gltfAccessor_Sparse_Values );
#undef gltfItemClassParser

#pragma region helper macro to define more gltf data types that only rely on token
#define gltfItemClass(className,type,function)								\
class gltfItem_##className : public parsable, public parseType<type>		\
{public:																	\
	gltfItem_##className( idStr Name ) : name( Name ){ item = nullptr; }	\
	virtual void parse( idToken &token ) { function }						\
	virtual idStr &Name( ) { return name; }									\
private:																	\
	idStr name;}
#pragma endregion 

gltfItemClass(integer, int, *item = token.GetIntValue( ); );
gltfItemClass(number, float, *item = token.GetFloatValue( ); );
gltfItemClass(boolean, bool, token.Icmp("true") == 0 ? *item=true : token.Icmp("false") == 0 ? *item=false : common->FatalError("parse error"); );
#undef gltfItemClass

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
#pragma endregion 

#pragma region GLTF Object parsing
class gltfPropertyArray;
class gltfPropertyItem 
{
public:
	gltfPropertyItem( ) : array(nullptr){ }
	gltfPropertyArray * array;
	idToken item; 
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
private:
	bool iterating;
	bool dirty;
	int index;
	idLexer * parser;
	idList<gltfPropertyItem*> properties;
	gltfPropertyItem * endPtr;
	bool isArrayOfStructs;
};
#pragma endregion

class GLTF_Parser 
{
public:
	GLTF_Parser();
	void Init();
	bool Parse();
	bool Load(idStr filename );
	bool loadGLB(idStr filename );

	//current/last loaded gltf asset and index offsets
	gltfData *currentAsset;
private:
	void CreateBgfxData( );

	void Parse_ASSET( idToken &token );
	void Parse_SCENE( idToken &token );
	void Parse_SCENES( idToken &token );
	void Parse_NODES( idToken &token );
	void Parse_MATERIALS( idToken &token );
	void Parse_MESHES( idToken &token );
	void Parse_TEXTURES( idToken &token );
	void Parse_IMAGES( idToken &token );
	void Parse_ACCESSORS( idToken &token );
	void Parse_BUFFERVIEWS( idToken &token );
	void Parse_SAMPLERS( idToken &token );
	void Parse_BUFFERS( idToken &token );
	void Parse_ANIMATIONS( idToken &token );
	void Parse_SKINS( idToken &token );
	void Parse_EXTENSIONS_USED( idToken &token );
	void Parse_EXTENSIONS_REQUIRED( idToken &token );

	gltfProperty ParseProp( idToken &token );
	gltfProperty ResolveProp( idToken &token );
	
	idLexer	parser;
	idToken	token;
	idStr currentFile;

	bool buffersDone;
};