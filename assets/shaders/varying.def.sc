 vec3 v_position  : TEXCOORD1; // = vec3(0.0, 0.0, 0.0);
 vec2 v_texcoord  : TEXCOORD0; // = vec2(0.0, 0.0);
 vec3 v_normal    : NORMAL   ; // = vec3(0.0, 0.0, 0.0);
 vec4 v_tangent   : TANGENT  ; // = vec4(0.0,0.0,0.0,1.0);
 vec3 v_bitangent : BITANGENT; // = vec3(0.0, 0.0, 0.0);
 vec4 v_color     : COLOR0   ; // = vec4(0.0,0.0,0.0,1.0);
 vec3 v_worldpos  : POSITION1; // = vec3(0.0, 0.0, 0.0);

 vec3 a_position  : POSITION;
 vec2 a_texcoord0 : TEXCOORD0;
 vec4 a_texcoord1 : TEXCOORD1;
 vec4 a_texcoord2 : TEXCOORD2;
 vec3 a_normal    : NORMAL;
 vec4 a_tangent   : TANGENT;
 vec3 a_bitangent : BITANGENT;
 vec4 a_color0    : COLOR0;
 vec4 a_color1    : COLOR1;
 vec4 a_weight    : BLENDWEIGHT;
ivec4 a_indices   : BLENDINDICES;

vec4 a_texcoord3  : TEXCOORD3 = vec4(0.0, 0.0,0.0,0.0);
vec4 v_texcoord3  : TEXCOORD3;
