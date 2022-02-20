#include "common.sh"
#include <bgfx_shader.sh>
#include "tonemapping.sh"

uniform vec4 u_exposureVec;
#define u_exposure u_exposureVec.x

uniform vec4 u_tonemappingModeVec;
#define u_tonemappingMode int(u_tonemappingModeVec.x)

#define TONEMAP_NONE 0
#define TONEMAP_EXPONENTIAL 1
#define TONEMAP_REINHARD 2
#define TONEMAP_REINHARD_LUM 3
#define TONEMAP_HABLE 4
#define TONEMAP_DUIKER 5
#define TONEMAP_ACES 6
#define TONEMAP_ACES_LUM 7

SAMPLER2D(s_texColor, 0);


float DigitBin(const in int x)
{
    return x==0?480599.0:x==1?139810.0:x==2?476951.0:x==3?476999.0:x==4?350020.0:x==5?464711.0:x==6?464727.0:x==7?476228.0:x==8?481111.0:x==9?481095.0:0.0;
}

// Improved version
//
// Most important change is dropping everything left of the decimal point ASAP 
// when printing the fractional digits. This is done to bring the magnitule down
// for the following division and modulo.
//
// Another change is to replace the logarithm with a power-of-ten value 
// calculation that is needed later anyway.
// This change is optional, either one works.
float PrintValue(vec2 fragCoord, vec2 pixelCoord, vec2 fontSize, float value,
		float digits, float decimals) {
	vec2 charCoord = (fragCoord - pixelCoord) / fontSize;
	if(charCoord.y < 0.0 || charCoord.y >= 1.0) return 0.0;
	float bits = 0.0;
	float digitIndex1 = digits - floor(charCoord.x)+ 1.0;
	if(- digitIndex1 <= decimals) {
		float pow1 = pow(10.0, digitIndex1);
		float absValue = abs(value);
		float pivot = max(absValue, 1.5) * 10.0;
		if(pivot < pow1) {
			if(value < 0.0 && pivot >= pow1 * 0.1) bits = 1792.0;
		} else if(digitIndex1 == 0.0) {
			if(decimals > 0.0) bits = 2.0;
		} else {
			value = digitIndex1 < 0.0 ? fract(absValue) : absValue * 10.0;
			bits = DigitBin(int (mod(value / pow1, 10.0)));
		}
	}
	return floor(mod(bits / pow(2.0, floor(fract(charCoord.x) * 4.0) + floor(charCoord.y * 5.0) * 4.0), 2.0));
}

// Multiples of 4x5 work best
vec2 fontSize = vec2(4,5) * vec2(5,3);

vec2 grid(int x, int y) { return fontSize.xx * vec2(1,ceil(fontSize.y/fontSize.x)) * vec2(x,y) + vec2(2.0,2.0); }

void main()
{
	// Multiples of 4x5 work best
	vec2 vFontSize = vec2(8.0, 15.0);

    vec2 texcoord = gl_FragCoord.xy / u_viewRect.zw;
    vec4 result = texture2D(s_texColor, texcoord);
    result.rgb *= u_exposure;

    switch(u_tonemappingMode)
    {
        default:
        case TONEMAP_NONE:
            result.rgb = saturate(result.rgb);
            break;
        case TONEMAP_EXPONENTIAL:
            result.rgb = tonemap_exponential(result.rgb);
            break;
        case TONEMAP_REINHARD:
            result.rgb = tonemap_reinhard(result.rgb);
            break;
        case TONEMAP_REINHARD_LUM:
            result.rgb = tonemap_reinhard_luminance(result.rgb);
            break;
        case TONEMAP_HABLE:
            result.rgb = tonemap_hable(result.rgb);
            break;
        case TONEMAP_DUIKER:
            result.rgb = tonemap_duiker(result.rgb);
            break;
        case TONEMAP_ACES:
            result.rgb = tonemap_aces(result.rgb);
            break;
        case TONEMAP_ACES_LUM:
            result.rgb = tonemap_aces_luminance(result.rgb);
            break;
    }

	result.rgb = mix( result.rgb , vec3(1.0, 0.0, 1.0), PrintValue(gl_FragCoord.xy, vec2(184.0, 5.0), vFontSize, u_exposure, 2.0, 0.0));
	result.rgb = mix( result.rgb , vec3(1.0, 0.0, 1.0), PrintValue(gl_FragCoord.xy, vec2(184.0 + 48.0, 5.0), vFontSize, u_tonemappingMode, 2.0, 0.0));

    gl_FragColor = result;
	}
