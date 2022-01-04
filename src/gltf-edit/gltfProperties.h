#pragma once
#include "idFramework/Common.h"
#include "idFramework/idlib/containers/StrList.h"
#include <functional>
#include "bgfx-stubs/bgfxRender.h"

enum gltfProperty {
	INVALID, 
	ASSET,
	ACCESSOR,
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
class gltfData;

// todo:
//materials, meshes , nodes

class gltfMesh_Primitive_Attribute {
public:
	gltfMesh_Primitive_Attribute( ) : accessorIndex(-1){ }
	idStr attributeSemantic;
	int accessorIndex;
};

class gltfMesh_Primitive {
public:
	gltfMesh_Primitive( ): indices(-1), material(-1), mode(-1){ }
	idList<gltfMesh_Primitive_Attribute*> attributes;
	int	indices;
	int  material;
	int mode;
	idStr target;
	idStr extensions;
	idStr extras;
};

class gltfMesh {
public:
	gltfMesh( ) { };

	idList<gltfMesh_Primitive*> primitives;	// gltfMesh_Primitive[1,*]
	idList<double> weights;					// number[1,*]
	idStr name;
	idStr extensions;
	idStr extras;
};

class gltfCamera_Orthographic {
public:
	gltfCamera_Orthographic( ) : xmag( 0.0f ), ymag( 0.0f ), zfar( 0.0f ), znear( 0.0f ) { };
	float xmag;
	float ymag;
	float zfar;
	float znear;
	idStr extensions;
	idStr extras;
};

class gltfCamera_Perspective{
	public:
		gltfCamera_Perspective( ) : aspectRatio( 0.0f ), yfov( 0.0f ) , zfar( 0.0f ) , znear( 0.0f ) { };
		float aspectRatio;
		float yfov;
		float zfar;
		float znear;
		idStr extensions;
		idStr extras;
};

class gltfCamera{
public:
	gltfCamera( ) { };
	idStr orthographic;
	idStr perspective;
	idStr type;
	idStr name;
	idStr extensions;
	idStr extras;
};

class gltfAnimation_Channel_Target {
public:
	gltfAnimation_Channel_Target( ) : node( -1 ) { };
	int node;
	idStr path;
	idStr extensions;
	idStr extras;
};

class gltfAnimation_Channel {
public:
	gltfAnimation_Channel( ) : sampler( -1 ) { };
	int sampler;
	idStr target;
	idStr extensions;
	idStr extras;
};

class gltfAnimation_Sampler {
public:
	gltfAnimation_Sampler( ) : input( -1 ), output( -1 ) { };
	int input;
	idStr interpolation;
	int output;
	idStr extensions;
	idStr extras;
};

class gltfAnimation {
public:
	gltfAnimation( ) { };
	idStr channels;
	idStr samplers;
	idStr name;
	idStr extensions;
	idStr extras;
};

class gltfAccessor_Sparse_Values {
public:
	gltfAccessor_Sparse_Values( ) : bufferView( -1 ), byteOffset( -1 ) { };
	int bufferView;
	int byteOffset;
	idStr extensions;
	idStr extras;
};

class gltfAccessor_Sparse_Indices {
public:
	gltfAccessor_Sparse_Indices( ) : bufferView( -1 ), byteOffset( -1 ), componentType( -1 ) { };
	int bufferView;
	int byteOffset;
	int componentType;
	idStr extensions;
	idStr extras;
};

class gltfAccessor_Sparse {
public:
	gltfAccessor_Sparse( ) : count( -1 ) { };
	int count;
	gltfAccessor_Sparse_Indices indices;
	gltfAccessor_Sparse_Values values;
	idStr extensions;
	idStr extras;
};

class gltfAccessor {
public:
	gltfAccessor ( ) : bufferView( -1 ), byteOffset( 0 ), componentType( -1 ), normalized( false ), count( -1 ) { }
	int bufferView;
	int byteOffset;
	int componentType;
	bool normalized;
	int count;
	idStr type;
	idList<double> max;
	idList<double> min;
	gltfAccessor_Sparse sparse;
	idStr name;
	idStr extensions;
	idStr extras;
};

class gltfBufferView {
public: 
	gltfBufferView( ) : buffer(-1),byteLength(-1),byteStride(0),byteOffset(0),target(-1){};
	int buffer;
	int byteLength;
	int byteStride;
	int byteOffset;
	int target;
	idStr name;
	idStr extensions;
	idStr extras;
	//
	gltfData *parent;
};

class gltfBuffer {
public:
	gltfBuffer( ) : byteLength(-1) , parent(nullptr){ };
	idStr uri;
	int byteLength;
	idStr name;
	idStr extensions;
	idStr extras;
	//
	gltfData * parent;
};

class gltfSampler {
public:
	gltfSampler( ) : magFilter(0),minFilter(0),wrapS(-10497),wrapT(10497){ };
	int	magFilter;
	int	minFilter;
	int	wrapS;
	int	wrapT;
	idStr name;
	idStr extensions;
	idStr extras;
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
	//
	bgfxTextureHandle bgfxTexture;
};

class gltfSkin 	{
public:
	gltfSkin( ) { };
	int		inverseBindMatrices;
	int		skeleton;
	idStr	joints; // integer[1,*]
	idStr	name;
	idStr	extensions;
	idStr	extras;
};

class gltfTexture {
public:
	gltfTexture( ) : sampler(-1), source(-1){ }
	int		sampler;
	int		source;
	idStr	name;
	idStr	extensions;
	idStr	extras;
};

class gltfAsset {
public:
	gltfAsset( ) { }
	idStr	copyright;
	idStr	generator;
	idStr	version;
	idStr	minVersion;
	idStr	extensions;
	idStr	extras;
};

class gltfExtensionsUsed{
public:
	gltfExtensionsUsed( ) { }
	idStr	extension;
};

/////////////////////////////////////////////////////////////////////////////
//// For these to function you need to add an private idList<gltf{name}*> {target}
#define GLTFCACHEITEM(name,target) \
gltf##name * name ( ) { target.AssureSizeAlloc( target.Num()+1,idListNewElement<gltf##name>); return target[target.Num()-1];} \
const idList<gltf##name*> & ##name##List() { return target; }


// URI's are resolbed during parsing so that
// all data should be layed out like an GLB with multple bin chunks
// EACH URI will habe an unique chunk
// JSON chunk MUST be the first one to be allocated/added

class gltfData {
public:
	gltfData( ) : json( nullptr ), data( nullptr ), totalChunks( -1 ) { };
	~gltfData( );
	byte *AddData( int size, int *bufferID = nullptr );
	byte *GetJsonData( ) { return json; }
	byte *GetData( int index ) { return data[index]; }
	void FileName( const idStr &file ) { fileName = file; fileNameHash = idStr::Hash( file.c_str( ) ); }
	int FileNameHash( ) { return fileNameHash; }
	idStr &FileName( ) { return fileName; }

