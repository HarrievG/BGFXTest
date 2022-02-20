$input a_position, a_normal, a_tangent, a_texcoord0
$output v_worldpos, v_normal, v_tangent, v_texcoord


#include "common.sh"
#include <bgfx_shader.sh>

// model transformation for normals to preserve perpendicularity
// usually this is based on the model view matrix
// but shading is done in world space
uniform mat4 u_normalMatrix;

void main()
{
    v_worldpos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    v_normal = mul(u_normalMatrix, a_normal);
    v_tangent = mul(u_model[0],a_tangent.xyz);
    v_texcoord = a_texcoord0;
	//v_color = a_color0;
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
