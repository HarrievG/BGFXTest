$input a_position, a_color0, a_texcoord3
$output v_color, v_texcoord3

#include "common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position.xy, 0.0, 1.0) );
	v_texcoord3 = a_texcoord3;
	v_color = a_color0;
}
