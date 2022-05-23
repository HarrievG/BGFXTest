$input a_position, a_normal, a_tangent, a_texcoord0, a_weight, a_indices
$output v_worldpos, v_normal, v_tangent, v_texcoord


#include "common.sh"
#include <bgfx_shader.sh>
#include "animation.sh"


// model transformation for normals to preserve perpendicularity
// usually this is based on the model view matrix
// but shading is done in world space
uniform mat4 u_normalMatrix;


uniform vec4 u_vertexOptions;
#define u_HasBones					((uint(u_vertexOptions.x) & (1 << 0)) != 0)

vec4 getPosition(vec3 _position, int4 _indices, vec4 _weights)
{
#ifdef USE_MORPHING
   return mul(getTargetPosition(),pos);
#endif

#ifdef USE_SKINNING
	if(u_HasBones)
		return mul(getSkinningMatrix(_indices,_weights) ,vec4(_position, 1.0));
	else
		return vec4(_position,1);
#endif

	return vec4(_position,1);
}

void main()
{
	vec4 target = getPosition(a_position,a_indices,a_weight);
	vec3 pos = target.xyz;// / target.w;
    v_worldpos = mul(u_model[0], pos );
    v_normal = mul(u_normalMatrix, a_normal);
    v_tangent = mul(u_model[0],a_tangent.xyz);
    v_texcoord = a_texcoord0;
	//v_color = a_color0;
    gl_Position = mul(u_modelViewProj, vec4(pos , 1.0));
}
