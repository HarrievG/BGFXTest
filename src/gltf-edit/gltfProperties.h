#pragma once
#include "idFramework/Common.h"
#include "idFramework/idlib/containers/StrList.h"
#include <functional>
#include "bgfx-stubs/bgfxRender.h"
#include "idFramework/idlib/math/Quat.h"
#include "idlib/Lib.h"
#include "gltfCamera.h"

enum gltfProperty {
	INVALID,
	ASSET,
	ACCESSOR,
	CAMERAS,
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
	EXTENSIONS,
	EXTENSIONS_USED,
	EXTENSIONS_REQUIRED
};


class gltfData;
struct gltf_sampler_mag_type_map {
	int id;
	uint bgfxFlagMag;
};

struct gltf_sampler_wrap_type_map {
	   int id;
	   uint bgfxFlagU;
	   uint bgfxFlagV;
};

struct gltf_mesh_attribute_map {
	idStr stringID;
	bgfx::Attrib::Enum attib;
	uint elementSize;
};

struct gltf_accessor_component
{
	enum Type { 
		_byte,
		_uByte,
		_short,
		_uShort,
		_uInt,
		_float,
		_double,
		Count
	};
};

template< class T >
struct gltf_accessor_component_type_map {
	idStr stringID;
	int id;
	T type;
	uint sizeInBytes;//single element
};

class gltfExtra
{
public:
	gltfExtra( ) { }
	idStr json;
};

class gltfExt_KHR_lights_punctual;
class gltfExtensions {
public:
	gltfExtensions( ) { }
	idList<gltfExt_KHR_lights_punctual *>	KHR_lights_punctual;
};

class gltfNode_KHR_lights_punctual {
public:
	int light; 
};

class gltfNode_Extensions {
public:
	gltfNode_Extensions( ) : 
		KHR_lights_punctual( nullptr) { }
	gltfNode_KHR_lights_punctual* KHR_lights_punctual;
};

class gltfExt_KHR_materials_pbrSpecularGlossiness;
class gltfMaterial_Extensions {
public:
	gltfMaterial_Extensions( ) :
		KHR_materials_pbrSpecularGlossiness( nullptr ) { }
	gltfExt_KHR_materials_pbrSpecularGlossiness *KHR_materials_pbrSpecularGlossiness;
};

class gltfNode {
public:
	gltfNode( ) : camera( -1 ), skin( -1 ), matrix( mat4_zero ),
		mesh( -1 ), rotation( 0.f, 0.f, 0.f, 1.f ), scale( 1.f, 1.f, 1.f ),
		translation( vec3_zero ), parent( nullptr ), dirty( true ) { }
	int						camera;
	idList<int>				children;
	int						skin;
	idMat4					matrix;
	int						mesh;
	idQuat					rotation;
	idVec3					scale;
	idVec3					translation;
	idList<double>			weights;
	idStr					name;
	gltfNode_Extensions		extensions;
	idStr					extras;

	//
	gltfNode *parent;
	bool			dirty;
};

struct gltfCameraNodePtrs {
	gltfNode *translationNode = nullptr;
	gltfNode *orientationNode = nullptr;
};

class gltfScene {
public:
	gltfScene( ) { }
	idList<int> nodes;
	idStr name;
	idStr extensions;
	idStr extras;
};

class gltfMesh_Primitive_Attribute {
public:
	gltfMesh_Primitive_Attribute( ) : accessorIndex( -1 ), bgfxType( bgfx::Attrib::Count ), elementSize( 0 ) { }
	idStr attributeSemantic;
	int accessorIndex;
	bgfx::Attrib::Enum bgfxType;
	uint elementSize;
};

class gltfMesh_Primitive {
public:
	gltfMesh_Primitive( ) : indices( -1 ), material( -1 ), mode( -1 ) { }
	idList<gltfMesh_Primitive_Attribute *> attributes;
	int	indices;
	int  material;
	int mode;
	idStr target;
	idStr extensions;
	idStr extras;
	//
	bgfx::VertexBufferHandle vertexBufferHandle;
	bgfx::IndexBufferHandle indexBufferHandle;
};

class gltfMesh {
public:
	gltfMesh( ) { };

