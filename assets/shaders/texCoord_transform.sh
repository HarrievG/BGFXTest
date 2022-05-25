// KHR_texture_transform extension
// see https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_lights_punctual

uniform vec4 u_texTransformMask;


#define u_hasBaseColorTextureTransMask			((uint(u_texTransformMask.x) & (1 << SAMPLER_PBR_BASECOLOR)) != 0)
#define u_hasMetallicRoughnessTextureTrans		((uint(u_texTransformMask.x) & (1 << SAMPLER_PBR_METALROUGHNESS)) != 0)
//#define u_hasNormalTextureTrans				((uint(half(u_texTransformMask.x)) & (1 << SAMPLER_PBR_NORMAL)) != 0)
//#define u_hasOcclusionTextureTrans			((uint(half(u_texTransformMask.x)) & (1 << SAMPLER_PBR_OCCLUSION)) != 0)
//#define u_hasEmissiveTextureTrans				((uint(half(u_texTransformMask.x)) & (1 << SAMPLER_PBR_EMISSIVE)) != 0)

#define u_BaseColorTextureTransIdx				u_texTransformMask.y
//#define u_MetallicRoughnessTextureTransIdx	uint(half2(u_texTransformMask.y).y)
//#define u_NormalTextureTransIdx				uint(half2(u_texTransformMask.z).x)
//#define u_OcclusionTextureTransIdx			uint(half2(u_texTransformMask.z).y)
//#define u_EmissiveTextureTransIdx			uint(half2(u_texTransformMask.w).x)

struct TextureTransform
{
    vec2	offset;
    vec2	scale;
	float	rotation;
	uint	texCoord;
	uint	mask;
};

BUFFER_RO(b_TextureTransforms, vec2, SAMPLER_TEXTURE_TRANSFORMS);



vec2 getTexCoord(uint texSlot,vec2 _texcoord)
{
	
	return _texcoord;
}

TextureTransform getTransform(uint i)
{
	int index = 3 * i;
    TextureTransform transForm;

	transForm.offset	= b_TextureTransforms[index + 0];	

    transForm.scale		= b_TextureTransforms[index + 1];

	transForm.rotation	= b_TextureTransforms[index + 2].x;
	half2 fp16			= b_TextureTransforms[index + 2].y;  
	transForm.texCoord	= uint(fp16.x);
	transForm.mask		= uint(fp16.y);

    return transForm;
}

vec2 getTexCoordT(TextureTransform transform , vec2 _texcoord)
{
	vec2 _offset = transform.offset;
	vec2 _scale = transform.scale;
	float _rot = transform.rotation;
	
	
	mat3 translation = mat3(1,0,0,
							0,1,0,
							_offset.x,_offset.y,1);
	mat3 rotation = mat3(
		cos(_rot), sin(_rot), 0,
	   -sin(_rot), cos(_rot), 0,
			   0,		 0, 1
	);

	mat3 scale = mat3(0,0,0,
					_scale.x,_scale.y,0,
					0,0,1);

	mat3 target = mul(mul(translation,rotation),scale);
	return ( mul(target,vec3(_texcoord, 1)) ).xy;	
}


 vec2 pbrBaseColorTexCoord(vec2 texcoord)
 {
	if (u_hasBaseColorTextureTransMask)
	{
		half2 fp16	= u_BaseColorTextureTransIdx;  
		TextureTransform transform = getTransform(fp16.x);
		return  getTexCoordT(transform, texcoord);
	}		
	else
		return texcoord;

 }