	static idList<gltfData *> dataList;
	static gltfData *Data( ) { dataList.AssureSizeAlloc( dataList.Num( ) + 1, idListNewElement<gltfData> ); return dataList[dataList.Num( ) - 1]; }
	static const idList<gltfData *> &DataList( ) { return dataList; }
	static void ClearData( ) { common->Warning("TODO! DATA NOT FREED");}

	GLTFCACHEITEM( Buffer, buffers ) 
	GLTFCACHEITEM( Sampler, samplers )
	GLTFCACHEITEM( BufferView, bufferViews )
	GLTFCACHEITEM( Image, images )
	GLTFCACHEITEM( Texture, textures )
	GLTFCACHEITEM( Accessor, accessors )
	GLTFCACHEITEM( ExtensionsUsed, extensionsUsed )
	GLTFCACHEITEM( Mesh, meshes )

private:
	idStr fileName;
	int	fileNameHash;

	byte *json;
	byte **data;
	int totalChunks;

	idList<gltfBuffer *>					buffers;
	idList<gltfImage *>						images;
	idList<gltfData *>						assetData;
	idList<gltfSampler *>					samplers;
	idList<gltfBufferView *>				bufferViews;
	idList<gltfTexture *>					textures;
	idList<gltfAccessor *>					accessors;
	idList<gltfExtensionsUsed *>			extensionsUsed;
	idList<gltfMesh *>						meshes;

};
#undef GLTFCACHEITEM