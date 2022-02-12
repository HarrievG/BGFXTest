$input v_position, v_texcoord, v_normal, v_tangent, v_bitangent , v_color

#include <bgfx_shader.sh>

void main() {
	gl_FragColor = v_color;
}
