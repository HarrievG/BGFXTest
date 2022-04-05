$input v_color, v_texcoord3

#include "common.sh"

SAMPLERCUBE(s_texColor, 0);

void main()
{
	vec4 color = textureCube(s_texColor, v_texcoord3.xyz);
	int index = int(v_texcoord3.w*4.0 + 0.5);
	float alpha = index < 1 ? color.z :
		index < 2 ? color.y :
		index < 3 ? color.x : color.w;
	gl_FragColor = vec4(v_color.xyz, v_color.a * alpha);
}