	idList<gltfMesh_Primitive *> primitives;	// gltfMesh_Primitive[1,*]
	idList<double> weights;						// number[1,*]
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

class gltfCamera_Perspective {
public:
	gltfCamera_Perspective( ) : aspectRatio( 0.0f ), yfov( 0.0f ), zfar( 0.0f ), znear( 0.0f ) { };
	float aspectRatio;
	float yfov;
	float zfar;
	float znear;
	idStr extensions;
	idStr extras;
};

class gltfCamera {
public:
	gltfCamera( ) { };
	gltfCamera_Orthographic orthographic;
	gltfCamera_Perspective perspective;
	idStr type;
	idStr name;
	idStr extensions;
	idStr extras;
};

class gltfAnimation_Channel_Target {
public:
	gltfAnimation_Channel_Target( ) : node( -1 ), TRS( gltfTRS::count ) { };
	int node;
	idStr path;
	idStr extensions;
	idStr extras;

	enum gltfTRS {
		none,
		rotation,
		translation,
		scale,
		weights,
		count
	};

	gltfTRS TRS;

	static gltfTRS resolveType( idStr type ) {
		if ( type == "translation" )
			return gltfTRS::translation;
		else if ( type == "rotation" )
			return  gltfTRS::rotation;
		else if ( type == "scale" )
			return  gltfTRS::scale;
		else if ( type == "weights" )
			return  gltfTRS::weights;
		return gltfTRS::count;
	}
};

class gltfAnimation_Channel {
public:
	gltfAnimation_Channel( ) : sampler( -1 ) { };
	int sampler;
	gltfAnimation_Channel_Target target;
	idStr extensions;
	idStr extras;
};

class gltfAnimation_Sampler {
public:
	gltfAnimation_Sampler( ) : input( -1 ), interpolation("LINEAR"),output( -1 ), intType(gltfInterpType::count) { };
	int input;
	idStr interpolation;
	int output;
	idStr extensions;
	idStr extras;

	enum gltfInterpType {
		linear,
		step,
		cubicSpline,
		count
	};

	gltfInterpType intType;

	static gltfInterpType resolveType( idStr type ) {
		if ( type == "LINEAR" )
			return gltfInterpType::linear;
		else if ( type == "STEP" )
			return gltfInterpType::step;
		else if ( type == "CUBICSPLINE" )
			return gltfInterpType::cubicSpline;
		return gltfInterpType::count;
	}

};

class gltfAnimation {
public:
	gltfAnimation( ) : maxTime (0.0f),numFrames(0) { };
	idList<gltfAnimation_Channel*> channels;
	idList<gltfAnimation_Sampler*> samplers;
	idStr name;
	idStr extensions;
	idStr extras;

	float maxTime;

	//id specific
	mutable int	ref_count;
	int numFrames;
	void DecreaseRefs() const {ref_count--;};
	void IncreaseRefs() const {ref_count++;};
	bool GetBounds( idBounds &bnds, int time, int cyclecount ) const { return false;}
	bool GetOriginRotation( idQuat &rotation, int time, int cyclecount ) const { return false;}
	bool GetOrigin( idVec3 &offset, int time, int cyclecount ) const { return false;}
	const idVec3 &TotalMovementDelta( void ) const {static idVec3 temp; return temp; }
	int NumFrames() const {return numFrames;}
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
	gltfAccessor( ) : bufferView( -1 ), byteOffset( 0 ), componentType( -1 ), normalized( false ), count( -1 ) ,
		floatView(nullptr),vecView(nullptr),quatView(nullptr),matView(nullptr){ }
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

	bgfx::AttribType::Enum bgfxType;
	uint typeSize;

