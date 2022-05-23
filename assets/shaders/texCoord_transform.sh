// KHR_texture_transform extension
// see https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_lights_punctual

uniform vec4 u_texTransformMask;

#define u_test				((uint(u_texTransformMask.x) & (1 << 0)) != 0)
struct TextureTransform
{
    vec2	offset;
    vec2	scale;
	float	rotation;
	uint	texCooord;
	uint	mask;
};

BUFFER_RO(b_TextureTransforms, vec2, SAMPLER_TEXTURE_TRANSFORMS);


TextureTransform getTransform(uint i)
{
	int index = 3 * i;
    TextureTransform transForm;

	transForm.offset	= b_TextureTransforms[index + 0];	

    transForm.scale		= b_TextureTransforms[index + 1];

	transForm.rotation	= b_TextureTransforms[index + 2].x;
	half2 fp16			= b_TextureTransforms[index + 2].y;  
	transForm.texCooord	= int(fp16.x);
	transForm.mask		= uint(fp16.y);

    return transForm;
}

vec2 getTexCoord(TextureTransform transform , vec2 _texcoord)
{
	if (!u_test)
		return _texcoord;
	//else
	//{
		//vec2 _offset = u_textureTransform_offset;
		//vec2 _scale = u_textureTransform_scale;
		//float _rot = u_textureTransform_rotation;
		//
		//mat3 translation = mat3(1,0,0, 0,1,0, _offset.x, _offset.y, 1);
		//mat3 rotation = mat3(
		//	cos(_rot), sin(_rot), 0,
		//   -sin(_rot), cos(_rot), 0,
		//		   0,		 0, 1
		//);
		//mat3 scale = mat3(_scale.x,0,0, 0,_scale.y,0, 0,0,1);
		//
		//mat3 target = mul(mul(translation,rotation),scale);
		//return ( mul(target,vec3(_texcoord, 1)) ).xy;		
	//}
	return vec2(1,1);
}