// shaderc command line:
// bin\shadercRelease.exe -f shaders\fs_forward.sc -o shaders\fs_forward.bin --platform windows --type fragment --verbose -i ./ -p ps_5_0 --debug -O 0 --define USE_SKINNING


float intBitsToFloat(int _x) { return asfloat(_x); }
float2 intBitsToFloat(uint2 _x) { return asfloat(_x); }
float3 intBitsToFloat(uint3 _x) { return asfloat(_x); }
float4 intBitsToFloat(uint4 _x) { return asfloat(_x); }
float uintBitsToFloat(uint _x) { return asfloat(_x); }
float2 uintBitsToFloat(uint2 _x) { return asfloat(_x); }
float3 uintBitsToFloat(uint3 _x) { return asfloat(_x); }
float4 uintBitsToFloat(uint4 _x) { return asfloat(_x); }
uint floatBitsToUint(float _x) { return asuint(_x); }
uint2 floatBitsToUint(float2 _x) { return asuint(_x); }
uint3 floatBitsToUint(float3 _x) { return asuint(_x); }
uint4 floatBitsToUint(float4 _x) { return asuint(_x); }
int floatBitsToInt(float _x) { return asint(_x); }
int2 floatBitsToInt(float2 _x) { return asint(_x); }
int3 floatBitsToInt(float3 _x) { return asint(_x); }
int4 floatBitsToInt(float4 _x) { return asint(_x); }
uint bitfieldReverse(uint _x) { return reversebits(_x); }
uint2 bitfieldReverse(uint2 _x) { return reversebits(_x); }
uint3 bitfieldReverse(uint3 _x) { return reversebits(_x); }
uint4 bitfieldReverse(uint4 _x) { return reversebits(_x); }
uint packHalf2x16(float2 _x)
{
return (f32tof16(_x.y)<<16) | f32tof16(_x.x);
}
float2 unpackHalf2x16(uint _x)
{
return float2(f16tof32(_x & 0xffff), f16tof32(_x >> 16) );
}
struct BgfxSampler2D
{
SamplerState m_sampler;
Texture2D m_texture;
};
struct BgfxISampler2D
{
Texture2D<int4> m_texture;
};
struct BgfxUSampler2D
{
Texture2D<uint4> m_texture;
};
struct BgfxSampler2DArray
{
SamplerState m_sampler;
Texture2DArray m_texture;
};
struct BgfxSampler2DShadow
{
SamplerComparisonState m_sampler;
Texture2D m_texture;
};
struct BgfxSampler2DArrayShadow
{
SamplerComparisonState m_sampler;
Texture2DArray m_texture;
};
struct BgfxSampler3D
{
SamplerState m_sampler;
Texture3D m_texture;
};
struct BgfxISampler3D
{
Texture3D<int4> m_texture;
};
struct BgfxUSampler3D
{
Texture3D<uint4> m_texture;
};
struct BgfxSamplerCube
{
SamplerState m_sampler;
TextureCube m_texture;
};
struct BgfxSamplerCubeShadow
{
SamplerComparisonState m_sampler;
TextureCube m_texture;
};
struct BgfxSampler2DMS
{
Texture2DMS<float4> m_texture;
};
float4 bgfxTexture2D(BgfxSampler2D _sampler, float2 _coord)
{
return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
}
float4 bgfxTexture2DBias(BgfxSampler2D _sampler, float2 _coord, float _bias)
{
return _sampler.m_texture.SampleBias(_sampler.m_sampler, _coord, _bias);
}
float4 bgfxTexture2DLod(BgfxSampler2D _sampler, float2 _coord, float _level)
{
return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
}
float4 bgfxTexture2DLodOffset(BgfxSampler2D _sampler, float2 _coord, float _level, int2 _offset)
{
return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level, _offset);
}
float4 bgfxTexture2DProj(BgfxSampler2D _sampler, float3 _coord)
{
float2 coord = _coord.xy * rcp(_coord.z);
return _sampler.m_texture.Sample(_sampler.m_sampler, coord);
}
float4 bgfxTexture2DProj(BgfxSampler2D _sampler, float4 _coord)
{
float2 coord = _coord.xy * rcp(_coord.w);
return _sampler.m_texture.Sample(_sampler.m_sampler, coord);
}
float4 bgfxTexture2DGrad(BgfxSampler2D _sampler, float2 _coord, float2 _dPdx, float2 _dPdy)
{
return _sampler.m_texture.SampleGrad(_sampler.m_sampler, _coord, _dPdx, _dPdy);
}
float4 bgfxTexture2DArray(BgfxSampler2DArray _sampler, float3 _coord)
{
return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
}
float4 bgfxTexture2DArrayLod(BgfxSampler2DArray _sampler, float3 _coord, float _lod)
{
return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _lod);
}
float4 bgfxTexture2DArrayLodOffset(BgfxSampler2DArray _sampler, float3 _coord, float _level, int2 _offset)
{
return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level, _offset);
}
float bgfxShadow2D(BgfxSampler2DShadow _sampler, float3 _coord)
{
return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, _coord.xy, _coord.z);
}
float bgfxShadow2DProj(BgfxSampler2DShadow _sampler, float4 _coord)
{
float3 coord = _coord.xyz * rcp(_coord.w);
return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, coord.xy, coord.z);
}
float4 bgfxShadow2DArray(BgfxSampler2DArrayShadow _sampler, float4 _coord)
{
return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, _coord.xyz, _coord.w);
}
float4 bgfxTexture3D(BgfxSampler3D _sampler, float3 _coord)
{
return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
}
float4 bgfxTexture3DLod(BgfxSampler3D _sampler, float3 _coord, float _level)
{
return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
}
int4 bgfxTexture3D(BgfxISampler3D _sampler, float3 _coord)
{
uint3 size;
_sampler.m_texture.GetDimensions(size.x, size.y, size.z);
return _sampler.m_texture.Load(int4(_coord * size, 0) );
}
uint4 bgfxTexture3D(BgfxUSampler3D _sampler, float3 _coord)
{
uint3 size;
_sampler.m_texture.GetDimensions(size.x, size.y, size.z);
return _sampler.m_texture.Load(int4(_coord * size, 0) );
}
float4 bgfxTextureCube(BgfxSamplerCube _sampler, float3 _coord)
{
return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
}
float4 bgfxTextureCubeBias(BgfxSamplerCube _sampler, float3 _coord, float _bias)
{
return _sampler.m_texture.SampleBias(_sampler.m_sampler, _coord, _bias);
}
float4 bgfxTextureCubeLod(BgfxSamplerCube _sampler, float3 _coord, float _level)
{
return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
}
float bgfxShadowCube(BgfxSamplerCubeShadow _sampler, float4 _coord)
{
return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, _coord.xyz, _coord.w);
}
float4 bgfxTexelFetch(BgfxSampler2D _sampler, int2 _coord, int _lod)
{
return _sampler.m_texture.Load(int3(_coord, _lod) );
}
float4 bgfxTexelFetchOffset(BgfxSampler2D _sampler, int2 _coord, int _lod, int2 _offset)
{
return _sampler.m_texture.Load(int3(_coord, _lod), _offset );
}
float2 bgfxTextureSize(BgfxSampler2D _sampler, int _lod)
{
float2 result;
_sampler.m_texture.GetDimensions(result.x, result.y);
return result;
}
float4 bgfxTextureGather(BgfxSampler2D _sampler, float2 _coord)
{
return _sampler.m_texture.GatherRed(_sampler.m_sampler, _coord );
}
float4 bgfxTextureGatherOffset(BgfxSampler2D _sampler, float2 _coord, int2 _offset)
{
return _sampler.m_texture.GatherRed(_sampler.m_sampler, _coord, _offset );
}
float4 bgfxTextureGather(BgfxSampler2DArray _sampler, float3 _coord)
{
return _sampler.m_texture.GatherRed(_sampler.m_sampler, _coord );
}
int4 bgfxTexelFetch(BgfxISampler2D _sampler, int2 _coord, int _lod)
{
return _sampler.m_texture.Load(int3(_coord, _lod) );
}
uint4 bgfxTexelFetch(BgfxUSampler2D _sampler, int2 _coord, int _lod)
{
return _sampler.m_texture.Load(int3(_coord, _lod) );
}
float4 bgfxTexelFetch(BgfxSampler2DMS _sampler, int2 _coord, int _sampleIdx)
{
return _sampler.m_texture.Load(_coord, _sampleIdx);
}
float4 bgfxTexelFetch(BgfxSampler2DArray _sampler, int3 _coord, int _lod)
{
return _sampler.m_texture.Load(int4(_coord, _lod) );
}
float4 bgfxTexelFetch(BgfxSampler3D _sampler, int3 _coord, int _lod)
{
return _sampler.m_texture.Load(int4(_coord, _lod) );
}
float3 bgfxTextureSize(BgfxSampler3D _sampler, int _lod)
{
float3 result;
_sampler.m_texture.GetDimensions(result.x, result.y, result.z);
return result;
}
float3 instMul(float3 _vec, float3x3 _mtx) { return mul(_mtx, _vec); }
float3 instMul(float3x3 _mtx, float3 _vec) { return mul(_vec, _mtx); }
float4 instMul(float4 _vec, float4x4 _mtx) { return mul(_mtx, _vec); }
float4 instMul(float4x4 _mtx, float4 _vec) { return mul(_vec, _mtx); }
bool2 lessThan(float2 _a, float2 _b) { return _a < _b; }
bool3 lessThan(float3 _a, float3 _b) { return _a < _b; }
bool4 lessThan(float4 _a, float4 _b) { return _a < _b; }
bool2 lessThanEqual(float2 _a, float2 _b) { return _a <= _b; }
bool3 lessThanEqual(float3 _a, float3 _b) { return _a <= _b; }
bool4 lessThanEqual(float4 _a, float4 _b) { return _a <= _b; }
bool2 greaterThan(float2 _a, float2 _b) { return _a > _b; }
bool3 greaterThan(float3 _a, float3 _b) { return _a > _b; }
bool4 greaterThan(float4 _a, float4 _b) { return _a > _b; }
bool2 greaterThanEqual(float2 _a, float2 _b) { return _a >= _b; }
bool3 greaterThanEqual(float3 _a, float3 _b) { return _a >= _b; }
bool4 greaterThanEqual(float4 _a, float4 _b) { return _a >= _b; }
bool2 notEqual(float2 _a, float2 _b) { return _a != _b; }
bool3 notEqual(float3 _a, float3 _b) { return _a != _b; }
bool4 notEqual(float4 _a, float4 _b) { return _a != _b; }
bool2 equal(float2 _a, float2 _b) { return _a == _b; }
bool3 equal(float3 _a, float3 _b) { return _a == _b; }
bool4 equal(float4 _a, float4 _b) { return _a == _b; }
float mix(float _a, float _b, float _t) { return lerp(_a, _b, _t); }
float2 mix(float2 _a, float2 _b, float2 _t) { return lerp(_a, _b, _t); }
float3 mix(float3 _a, float3 _b, float3 _t) { return lerp(_a, _b, _t); }
float4 mix(float4 _a, float4 _b, float4 _t) { return lerp(_a, _b, _t); }
float mod(float _a, float _b) { return _a - _b * floor(_a / _b); }
float2 mod(float2 _a, float2 _b) { return _a - _b * floor(_a / _b); }
float3 mod(float3 _a, float3 _b) { return _a - _b * floor(_a / _b); }
float4 mod(float4 _a, float4 _b) { return _a - _b * floor(_a / _b); }
float2 vec2_splat(float _x) { return float2(_x, _x); }
float3 vec3_splat(float _x) { return float3(_x, _x, _x); }
float4 vec4_splat(float _x) { return float4(_x, _x, _x, _x); }
uint2 uvec2_splat(uint _x) { return uint2(_x, _x); }
uint3 uvec3_splat(uint _x) { return uint3(_x, _x, _x); }
uint4 uvec4_splat(uint _x) { return uint4(_x, _x, _x, _x); }
float4x4 mtxFromRows(float4 _0, float4 _1, float4 _2, float4 _3)
{
return float4x4(_0, _1, _2, _3);
}
float4x4 mtxFromCols(float4 _0, float4 _1, float4 _2, float4 _3)
{
return transpose(float4x4(_0, _1, _2, _3) );
}
float3x3 mtxFromRows(float3 _0, float3 _1, float3 _2)
{
return float3x3(_0, _1, _2);
}
float3x3 mtxFromCols(float3 _0, float3 _1, float3 _2)
{
return transpose(float3x3(_0, _1, _2) );
}
static float4 u_viewRect;
static float4 u_viewTexel;
static float4x4 u_view;
static float4x4 u_invView;
static float4x4 u_proj;
static float4x4 u_invProj;
static float4x4 u_viewProj;
static float4x4 u_invViewProj;
static float4x4 u_model[32];
static float4x4 u_modelView;
static float4x4 u_modelViewProj;
static float4 u_alphaRef4;
float4 encodeRE8(float _r)
{
float exponent = ceil(log2(_r) );
return float4(_r / exp2(exponent)
, 0.0
, 0.0
, (exponent + 128.0) / 255.0
);
}
float decodeRE8(float4 _re8)
{
float exponent = _re8.w * 255.0 - 128.0;
return _re8.x * exp2(exponent);
}
float4 encodeRGBE8(float3 _rgb)
{
float4 rgbe8;
float maxComponent = max(max(_rgb.x, _rgb.y), _rgb.z);
float exponent = ceil(log2(maxComponent) );
rgbe8.xyz = _rgb / exp2(exponent);
rgbe8.w = (exponent + 128.0) / 255.0;
return rgbe8;
}
float3 decodeRGBE8(float4 _rgbe8)
{
float exponent = _rgbe8.w * 255.0 - 128.0;
float3 rgb = _rgbe8.xyz * exp2(exponent);
return rgb;
}
float3 encodeNormalUint(float3 _normal)
{
return _normal * 0.5 + 0.5;
}
float3 decodeNormalUint(float3 _encodedNormal)
{
return _encodedNormal * 2.0 - 1.0;
}
float2 encodeNormalSphereMap(float3 _normal)
{
return normalize(_normal.xy) * sqrt(_normal.z * 0.5 + 0.5);
}
float3 decodeNormalSphereMap(float2 _encodedNormal)
{
float zz = dot(_encodedNormal, _encodedNormal) * 2.0 - 1.0;
return float3(normalize(_encodedNormal.xy) * sqrt(1.0 - zz*zz), zz);
}
float2 octahedronWrap(float2 _val)
{
return (1.0 - abs(_val.yx) )
* mix(vec2_splat(-1.0), vec2_splat(1.0), float2(greaterThanEqual(_val.xy, vec2_splat(0.0) ) ) );
}
float2 encodeNormalOctahedron(float3 _normal)
{
_normal /= abs(_normal.x) + abs(_normal.y) + abs(_normal.z);
_normal.xy = _normal.z >= 0.0 ? _normal.xy : octahedronWrap(_normal.xy);
_normal.xy = _normal.xy * 0.5 + 0.5;
return _normal.xy;
}
float3 decodeNormalOctahedron(float2 _encodedNormal)
{
_encodedNormal = _encodedNormal * 2.0 - 1.0;
float3 normal;
normal.z = 1.0 - abs(_encodedNormal.x) - abs(_encodedNormal.y);
normal.xy = normal.z >= 0.0 ? _encodedNormal.xy : octahedronWrap(_encodedNormal.xy);
return normalize(normal);
}
float3 convertRGB2XYZ(float3 _rgb)
{
float3 xyz;
xyz.x = dot(float3(0.4124564, 0.3575761, 0.1804375), _rgb);
xyz.y = dot(float3(0.2126729, 0.7151522, 0.0721750), _rgb);
xyz.z = dot(float3(0.0193339, 0.1191920, 0.9503041), _rgb);
return xyz;
}
float3 convertXYZ2RGB(float3 _xyz)
{
float3 rgb;
rgb.x = dot(float3( 3.2404542, -1.5371385, -0.4985314), _xyz);
rgb.y = dot(float3(-0.9692660, 1.8760108, 0.0415560), _xyz);
rgb.z = dot(float3( 0.0556434, -0.2040259, 1.0572252), _xyz);
return rgb;
}
float3 convertXYZ2Yxy(float3 _xyz)
{
float inv = 1.0/dot(_xyz, float3(1.0, 1.0, 1.0) );
return float3(_xyz.y, _xyz.x*inv, _xyz.y*inv);
}
float3 convertYxy2XYZ(float3 _Yxy)
{
float3 xyz;
xyz.x = _Yxy.x*_Yxy.y/_Yxy.z;
xyz.y = _Yxy.x;
xyz.z = _Yxy.x*(1.0 - _Yxy.y - _Yxy.z)/_Yxy.z;
return xyz;
}
float3 convertRGB2Yxy(float3 _rgb)
{
return convertXYZ2Yxy(convertRGB2XYZ(_rgb) );
}
float3 convertYxy2RGB(float3 _Yxy)
{
return convertXYZ2RGB(convertYxy2XYZ(_Yxy) );
}
float3 convertRGB2Yuv(float3 _rgb)
{
float3 yuv;
yuv.x = dot(_rgb, float3(0.299, 0.587, 0.114) );
yuv.y = (_rgb.x - yuv.x)*0.713 + 0.5;
yuv.z = (_rgb.z - yuv.x)*0.564 + 0.5;
return yuv;
}
float3 convertYuv2RGB(float3 _yuv)
{
float3 rgb;
rgb.x = _yuv.x + 1.403*(_yuv.y-0.5);
rgb.y = _yuv.x - 0.344*(_yuv.y-0.5) - 0.714*(_yuv.z-0.5);
rgb.z = _yuv.x + 1.773*(_yuv.z-0.5);
return rgb;
}
float3 convertRGB2YIQ(float3 _rgb)
{
float3 yiq;
yiq.x = dot(float3(0.299, 0.587, 0.114 ), _rgb);
yiq.y = dot(float3(0.595716, -0.274453, -0.321263), _rgb);
yiq.z = dot(float3(0.211456, -0.522591, 0.311135), _rgb);
return yiq;
}
float3 convertYIQ2RGB(float3 _yiq)
{
float3 rgb;
rgb.x = dot(float3(1.0, 0.9563, 0.6210), _yiq);
rgb.y = dot(float3(1.0, -0.2721, -0.6474), _yiq);
rgb.z = dot(float3(1.0, -1.1070, 1.7046), _yiq);
return rgb;
}
float3 toLinear(float3 _rgb)
{
return pow(abs(_rgb), vec3_splat(2.2) );
}
float4 toLinear(float4 _rgba)
{
return float4(toLinear(_rgba.xyz), _rgba.w);
}
float3 toLinearAccurate(float3 _rgb)
{
float3 lo = _rgb / 12.92;
float3 hi = pow( (_rgb + 0.055) / 1.055, vec3_splat(2.4) );
float3 rgb = mix(hi, lo, float3(lessThanEqual(_rgb, vec3_splat(0.04045) ) ) );
return rgb;
}
float4 toLinearAccurate(float4 _rgba)
{
return float4(toLinearAccurate(_rgba.xyz), _rgba.w);
}
float toGamma(float _r)
{
return pow(abs(_r), 1.0/2.2);
}
float3 toGamma(float3 _rgb)
{
return pow(abs(_rgb), vec3_splat(1.0/2.2) );
}
float4 toGamma(float4 _rgba)
{
return float4(toGamma(_rgba.xyz), _rgba.w);
}
float3 toGammaAccurate(float3 _rgb)
{
float3 lo = _rgb * 12.92;
float3 hi = pow(abs(_rgb), vec3_splat(1.0/2.4) ) * 1.055 - 0.055;
float3 rgb = mix(hi, lo, float3(lessThanEqual(_rgb, vec3_splat(0.0031308) ) ) );
return rgb;
}
float4 toGammaAccurate(float4 _rgba)
{
return float4(toGammaAccurate(_rgba.xyz), _rgba.w);
}
float3 toReinhard(float3 _rgb)
{
return toGamma(_rgb/(_rgb+vec3_splat(1.0) ) );
}
float4 toReinhard(float4 _rgba)
{
return float4(toReinhard(_rgba.xyz), _rgba.w);
}
float3 toFilmic(float3 _rgb)
{
_rgb = max(vec3_splat(0.0), _rgb - 0.004);
_rgb = (_rgb*(6.2*_rgb + 0.5) ) / (_rgb*(6.2*_rgb + 1.7) + 0.06);
return _rgb;
}
float4 toFilmic(float4 _rgba)
{
return float4(toFilmic(_rgba.xyz), _rgba.w);
}
float3 toAcesFilmic(float3 _rgb)
{
float aa = 2.51f;
float bb = 0.03f;
float cc = 2.43f;
float dd = 0.59f;
float ee = 0.14f;
return saturate( (_rgb*(aa*_rgb + bb) )/(_rgb*(cc*_rgb + dd) + ee) );
}
float4 toAcesFilmic(float4 _rgba)
{
return float4(toAcesFilmic(_rgba.xyz), _rgba.w);
}
float3 luma(float3 _rgb)
{
float yy = dot(float3(0.2126729, 0.7151522, 0.0721750), _rgb);
return vec3_splat(yy);
}
float4 luma(float4 _rgba)
{
return float4(luma(_rgba.xyz), _rgba.w);
}
float3 conSatBri(float3 _rgb, float3 _csb)
{
float3 rgb = _rgb * _csb.z;
rgb = mix(luma(rgb), rgb, _csb.y);
rgb = mix(vec3_splat(0.5), rgb, _csb.x);
return rgb;
}
float4 conSatBri(float4 _rgba, float3 _csb)
{
return float4(conSatBri(_rgba.xyz, _csb), _rgba.w);
}
float3 posterize(float3 _rgb, float _numColors)
{
return floor(_rgb*_numColors) / _numColors;
}
float4 posterize(float4 _rgba, float _numColors)
{
return float4(posterize(_rgba.xyz, _numColors), _rgba.w);
}
float3 sepia(float3 _rgb)
{
float3 color;
color.x = dot(_rgb, float3(0.393, 0.769, 0.189) );
color.y = dot(_rgb, float3(0.349, 0.686, 0.168) );
color.z = dot(_rgb, float3(0.272, 0.534, 0.131) );
return color;
}
float4 sepia(float4 _rgba)
{
return float4(sepia(_rgba.xyz), _rgba.w);
}
float3 blendOverlay(float3 _base, float3 _blend)
{
float3 lt = 2.0 * _base * _blend;
float3 gte = 1.0 - 2.0 * (1.0 - _base) * (1.0 - _blend);
return mix(lt, gte, step(vec3_splat(0.5), _base) );
}
float4 blendOverlay(float4 _base, float4 _blend)
{
return float4(blendOverlay(_base.xyz, _blend.xyz), _base.w);
}
float3 adjustHue(float3 _rgb, float _hue)
{
float3 yiq = convertRGB2YIQ(_rgb);
float angle = _hue + atan2(yiq.z, yiq.y);
float len = length(yiq.yz);
return convertYIQ2RGB(float3(yiq.x, len*cos(angle), len*sin(angle) ) );
}
float4 packFloatToRgba(float _value)
{
const float4 shift = float4(256 * 256 * 256, 256 * 256, 256, 1.0);
const float4 mask = float4(0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
float4 comp = frac(_value * shift);
comp -= comp.xxyz * mask;
return comp;
}
float unpackRgbaToFloat(float4 _rgba)
{
const float4 shift = float4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
return dot(_rgba, shift);
}
float2 packHalfFloat(float _value)
{
const float2 shift = float2(256, 1.0);
const float2 mask = float2(0, 1.0 / 256.0);
float2 comp = frac(_value * shift);
comp -= comp.xx * mask;
return comp;
}
float unpackHalfFloat(float2 _rg)
{
const float2 shift = float2(1.0 / 256.0, 1.0);
return dot(_rg, shift);
}
float random(float2 _uv)
{
return frac(sin(dot(_uv.xy, float2(12.9898, 78.233) ) ) * 43758.5453);
}
float3 fixCubeLookup(float3 _v, float _lod, float _topLevelCubeSize)
{
float ax = abs(_v.x);
float ay = abs(_v.y);
float az = abs(_v.z);
float vmax = max(max(ax, ay), az);
float scale = 1.0 - exp2(_lod) / _topLevelCubeSize;
if (ax != vmax) { _v.x *= scale; }
if (ay != vmax) { _v.y *= scale; }
if (az != vmax) { _v.z *= scale; }
return _v;
}
float2 texture2DBc5(BgfxSampler2D _sampler, float2 _uv)
{
return bgfxTexture2D(_sampler, _uv).xy;
}
float3x3 mat3FromCols(float3 c0, float3 c1, float3 c2)
{
return transpose(float3x3(c0, c1, c2));
}
float clampDot(float3 v1, float3 v2) {
return clamp(dot(v1, v2), 0.0, 1.0);
}
float reinhard(float _x)
{
return _x / (_x + 1.0);
}
float3 reinhard(float3 _x)
{
return _x / (_x + 1.0);
}
float reinhard2(float _x, float _whiteSqr)
{
return (_x * (1.0 + _x/_whiteSqr) ) / (1.0 + _x);
}
float3 reinhard2(float3 _x, float _whiteSqr)
{
return (_x * (1.0 + _x/_whiteSqr) ) / (1.0 + _x);
}
float4 imageLoad(Texture2D<float> _image, int2 _uv) { return _image[_uv].xxxx; } int2 imageSize(Texture2D<float> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(RWTexture2D<float> _image, int2 _uv) { return _image[_uv].xxxx; } void imageStore(RWTexture2D<float> _image, int2 _uv, float4 _value) { _image[_uv] = _value.x; } int2 imageSize(RWTexture2D<float> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(Texture2DArray<float> _image, int3 _uvw) { return _image[_uvw].xxxx; } int3 imageSize(Texture2DArray<float> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture2DArray<float> _image, int3 _uvw) { return _image[_uvw].xxxx; } void imageStore(RWTexture2DArray<float> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.x; } int3 imageSize(RWTexture2DArray<float> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(Texture3D<float> _image, int3 _uvw) { return _image[_uvw].xxxx; } int3 imageSize(Texture3D<float> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture3D<float> _image, int3 _uvw) { return _image[_uvw].xxxx; } void imageStore(RWTexture3D<float> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.x; } int3 imageSize(RWTexture3D<float> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); }
float4 imageLoad(Texture2D<float2> _image, int2 _uv) { return _image[_uv].xyyy; } int2 imageSize(Texture2D<float2> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(RWTexture2D<float2> _image, int2 _uv) { return _image[_uv].xyyy; } void imageStore(RWTexture2D<float2> _image, int2 _uv, float4 _value) { _image[_uv] = _value.xy; } int2 imageSize(RWTexture2D<float2> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(Texture2DArray<float2> _image, int3 _uvw) { return _image[_uvw].xyyy; } int3 imageSize(Texture2DArray<float2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture2DArray<float2> _image, int3 _uvw) { return _image[_uvw].xyyy; } void imageStore(RWTexture2DArray<float2> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.xy; } int3 imageSize(RWTexture2DArray<float2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(Texture3D<float2> _image, int3 _uvw) { return _image[_uvw].xyyy; } int3 imageSize(Texture3D<float2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture3D<float2> _image, int3 _uvw) { return _image[_uvw].xyyy; } void imageStore(RWTexture3D<float2> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.xy; } int3 imageSize(RWTexture3D<float2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); }
float4 imageLoad(Texture2D<float4> _image, int2 _uv) { return _image[_uv].xyzw; } int2 imageSize(Texture2D<float4> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(RWTexture2D<float4> _image, int2 _uv) { return _image[_uv].xyzw; } void imageStore(RWTexture2D<float4> _image, int2 _uv, float4 _value) { _image[_uv] = _value.xyzw; } int2 imageSize(RWTexture2D<float4> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(Texture2DArray<float4> _image, int3 _uvw) { return _image[_uvw].xyzw; } int3 imageSize(Texture2DArray<float4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture2DArray<float4> _image, int3 _uvw) { return _image[_uvw].xyzw; } void imageStore(RWTexture2DArray<float4> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.xyzw; } int3 imageSize(RWTexture2DArray<float4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(Texture3D<float4> _image, int3 _uvw) { return _image[_uvw].xyzw; } int3 imageSize(Texture3D<float4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture3D<float4> _image, int3 _uvw) { return _image[_uvw].xyzw; } void imageStore(RWTexture3D<float4> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.xyzw; } int3 imageSize(RWTexture3D<float4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); }
uint4 imageLoad(Texture2D<uint> _image, int2 _uv) { return _image[_uv].xxxx; } int2 imageSize(Texture2D<uint> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } uint4 imageLoad(RWTexture2D<uint> _image, int2 _uv) { return _image[_uv].xxxx; } void imageStore(RWTexture2D<uint> _image, int2 _uv, uint4 _value) { _image[_uv] = _value.x; } int2 imageSize(RWTexture2D<uint> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } uint4 imageLoad(Texture2DArray<uint> _image, int3 _uvw) { return _image[_uvw].xxxx; } int3 imageSize(Texture2DArray<uint> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } uint4 imageLoad(RWTexture2DArray<uint> _image, int3 _uvw) { return _image[_uvw].xxxx; } void imageStore(RWTexture2DArray<uint> _image, int3 _uvw, uint4 _value) { _image[_uvw] = _value.x; } int3 imageSize(RWTexture2DArray<uint> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } uint4 imageLoad(Texture3D<uint> _image, int3 _uvw) { return _image[_uvw].xxxx; } int3 imageSize(Texture3D<uint> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } uint4 imageLoad(RWTexture3D<uint> _image, int3 _uvw) { return _image[_uvw].xxxx; } void imageStore(RWTexture3D<uint> _image, int3 _uvw, uint4 _value) { _image[_uvw] = _value.x; } int3 imageSize(RWTexture3D<uint> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); }
uint4 imageLoad(Texture2D<uint2> _image, int2 _uv) { return _image[_uv].xyyy; } int2 imageSize(Texture2D<uint2> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } uint4 imageLoad(RWTexture2D<uint2> _image, int2 _uv) { return _image[_uv].xyyy; } void imageStore(RWTexture2D<uint2> _image, int2 _uv, uint4 _value) { _image[_uv] = _value.xy; } int2 imageSize(RWTexture2D<uint2> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } uint4 imageLoad(Texture2DArray<uint2> _image, int3 _uvw) { return _image[_uvw].xyyy; } int3 imageSize(Texture2DArray<uint2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } uint4 imageLoad(RWTexture2DArray<uint2> _image, int3 _uvw) { return _image[_uvw].xyyy; } void imageStore(RWTexture2DArray<uint2> _image, int3 _uvw, uint4 _value) { _image[_uvw] = _value.xy; } int3 imageSize(RWTexture2DArray<uint2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } uint4 imageLoad(Texture3D<uint2> _image, int3 _uvw) { return _image[_uvw].xyyy; } int3 imageSize(Texture3D<uint2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } uint4 imageLoad(RWTexture3D<uint2> _image, int3 _uvw) { return _image[_uvw].xyyy; } void imageStore(RWTexture3D<uint2> _image, int3 _uvw, uint4 _value) { _image[_uvw] = _value.xy; } int3 imageSize(RWTexture3D<uint2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); }
uint4 imageLoad(Texture2D<uint4> _image, int2 _uv) { return _image[_uv].xyzw; } int2 imageSize(Texture2D<uint4> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } uint4 imageLoad(RWTexture2D<uint4> _image, int2 _uv) { return _image[_uv].xyzw; } void imageStore(RWTexture2D<uint4> _image, int2 _uv, uint4 _value) { _image[_uv] = _value.xyzw; } int2 imageSize(RWTexture2D<uint4> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } uint4 imageLoad(Texture2DArray<uint4> _image, int3 _uvw) { return _image[_uvw].xyzw; } int3 imageSize(Texture2DArray<uint4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } uint4 imageLoad(RWTexture2DArray<uint4> _image, int3 _uvw) { return _image[_uvw].xyzw; } void imageStore(RWTexture2DArray<uint4> _image, int3 _uvw, uint4 _value) { _image[_uvw] = _value.xyzw; } int3 imageSize(RWTexture2DArray<uint4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } uint4 imageLoad(Texture3D<uint4> _image, int3 _uvw) { return _image[_uvw].xyzw; } int3 imageSize(Texture3D<uint4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } uint4 imageLoad(RWTexture3D<uint4> _image, int3 _uvw) { return _image[_uvw].xyzw; } void imageStore(RWTexture3D<uint4> _image, int3 _uvw, uint4 _value) { _image[_uvw] = _value.xyzw; } int3 imageSize(RWTexture3D<uint4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); }
float4 imageLoad(Texture2D<unorm float> _image, int2 _uv) { return _image[_uv].xxxx; } int2 imageSize(Texture2D<unorm float> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(RWTexture2D<unorm float> _image, int2 _uv) { return _image[_uv].xxxx; } void imageStore(RWTexture2D<unorm float> _image, int2 _uv, float4 _value) { _image[_uv] = _value.x; } int2 imageSize(RWTexture2D<unorm float> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(Texture2DArray<unorm float> _image, int3 _uvw) { return _image[_uvw].xxxx; } int3 imageSize(Texture2DArray<unorm float> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture2DArray<unorm float> _image, int3 _uvw) { return _image[_uvw].xxxx; } void imageStore(RWTexture2DArray<unorm float> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.x; } int3 imageSize(RWTexture2DArray<unorm float> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(Texture3D<unorm float> _image, int3 _uvw) { return _image[_uvw].xxxx; } int3 imageSize(Texture3D<unorm float> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture3D<unorm float> _image, int3 _uvw) { return _image[_uvw].xxxx; } void imageStore(RWTexture3D<unorm float> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.x; } int3 imageSize(RWTexture3D<unorm float> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); }
float4 imageLoad(Texture2D<unorm float2> _image, int2 _uv) { return _image[_uv].xyyy; } int2 imageSize(Texture2D<unorm float2> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(RWTexture2D<unorm float2> _image, int2 _uv) { return _image[_uv].xyyy; } void imageStore(RWTexture2D<unorm float2> _image, int2 _uv, float4 _value) { _image[_uv] = _value.xy; } int2 imageSize(RWTexture2D<unorm float2> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(Texture2DArray<unorm float2> _image, int3 _uvw) { return _image[_uvw].xyyy; } int3 imageSize(Texture2DArray<unorm float2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture2DArray<unorm float2> _image, int3 _uvw) { return _image[_uvw].xyyy; } void imageStore(RWTexture2DArray<unorm float2> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.xy; } int3 imageSize(RWTexture2DArray<unorm float2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(Texture3D<unorm float2> _image, int3 _uvw) { return _image[_uvw].xyyy; } int3 imageSize(Texture3D<unorm float2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture3D<unorm float2> _image, int3 _uvw) { return _image[_uvw].xyyy; } void imageStore(RWTexture3D<unorm float2> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.xy; } int3 imageSize(RWTexture3D<unorm float2> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); }
float4 imageLoad(Texture2D<unorm float4> _image, int2 _uv) { return _image[_uv].xyzw; } int2 imageSize(Texture2D<unorm float4> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(RWTexture2D<unorm float4> _image, int2 _uv) { return _image[_uv].xyzw; } void imageStore(RWTexture2D<unorm float4> _image, int2 _uv, float4 _value) { _image[_uv] = _value.xyzw; } int2 imageSize(RWTexture2D<unorm float4> _image) { uint2 result; _image.GetDimensions(result.x, result.y); return int2(result); } float4 imageLoad(Texture2DArray<unorm float4> _image, int3 _uvw) { return _image[_uvw].xyzw; } int3 imageSize(Texture2DArray<unorm float4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture2DArray<unorm float4> _image, int3 _uvw) { return _image[_uvw].xyzw; } void imageStore(RWTexture2DArray<unorm float4> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.xyzw; } int3 imageSize(RWTexture2DArray<unorm float4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(Texture3D<unorm float4> _image, int3 _uvw) { return _image[_uvw].xyzw; } int3 imageSize(Texture3D<unorm float4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); } float4 imageLoad(RWTexture3D<unorm float4> _image, int3 _uvw) { return _image[_uvw].xyzw; } void imageStore(RWTexture3D<unorm float4> _image, int3 _uvw, float4 _value) { _image[_uvw] = _value.xyzw; } int3 imageSize(RWTexture3D<unorm float4> _image) { uint3 result; _image.GetDimensions(result.x, result.y, result.z); return int3(result); }
void imageAtomicAdd(RWTexture2D<uint> _image, int2 _uv, uint4 _value) { InterlockedAdd(_image[_uv], _value.x); }
float4 screen2Eye(float4 coord)
{
float3 ndc = float3(
2.0 * (coord.x - u_viewRect.x) / u_viewRect.z - 1.0,
2.0 * (u_viewRect.w - coord.y - 1 - u_viewRect.y) / u_viewRect.w - 1.0,
coord.z
);
float4 eye = mul(u_invProj, float4(ndc, 1.0));
eye = eye / eye.w;
return eye;
}
float screen2EyeDepth(float depth, float near, float far)
{
float ndc = depth;
float eye = far * near / (far + ndc * (near - far));
return eye;
}
float3 convertTangentNormal(float3 normal_ref, float3 tangent_ref, float3 normal)
{
float3 bitangent = cross(normal_ref, tangent_ref);
float3x3 TBN = mtxFromCols(
normalize(tangent_ref),
normalize(bitangent),
normalize(normal_ref)
);
return normalize(mul(TBN, normal));
}
float2 packNormal(float3 normal)
{
float f = sqrt(8.0 * -normal.z + 8.0);
return normal.xy / f + 0.5;
}
float3 unpackNormal(float2 encoded)
{
float2 fenc = encoded * 4.0 - 2.0;
float f = dot(fenc, fenc);
float g = sqrt(1.0 - f * 0.25);
return float3(fenc * g, -(1.0 - f * 0.5));
}
uniform float4 u_texTransformMask;
struct TextureTransform
{
float2 offset;
float2 scale;
float rotation;
uint texCoord;
uint mask;
};
Buffer<float2> b_TextureTransforms : register(t[13]);
float2 getTexCoord(uint texSlot,float2 _texcoord)
{
return _texcoord;
}
TextureTransform getTransform(uint i)
{
int index = 3 * i;
TextureTransform transForm;
transForm.offset = b_TextureTransforms[index + 0];
transForm.scale = b_TextureTransforms[index + 1];
transForm.rotation = b_TextureTransforms[index + 2].x;
half2 fp16 = b_TextureTransforms[index + 2].y;
transForm.texCoord = uint(fp16.x);
transForm.mask = uint(fp16.y);
return transForm;
}
float2 getTexCoordT(TextureTransform transform , float2 _texcoord)
{
float2 _offset = transform.offset;
float2 _scale = transform.scale;
float _rot = transform.rotation;
float3x3 translation = float3x3(1,0,0,
0,1,0,
_offset.x,_offset.y,1);
float3x3 rotation = float3x3(
cos(_rot), sin(_rot), 0,
-sin(_rot), cos(_rot), 0,
0, 0, 1
);
float3x3 scale = float3x3(0,0,0,
_scale.x,_scale.y,0,
0,0,1);
float3x3 target = mul(mul(translation,rotation),scale);
return ( mul(target,float3(_texcoord, 1)) ).xy;
}
float2 pbrBaseColorTexCoord(float2 texcoord)
{
if (((uint(u_texTransformMask.x) & (1 << 1)) != 0))
{
half2 fp16 = u_texTransformMask.y;
TextureTransform transform = getTransform(fp16.x);
return getTexCoordT(transform, texcoord);
}
else
return texcoord;
}
uniform SamplerState s_texAlbedoLUTSampler : register(s[0]); uniform Texture2D s_texAlbedoLUTTexture : register(t[0]); static BgfxSampler2D s_texAlbedoLUT = { s_texAlbedoLUTSampler, s_texAlbedoLUTTexture };
uniform SamplerState s_texBaseColorSampler : register(s[1]); uniform Texture2D s_texBaseColorTexture : register(t[1]); static BgfxSampler2D s_texBaseColor = { s_texBaseColorSampler, s_texBaseColorTexture };
uniform SamplerState s_texMetallicRoughnessSampler : register(s[2]); uniform Texture2D s_texMetallicRoughnessTexture : register(t[2]); static BgfxSampler2D s_texMetallicRoughness = { s_texMetallicRoughnessSampler, s_texMetallicRoughnessTexture };
uniform SamplerState s_texNormalSampler : register(s[3]); uniform Texture2D s_texNormalTexture : register(t[3]); static BgfxSampler2D s_texNormal = { s_texNormalSampler, s_texNormalTexture };
uniform SamplerState s_texOcclusionSampler : register(s[4]); uniform Texture2D s_texOcclusionTexture : register(t[4]); static BgfxSampler2D s_texOcclusion = { s_texOcclusionSampler, s_texOcclusionTexture };
uniform SamplerState s_texEmissiveSampler : register(s[5]); uniform Texture2D s_texEmissiveTexture : register(t[5]); static BgfxSampler2D s_texEmissive = { s_texEmissiveSampler, s_texEmissiveTexture };
uniform float4 u_baseColorFactor;
uniform float4 u_metallicRoughnessNormalOcclusionFactor;
uniform float4 u_emissiveFactorVec;
uniform float4 u_hasTextures;
uniform float4 u_multipleScatteringVec;
struct PBRMaterial
{
float4 albedo;
float metallic;
float roughness;
float3 normal;
float occlusion;
float3 emissive;
float3 diffuseColor;
float3 F0;
float a;
};
float4 pbrBaseColor(float2 texcoord)
{
float2 finalCoord = vec2_splat(0.0f);
if(((uint(u_hasTextures.x) & (1 << 1)) != 0))
{
if (((uint(u_hasTextures.x) & (1 << 10)) != 0))
finalCoord = pbrBaseColorTexCoord(texcoord);
else
finalCoord = texcoord;
return bgfxTexture2D(s_texBaseColor, finalCoord ) * u_baseColorFactor;
}
else
{
return u_baseColorFactor;
}
}
float2 pbrMetallicRoughness(float2 texcoord)
{
if(((uint(u_hasTextures.x) & (1 << 2)) != 0))
{
return bgfxTexture2D(s_texMetallicRoughness, texcoord).bg * (u_metallicRoughnessNormalOcclusionFactor.xy);
}
else
{
return (u_metallicRoughnessNormalOcclusionFactor.xy);
}
}
float3 pbrNormal(float2 texcoord)
{
if(((uint(u_hasTextures.x) & (1 << 3)) != 0))
{
return normalize((bgfxTexture2D(s_texNormal, texcoord).rgb * 2.0) - 1.0) * (u_metallicRoughnessNormalOcclusionFactor.z);
}
else
{
return float3(0.0, 0.0, 1.0);
}
}
float pbrOcclusion(float2 texcoord)
{
if(((uint(u_hasTextures.x) & (1 << 4)) != 0))
{
float occlusion = bgfxTexture2D(s_texOcclusion, texcoord).r;
return occlusion + (1.0 - occlusion) * (1.0 - (u_metallicRoughnessNormalOcclusionFactor.w));
}
else
{
return 1.0;
}
}
float3 pbrEmissive(float2 texcoord)
{
if(((uint(u_hasTextures.x) & (1 << 5)) != 0))
{
return bgfxTexture2D(s_texEmissive, texcoord).rgb * (u_emissiveFactorVec.xyz);
}
else
{
return (u_emissiveFactorVec.xyz);
}
}
PBRMaterial pbrInitMaterial(PBRMaterial mat);
PBRMaterial pbrMaterial(float2 texcoord)
{
PBRMaterial mat;
mat.albedo = pbrBaseColor(texcoord);
float2 metallicRoughness = pbrMetallicRoughness(texcoord);
mat.metallic = metallicRoughness.r;
mat.roughness = metallicRoughness.g;
mat.normal = pbrNormal(texcoord);
mat.occlusion = pbrOcclusion(texcoord);
mat.emissive = pbrEmissive(texcoord);
mat = pbrInitMaterial(mat);
return mat;
}
PBRMaterial pbrInitMaterial(PBRMaterial mat)
{
const float3 dielectricSpecular = float3(0.04, 0.04, 0.04);
const float3 black = float3(0.0, 0.0, 0.0);
mat.diffuseColor = mix(mat.albedo.rgb * (vec3_splat(1.0) - dielectricSpecular), black, mat.metallic);
mat.F0 = mix(dielectricSpecular, mat.albedo.rgb, mat.metallic);
mat.a = mat.roughness * mat.roughness;
mat.a = max(mat.a, 0.01);
return mat;
}
float specularAntiAliasing(float3 N, float a)
{
const float SIGMA2 = 0.25;
const float KAPPA = 0.18;
float3 dndu = ddx(N);
float3 dndv = ddy(-N);
float variance = SIGMA2 * (dot(dndu, dndu) + dot(dndv, dndv));
float kernelRoughness2 = min(2.0 * variance, KAPPA);
return saturate(a + kernelRoughness2);
}
float3 F_Schlick(float VoH, float3 F0)
{
float f = pow(1.0 - VoH, 5.0);
return f + F0 * (1.0 - f);
}
float D_GGX(float NoH, float a)
{
a = NoH * a;
float k = a / (1.0 - NoH * NoH + a * a);
return k * k * (0.31830988618);
}
float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
float a2 = a * a;
float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
return 0.5 / (GGXV + GGXL);
}
float V_SmithGGX(float NoV, float NoL, float a)
{
float a2 = a * a;
float GGXV = NoV + sqrt(NoV * NoV * (1.0 - a2) + a2);
float GGXL = NoL + sqrt(NoL * NoL * (1.0 - a2) + a2);
return 1.0 / (GGXV * GGXL);
}
float Fd_Lambert()
{
return (0.31830988618);
}
float3 multipleScatteringFactor(PBRMaterial mat, float NoV)
{
if((u_multipleScatteringVec.x != 0.0))
{
float2 E = bgfxTexture2D(s_texAlbedoLUT, float2(NoV, mat.a)).xy;
float3 factorMetallic = vec3_splat(1.0) + mat.F0 * (1.0 / E.x - 1.0);
float3 factorDielectric = vec3_splat(1.0 / E.y);
return mix(factorDielectric, factorMetallic, mat.metallic);
}
else
return vec3_splat(1.0);
}
bool whiteFurnaceEnabled()
{
return ((u_multipleScatteringVec.y) > 0.0);
}
float3 whiteFurnace(float NoV, PBRMaterial mat)
{
float2 Es = bgfxTexture2D(s_texAlbedoLUT, float2(NoV, mat.a)).xy;
float E = mix(Es.y, Es.x, mat.metallic);
return E * vec3_splat((u_multipleScatteringVec.y));
}
float3 BRDF(float3 v, float3 l, float3 n, float NoV, float NoL, PBRMaterial mat)
{
float3 h = normalize(l + v);
float NoH = saturate(dot(n, h));
float VoH = saturate(dot(v, h));
float D = D_GGX(NoH, mat.a);
float3 F = F_Schlick(VoH, mat.F0);
float V = V_SmithGGXCorrelated(NoV, NoL, mat.a);
float3 Fr = F * (V * D);
float3 Fd = mat.diffuseColor * Fd_Lambert();
return Fr + (1.0 - F) * Fd;
}
uniform float4 u_lightCountVec;
uniform float4 u_ambientLightIrradiance;
Buffer<float4> b_pointLights : register(t[6]);
struct PointLight
{
float3 position;
float _padding;
float3 intensity;
float radius;
};
struct AmbientLight
{
float3 irradiance;
};
float distanceAttenuation(float distance)
{
return 1.0 / max(distance * distance, 0.01 * 0.01);
}
float smoothAttenuation(float distance, float radius)
{
float nom = saturate(1.0 - pow(distance / radius, 4.0));
return nom * nom * distanceAttenuation(distance);
}
uint pointLightCount()
{
return uint(u_lightCountVec.x);
}
PointLight getPointLight(uint i)
{
PointLight light;
light.position = b_pointLights[2 * i + 0].xyz;
float4 intensityRadiusVec = b_pointLights[2 * i + 1];
light.intensity = intensityRadiusVec.xyz;
light.radius = intensityRadiusVec.w;
return light;
}
AmbientLight getAmbientLight()
{
AmbientLight light;
light.irradiance = u_ambientLightIrradiance.xyz;
return light;
}
struct Light
{
float3 direction;
float range;
float3 color;
float intensity;
float3 position;
float innerConeCos;
float outerConeCos;
uint type;
};
const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / 2.2;
Buffer<float4> b_Lights : register(t[12]);
const int LightType_Directional = 0;
const int LightType_Point = 1;
const int LightType_Spot = 2;
const int LightType_old = 3;
float3 linearTosRGB(float3 color)
{
return pow(color, vec3_splat(INV_GAMMA));
}
Light getLight(uint i)
{
int index = 4 * i;
Light light;
light.direction = b_Lights[index + 0].xyz;
light.range = b_Lights[index + 0].w;
light.color = b_Lights[index + 1].xyz;
light.intensity = b_Lights[index + 1].w;
light.position = b_Lights[index + 2].xyz;
light.innerConeCos = b_Lights[index + 2].w;
light.outerConeCos = b_Lights[index + 3].x;
light.type = int(b_Lights[index + 3].y);
return light;
}
float getRangeAttenuation(float range, float distance)
{
if (range <= 0.0)
{
return 1.0 / pow(distance, 2.0);
}
return max(min(1.0 - pow(distance / range, 4.0), 1.0), 0.0) / pow(distance, 2.0);
}
float getSpotAttenuation(float3 pointToLight, float3 spotDirection, float outerConeCos, float innerConeCos)
{
float actualCos = dot(normalize(spotDirection), normalize(-pointToLight));
if (actualCos > outerConeCos)
{
if (actualCos < innerConeCos)
{
return smoothstep(outerConeCos, innerConeCos, actualCos);
}
return 1.0;
}
return 0.0;
}
float3 getLighIntensity(Light light, float3 pointToLight)
{
float rangeAttenuation = 1.0;
float spotAttenuation = 1.0;
if (light.type != LightType_Directional)
{
rangeAttenuation = getRangeAttenuation(light.range, length(pointToLight));
}
if (light.type == LightType_Spot)
{
spotAttenuation = getSpotAttenuation(pointToLight, light.direction, light.outerConeCos, light.innerConeCos);
}
return rangeAttenuation * spotAttenuation * light.intensity * light.color;
}
float applyIorToRoughness(float roughness, float ior)
{
return roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);
}
float3 F_SchlickX(float3 f0, float3 f90, float VdotH)
{
return f0 + (f90 - f0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}
float V_GGX(float NdotL, float NdotV, float alphaRoughness)
{
float alphaRoughnessSq = alphaRoughness * alphaRoughness;
float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
float GGX = GGXV + GGXL;
if (GGX > 0.0)
{
return 0.5 / GGX;
}
return 0.0;
}
float D_GGXX(float NdotH, float alphaRoughness)
{
float alphaRoughnessSq = alphaRoughness * alphaRoughness;
float f = (NdotH * NdotH) * (alphaRoughnessSq - 1.0) + 1.0;
return alphaRoughnessSq / (3.1415926535897932384626433832795 * f * f);
}
float3 getPunctualRadianceTransmission(float3 normal, float3 view, float3 pointToLight, float alphaRoughness,
float3 f0, float3 f90, float3 baseColor, float ior)
{
float transmissionRougness = applyIorToRoughness(alphaRoughness, ior);
float3 n = normalize(normal);
float3 v = normalize(view);
float3 l = normalize(pointToLight);
float3 l_mirror = normalize(l + 2.0*n*dot(-l, n));
float3 h = normalize(l_mirror + v);
float D = D_GGXX(clamp(dot(n, h), 0.0, 1.0), transmissionRougness);
float VoH = saturate(dot(v, h));
float3 F = F_SchlickX(f0, f90, clamp(dot(v, h), 0.0, 1.0));
float Vis = V_GGX(clamp(dot(n, l_mirror), 0.0, 1.0), clamp(dot(n, v), 0.0, 1.0), transmissionRougness);
return (1.0 - F) * baseColor * D * Vis;
}
float clampedDot(float3 x, float3 y)
{
return clamp(dot(x, y), 0.0, 1.0);
}
float3 BRDF_specularGGX(float3 f0, float3 f90, float alphaRoughness, float specularWeight, float VdotH, float NdotL, float NdotV, float NdotH)
{
float3 F = F_SchlickX(f0, f90, VdotH);
float Vis = V_GGX(NdotL, NdotV, alphaRoughness);
float D = D_GGXX(NdotH, alphaRoughness);
return specularWeight * F * Vis * D;
}
float3 getPunctualRadianceClearCoat(float3 clearcoatNormal, float3 v, float3 l, float3 h, float VdotH, float3 f0, float3 f90, float clearcoatRoughness)
{
float NdotL = clampedDot(clearcoatNormal, l);
float NdotV = clampedDot(clearcoatNormal, v);
float NdotH = clampedDot(clearcoatNormal, h);
return NdotL * BRDF_specularGGX(f0, f90, clearcoatRoughness * clearcoatRoughness, 1.0, VdotH, NdotL, NdotV, NdotH);
}
float lambdaSheenNumericHelper(float x, float alphaG)
{
float oneMinusAlphaSq = (1.0 - alphaG) * (1.0 - alphaG);
float a = mix(21.5473, 25.3245, oneMinusAlphaSq);
float b = mix(3.82987, 3.32435, oneMinusAlphaSq);
float c = mix(0.19823, 0.16801, oneMinusAlphaSq);
float d = mix(-1.97760, -1.27393, oneMinusAlphaSq);
float e = mix(-4.32054, -4.85967, oneMinusAlphaSq);
return a / (1.0 + b * pow(x, c)) + d * x + e;
}
float lambdaSheen(float cosTheta, float alphaG)
{
if (abs(cosTheta) < 0.5)
{
return exp(lambdaSheenNumericHelper(cosTheta, alphaG));
}
else
{
return exp(2.0 * lambdaSheenNumericHelper(0.5, alphaG) - lambdaSheenNumericHelper(1.0 - cosTheta, alphaG));
}
}
float V_Sheen(float NdotL, float NdotV, float sheenRoughness)
{
sheenRoughness = max(sheenRoughness, 0.000001);
float alphaG = sheenRoughness * sheenRoughness;
return clamp(1.0 / ((1.0 + lambdaSheen(NdotV, alphaG) + lambdaSheen(NdotL, alphaG)) *
(4.0 * NdotV * NdotL)), 0.0, 1.0);
}
float D_Charlie(float sheenRoughness, float NdotH)
{
sheenRoughness = max(sheenRoughness, 0.000001);
float alphaG = sheenRoughness * sheenRoughness;
float invR = 1.0 / alphaG;
float cos2h = NdotH * NdotH;
float sin2h = 1.0 - cos2h;
return (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * 3.1415926535897932384626433832795);
}
float3 BRDF_specularSheen(float3 sheenColor, float sheenRoughness, float NdotL, float NdotV, float NdotH)
{
float sheenDistribution = D_Charlie(sheenRoughness, NdotH);
float sheenVisibility = V_Sheen(NdotL, NdotV, sheenRoughness);
return sheenColor * sheenDistribution * sheenVisibility;
}
float3 getPunctualRadianceSheen(float3 sheenColor, float sheenRoughness, float NdotL, float NdotV, float NdotH)
{
return NdotL * BRDF_specularSheen(sheenColor, sheenRoughness, NdotL, NdotV, NdotH);
}
float3 applyVolumeAttenuation(float3 radiance, float transmissionDistance, float3 attenuationColor, float attenuationDistance)
{
if (attenuationDistance == 0.0)
{
return radiance;
}
else
{
float3 attenuationCoefficient = -log(attenuationColor) / attenuationDistance;
float3 transmittance = exp(-attenuationCoefficient * transmissionDistance);
return transmittance * radiance;
}
}
float3 getVolumeTransmissionRay(float3 n, float3 v, float thickness, float ior, float4x4 modelMatrix)
{
float3 refractionVector = refract(-v, normalize(n), 1.0 / ior);
float3 modelScale;
modelScale.x = length(float3(modelMatrix[0].xyz));
modelScale.y = length(float3(modelMatrix[1].xyz));
modelScale.z = length(float3(modelMatrix[2].xyz));
return normalize(refractionVector) * thickness * modelScale;
}
uniform float4 u_fragmentOptions;
uniform float4 u_camPos;
void main( float4 gl_FragCoord : SV_POSITION , float3 v_normal : NORMAL , float4 v_tangent : TANGENT , float2 v_texcoord : TEXCOORD0 , float3 v_worldpos : POSITION1 , out float4 bgfx_FragData0 : SV_TARGET0 )
{
float4 bgfx_VoidFrag = vec4_splat(0.0);
PBRMaterial mat = pbrMaterial(v_texcoord);
float3 N =convertTangentNormal(v_normal, v_tangent.xyz, mat.normal);
mat.a = specularAntiAliasing(N, mat.a);
float3 camPos = u_camPos.xyz;
float3 fragPos = v_worldpos;
float3 V = normalize(camPos - fragPos);
float NoV = abs(dot(N, V)) + 1e-5;
if(whiteFurnaceEnabled())
{
mat.F0 = vec3_splat(1.0);
float3 msFactor = multipleScatteringFactor(mat, NoV);
float3 radianceOut = whiteFurnace(NoV, mat) * msFactor;
bgfx_FragData0 = float4(radianceOut, 1.0);
return;
}
float3 msFactor = multipleScatteringFactor(mat, NoV);
float3 radianceOut = vec3_splat(0.0);
uint lights = pointLightCount();
for(uint i = 0; i < lights; i++)
{
Light pLight = getLight(i);
float3 pointToLight;
if (pLight.type != LightType_Directional)
{
pointToLight = pLight.position - fragPos;
}
else
{
pointToLight = -pLight.direction;
}
if (pLight.type < 3.0)
{
float3 L = normalize(pointToLight);
float3 intensity = getLighIntensity(pLight, pointToLight);
float NoL = saturate(dot(N, L));
if (NoL > 0.0 || NoV > 0.0)
radianceOut += BRDF(V, L, N, NoV, NoL, mat) * intensity * NoL;
}else
{
PointLight light = getPointLight(i);
float dist = distance(light.position, fragPos);
float attenuation = smoothAttenuation(dist, light.radius);
if(attenuation > 0.0)
{
float3 L = normalize(light.position - fragPos);
float3 radianceIn = light.intensity * attenuation;
float NoL = saturate(dot(N, L));
radianceOut += BRDF(V, L, N, NoV, NoL, mat) * msFactor * radianceIn * NoL;
}
}
}
radianceOut += getAmbientLight().irradiance * mat.diffuseColor * mat.occlusion;
radianceOut += mat.emissive;
bgfx_FragData0.rgb = radianceOut;
bgfx_FragData0.a = mat.albedo.a;
if ((u_hasTextures.w) > 0 && (mat.albedo.a - ((u_hasTextures.w)-1.0)) < 0.0)
discard;
if (((uint(u_fragmentOptions.x) & (1 << 2)) != 0))
bgfx_FragData0.rgb = (N + 1.0) / 2.0;
if (((uint(u_fragmentOptions.x) & (1 << 3)) != 0))
bgfx_FragData0.rgb = ( mat.normal + 1.0) / 2.0;
if (((uint(u_fragmentOptions.x) & (1 << 1)) != 0))
bgfx_FragData0.rgb = pbrBaseColor(v_texcoord);
}