	idList<float>   * floatView;
	idList<idVec3*> * vecView;
	idList<idQuat*> * quatView;
	idList<idMat4> * matView;
};

class gltfBufferView {
public:
	gltfBufferView( ) : buffer( -1 ), byteLength( -1 ), byteStride( 0 ), byteOffset( 0 ), target( -1 ) { };
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
	gltfBuffer( ) : byteLength( -1 ), parent( nullptr ) { };
	idStr uri;
	int byteLength;
	idStr name;
	idStr extensions;
	idStr extras;
	//
	gltfData *parent;
};

class gltfSampler {
public:
	gltfSampler( ) : magFilter( 0 ), minFilter( 0 ), wrapS( 10497 ), wrapT( 10497 ) { };
	int	magFilter;
	int	minFilter;
	int	wrapS;
	int	wrapT;
	idStr name;
	idStr extensions;
	idStr extras;
	//
	uint bgfxSamplerFlags;
};

class gltfImage {
public:
	gltfImage( ) : bufferView( -1 ) { }
	idStr	uri;
	idStr	mimeType;
	int		bufferView;
	idStr	name;
	idStr	extensions;
	idStr	extras;
	//
	bgfxTextureHandle bgfxTexture;
};

class gltfSkin {
public:
	gltfSkin( ) : inverseBindMatrices(-1),skeleton(-1),name("unnamedSkin"){ };
	int			inverseBindMatrices;
	int			skeleton;
	idList<int>	joints; // integer[1,*]
	idStr		name;
	idStr		extensions;
	idStr		extras;
};

class gltfOcclusionTexture_Info {
public:
	gltfOcclusionTexture_Info( ) : index( -1 ), texCoord( 0 ), strength( 1.0f ) { }
	int		index;
	int		texCoord;
	float	strength;
	idStr	extensions;
	idStr	extras;
};

class gltfNormalTexture_Info {
public:
	gltfNormalTexture_Info( ) : index( -1 ), texCoord( 0 ), scale( 1.0f ) { }
	int		index;
	int		texCoord;
	float	scale;
	idStr	extensions;
	idStr	extras;
};

class gltfTexture_Info {
public:
	gltfTexture_Info( ) : index( -1 ), texCoord( 0 ) { }
	int		index;
	int		texCoord;
	idStr	extensions;
	idStr	extras;
};

class gltfTexture {
public:
	gltfTexture( ) : sampler( -1 ), source( -1 ) { }
	int		sampler;
	int		source;
	idStr	name;
	idStr	extensions;
	idStr	extras;
};

class gltfMaterial_pbrMetallicRoughness {
public:
	gltfMaterial_pbrMetallicRoughness( ) : baseColorFactor( vec4_one ), metallicFactor( 1.0f ), roughnessFactor( 1.0f ) { }
	idVec4				baseColorFactor;
	gltfTexture_Info	baseColorTexture;
	float				metallicFactor;
	float				roughnessFactor;
	gltfTexture_Info	metallicRoughnessTexture;
	idStr				extensions;
	idStr				extras;
};

class gltfMaterial {
public:
	gltfMaterial( ) : emissiveFactor( vec3_zero ), alphaMode( "OPAQUE" ), alphaCutoff( 0.5f ), doubleSided( false ) { }
	gltfMaterial_pbrMetallicRoughness	pbrMetallicRoughness;
	gltfNormalTexture_Info				normalTexture;
	gltfOcclusionTexture_Info			occlusionTexture;
	gltfTexture_Info					emissiveTexture;
	idVec3								emissiveFactor;
	idStr								alphaMode;
	float								alphaCutoff;
	bool								doubleSided;
	idStr								name;
	gltfMaterial_Extensions				extensions;
	gltfExtra							extras;
	//
	bgfxMaterial						bgfxMaterial;
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

//this is not used.
//if an extension is found, it _will_ be used. (if implemented)
class gltfExtensionsUsed {
public:
	gltfExtensionsUsed( ) { }
	idStr	extension;
};

//ARCHIVED?
//https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Archived/KHR_materials_pbrSpecularGlossiness
class gltfExt_KHR_materials_pbrSpecularGlossiness
{
public:
	gltfExt_KHR_materials_pbrSpecularGlossiness( ) { }
	idVec4				diffuseFactor;
	gltfTexture_Info	diffuseTexture;
	idVec3				specularFactor;
	float				glossinessFactor;
	gltfTexture_Info	specularGlossinessTexture;
	idStr				extensions;
	idStr				extras;
};

//KHR_lights_punctual_spot
//https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_lights_punctual/schema/light.spot.schema.json
class gltfExt_KHR_lights_punctual_spot {
public:
	gltfExt_KHR_lights_punctual_spot( ) : innerConeAngle(0.0f), outerConeAngle( idMath::ONEFOURTH_PI ){ }
	float	innerConeAngle;
	float	outerConeAngle;
	idStr	extensions;
	idStr	extras;
};
typedef gltfExt_KHR_lights_punctual_spot spot;

//KHR_lights_punctual
//https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_lights_punctual/schema/light.schema.json
class gltfExt_KHR_lights_punctual {
public:
	gltfExt_KHR_lights_punctual( ) : color(vec3_one),intensity(1.0f),range(-1.0f),intType(-1) { }
	idVec3	color;
	float	intensity;
	spot	spot;
	idStr	type; //directional=0,point=1,spot=2
	float	range;
	idStr	name;
	idStr	extensions;
	idStr	extras;

