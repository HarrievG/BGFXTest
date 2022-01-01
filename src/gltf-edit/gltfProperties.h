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

//No URI's are left after parsing
// all data should be layed out like an GLB.
// EACH URI will be an unique chunk
// JSON chunk MUST be the first one to be allocated/added
class gltfData
{
public:
	gltfData( ) : json (nullptr), data( nullptr ), totalChunks(-1) { };
	~gltfData();
	byte* AddData(int size, int * bufferID=nullptr);
	byte* GetJsonData() { return json; }
	byte* GetData(int index ) { return data[index]; }	
	void FileName( const idStr & file ) { fileName = file; fileNameHash = idStr::Hash( file );}
	int FileNameHash() {return fileNameHash; }
	idStr & FileName() { return fileName; }
private:
	//buffer chunks
	byte *json;
	byte ** data;
	int totalChunks;

	idStr fileName;
	int	fileNameHash;
};

// todo:
//materials, meshes , nodes

class gltfMesh {
public:
	gltfMesh( ) { };
	idStr primitives;
	idStr weights;  // number[1,*]
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
	idStr indices;
	idStr values;
	idStr extensions;
	idStr extras;
};

class gltfAccessor {
public:
	gltfAccessor ( ) : bufferView( -1 ), byteOffset( -1 ), componentType( -1 ), normalized( false ), count( -1 ), max( -1 ), min( -1 ){ }
	int bufferView;
	int byteOffset;
	int componentType;
	bool normalized;
	int count;
	idStr type;
	int max;
	int min;
	idStr sparse;
	idStr name;
	idStr extensions;
	idStr extras;
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