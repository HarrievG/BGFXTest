$input v_color
#include "common.sh"


void main()
{
	vec4 unpackedColor = v_color * ( 1.0f / 255.0f );
	gl_FragColor = v_color;
}
