$input v_worldpos, v_normal, v_tangent, v_texcoord
// all unit-vectors need to be normalized in the fragment shader, the interpolation of vertex shader output doesn't preserve length

// define samplers and uniforms for retrieving material parameters
#define READ_MATERIAL

#include "common.sh"
#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "util.sh"
#include "pbr.sh"
#include "lights.sh"
#include "lights_punctual.sh"

uniform vec4 u_camPos;
void main()
{
    PBRMaterial mat = pbrMaterial(v_texcoord);
    // convert normal map from tangent space -> world space (= space of v_tangent, etc.)
    vec3 N =convertTangentNormal(v_normal, v_tangent.xyz, mat.normal);
    mat.a = specularAntiAliasing(N, mat.a);

    // shading

    vec3 camPos = u_camPos.xyz;
    vec3 fragPos = v_worldpos;

    vec3 V = normalize(camPos - fragPos);
    float NoV = abs(dot(N, V)) + 1e-5;

    if(whiteFurnaceEnabled())
    {
        mat.F0 = vec3_splat(1.0);
        vec3 msFactor = multipleScatteringFactor(mat, NoV);
        vec3 radianceOut = whiteFurnace(NoV, mat) * msFactor;
        gl_FragColor = vec4(radianceOut, 1.0);
        return;
    }

    vec3 msFactor = multipleScatteringFactor(mat, NoV);

    vec3 radianceOut = vec3_splat(0.0);

    uint lights = pointLightCount();
    for(uint i = 0; i < lights; i++)
    {
		Light pLight = getLight(i);

		
        vec3 pointToLight;
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
			vec3 L = normalize(pointToLight);
			vec3 intensity = getLighIntensity(pLight, pointToLight);
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
				vec3 L = normalize(light.position - fragPos);
		
				vec3 radianceIn = light.intensity * attenuation;
				float NoL = saturate(dot(N, L));
				radianceOut += BRDF(V, L, N, NoV, NoL, mat) * msFactor * radianceIn * NoL;
			}
		}

    }

    radianceOut += getAmbientLight().irradiance * mat.diffuseColor * mat.occlusion;
    radianceOut += mat.emissive;

    // output goes straight to HDR framebuffer, no clamping
    // tonemapping happens in final blit

    gl_FragColor.rgb = radianceOut;
    gl_FragColor.a = mat.albedo.a;

	//normal debug
	//gl_FragColor.rgb = (v_normal + 1.0) / 2.0;
	//gl_FragColor.rgb = (v_normal + N + 1.0) / 2.0; 
	//gl_FragColor.rgb = v_tangent * 0.5 + vec3_splat(0.5);
	gl_FragColor.rgb= pbrBaseColor(v_texcoord);
	//gl_FragColor.rgb = vec3_splat(mat.roughness);
	//gl_FragColor.rgb = vec3_splat(mat.metallic);
}
	