
$input a_position, a_normal, a_texcoord0, a_tangent, a_bitangent , a_color0
$output v_position, v_texcoord, v_normal, v_tangent, v_bitangent , v_color


#include <common.sh>

void main() {
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_color = a_color0;
}
