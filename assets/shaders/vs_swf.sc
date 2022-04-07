$input a_position,a_texcoord0,a_texcoord1, a_texcoord2, a_color0, a_color1
$output v_color

#include "common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position.xy, 0.0, 1.0) );
	v_color = a_color0;
}