	int intType;

	static int resolveType( idStr type ) {
		if (type == "directional" )
			return 0;
		else if (type == "point" )
			return 1;
		else if (type == "spot" )
			return 2;
		return -1;
	}
};

/////////////////////////////////////////////////////////////////////////////
//// For these to function you need to add an private idList<gltf{name}*> {target}
#define GLTFCACHEITEM(name,target) \
gltf##name * name ( ) { target.AssureSizeAlloc( target.Num()+1,idListNewElement<gltf##name>); return target[target.Num()-1];} \
const inline idList<gltf##name*> & ##name##List() { return target; }


// URI's are resolved during parsing so that
// all data should be layed out like an GLB with multiple bin chunks
// EACH URI will have an unique chunk
// JSON chunk MUST be the first one to be allocated/added

class gltfData {
public:
	gltfData( ) : fileNameHash( 0 ), json( nullptr ), data( nullptr ), totalChunks( -1 ) {  cameraManager = new gltfCameraManager( this ); };
	~gltfData( );
	byte *AddData( int size, int *bufferID = nullptr );
	byte *GetJsonData( int &size ) { size = jsonDataLength; return json; }
	byte *GetData( int index ) { return data[index]; }
	void FileName( const idStr &file ) { fileName = file; fileNameHash = fileDataHash.GenerateKey( file.c_str( ) ); }
	int FileNameHash( ) { return fileNameHash; }
	idStr &FileName( ) { return fileName; }

	static idHashIndex			fileDataHash;
	static idList<gltfData *>	dataList;
	//add data from filename
	static gltfData *Data( idStr &fileName ) {
		dataList.AssureSizeAlloc( dataList.Num( ) + 1, idListNewElement<gltfData> );
		dataList[dataList.Num( ) - 1]->FileName( fileName );
		fileDataHash.Add( fileDataHash.GenerateKey( fileName ), dataList.Num( ) - 1 );
		return dataList[dataList.Num( ) - 1];
	}
	//find data;
	static gltfData *Data( const char *filename ) { return dataList[fileDataHash.First( fileDataHash.GenerateKey( filename ) )]; }
	static const idList<gltfData *> &DataList( ) { return dataList; }
	static void ClearData( ) { common->Warning( "TODO! DATA NOT FREED" ); }

	//return the GLTF nodes that control the given camera
	//return TRUE if the camera uses 2 nodes (like when blender exports gltfs with +Y..)
	//This is determined by checking for an "_Orientation" suffix to the camera name of the node that has the target camera assigned. 
	// if so, translate node will be set to the parent node of the orientation node.
	//Note: does not take overides into account!
	gltfNode* GetCameraNodes( gltfCamera *camera )
	{
		gltfCameraNodePtrs result;

		assert( camera );
		int camId = -1;
		for ( auto *cam : cameras )
		{
			camId++;
			if ( cam == camera )
				break;
		}

		for ( int i = 0; i < nodes.Num( ); i++ )
		{
			if ( nodes[i]->camera != -1 && nodes[i]->camera == camId ) 
				return nodes[i];
		}

		return nullptr;
	}

	idMat4 GetViewMatrix( int camId ) const
	{
		if (cameraManager->HasOverideID(camId) )
		{
			auto overrideCam = cameraManager->GetOverride( camId );
			camId = overrideCam.newCameraID;
		}

		idMat4 result = mat4_identity;

		idList<gltfNode*> hierachy(2);
		gltfNode* parent = nullptr;

		for ( int i = 0; i < nodes.Num( ); i++ )
		{
			if ( nodes[i]->camera != -1 && nodes[i]->camera == camId ) 
			{
				parent = nodes[i];
				while ( parent ) {
					hierachy.Append( parent );
					parent = parent->parent;
				}
				break;
			}
		}

		for ( int i = hierachy.Num( ) - 1; i >= 0; i-- )
		{
			ResolveNodeMatrix(hierachy[i]);
			result *= hierachy[i]->matrix;
		}

		return result;
	}
	//Please note : assumes all nodes are _not_ dirty!
	idMat4 GetLightMatrix( int lightId ) const 
	{
		idMat4 result = mat4_identity;

		idList<gltfNode *> hierachy;
		gltfNode *parent = nullptr;
		hierachy.SetGranularity( 2 );

		for ( int i = 0; i < nodes.Num( ); i++ ) {
			if ( nodes[i]->extensions.KHR_lights_punctual && nodes[i]->extensions.KHR_lights_punctual->light == lightId ) {
				parent = nodes[i];
				while ( parent ) {
					hierachy.Append( parent );
					parent = parent->parent;
				}
				break;
			}
		}

		for ( int i = hierachy.Num( ) - 1; i >= 0; i-- )
			result *= hierachy[i]->matrix;

		return result;
	}
	
	// v * T * R * S. ->row major
	// v' = S * R * T * v -> column major;
	//bgfx = column-major
	//idmath = row major, except mat3
	//gltf matrices : column-major.
	//if mat* is valid , it will be multplied by this node's matrix that is resolved in its full hiararchy and stops at root.
	static void ResolveNodeMatrix( gltfNode *node, idMat4 *mat = nullptr ,gltfNode *root = nullptr ) 
	{
		if ( node->dirty ) 
		{
			idMat4 scaleMat = idMat4(
				node->scale.x, 0, 0, 0,
				0, node->scale.y, 0, 0,
				0, 0, node->scale.z, 0,
				0, 0, 0, 1
			);
			
			node->matrix = idMat4( mat3_identity, node->translation ) * node->rotation.ToMat4( ).Transpose( ) * scaleMat;

			node->dirty = false;
		}

		//resolve full hierarchy
		if ( mat != nullptr ) {
			idList<gltfNode *> hierachy(2);
			gltfNode *parent = node;
			while ( parent ) {
				ResolveNodeMatrix(parent);
				hierachy.Append( parent );
				if ( parent == root )
					break;
				parent = parent->parent;
			}
			for ( int i = hierachy.Num( ) - 1; i >= 0; i-- )
				*mat *= hierachy[i]->matrix;
		}
	}

	void Advance( gltfAnimation *anim = nullptr );

	//this copies the data and view cached on the accessor
	template <class T>
	idList<T*> &GetAccessorView( gltfAccessor *accessor );
	idList<float> &GetAccessorView( gltfAccessor *accessor );
	idList<idMat4> &GetAccessorViewMat( gltfAccessor *accessor );

	int &DefaultScene( ) { return scene; }
	GLTFCACHEITEM( Buffer, buffers )
	GLTFCACHEITEM( Sampler, samplers )
	GLTFCACHEITEM( BufferView, bufferViews )
	GLTFCACHEITEM( Image, images )
	GLTFCACHEITEM( Texture, textures )
	GLTFCACHEITEM( Accessor, accessors )
	GLTFCACHEITEM( ExtensionsUsed, extensionsUsed )
	GLTFCACHEITEM( Mesh, meshes )
	GLTFCACHEITEM( Scene, scenes )
	GLTFCACHEITEM( Node, nodes )
	GLTFCACHEITEM( Camera, cameras )
	GLTFCACHEITEM( Material, materials )
	GLTFCACHEITEM( Extensions, extensions )
	GLTFCACHEITEM( Animation, animations )
	GLTFCACHEITEM( Skin, skins )

	gltfCameraManager * cameraManager;
private:
	idStr fileName;
	int	fileNameHash;

	byte *json;
	byte **data;
	int jsonDataLength;
	int totalChunks;

	

	idList<gltfBuffer *>			buffers;
	idList<gltfImage *>				images;
	idList<gltfData *>				assetData;
	idList<gltfSampler *>			samplers;
	idList<gltfBufferView *>		bufferViews;
	idList<gltfTexture *>			textures;
	idList<gltfAccessor *>			accessors;
	idList<gltfExtensionsUsed *>	extensionsUsed;
	idList<gltfMesh *>				meshes;
	int								scene;
	idList<gltfScene *>				scenes;
	idList<gltfNode *>				nodes;
	idList<gltfCamera *>			cameras;
	idList<gltfMaterial *>			materials;
	idList<gltfExtensions *>		extensions;
	idList<gltfAnimation *>			animations;
	idList<gltfSkin *>				skins;
};

#undef GLTFCACHEITEM