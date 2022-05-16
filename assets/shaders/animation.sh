/*
 * Copyright 2021-2022 Harrie van Ginneken. All rights reserved.
 */

#include <bgfx_shader.sh>
#include "shaderlib.sh"

#ifdef USE_SKINNING

uniform mat4 u_boneMatrices[128];

mat4 getSkinningMatrix(vec4 _joints_0, vec4 _weights_0 )
{
    mat4 skin = mat4(vec4(1.0,0.0,0.0,0.0),vec4(0.0,1.0,0.0,0.0),vec4(0.0,0.0,1.0,0.0),vec4(0.0,0.0,0.0,1.0));

    skin +=
        _weights_0.x * u_boneMatrices[int(_joints_0.x)] +
        _weights_0.y * u_boneMatrices[int(_joints_0.y)] +
        _weights_0.z * u_boneMatrices[int(_joints_0.z)] +
        _weights_0.w * u_boneMatrices[int(_joints_0.w)];

    return skin;
}

mat4 getSkinningNormalMatrix(vec4 _joints_0 )
{
    mat4 skin = mat4(vec4(1.0,0.0,0.0,0.0),vec4(0.0,1.0,0.0,0.0),vec4(0.0,0.0,1.0,0.0),vec4(0.0,0.0,0.0,1.0));
    #if defined(HAS_WEIGHTS_0_VEC4) && defined(HAS_JOINTS_0_VEC4)
    skin +=
        a_weights_0.x * u_jointNormalMatrix[int(a_joints_0.x)] +
        a_weights_0.y * u_jointNormalMatrix[int(a_joints_0.y)] +
        a_weights_0.z * u_jointNormalMatrix[int(a_joints_0.z)] +
        a_weights_0.w * u_jointNormalMatrix[int(a_joints_0.w)];
    #endif

    #if defined(HAS_WEIGHTS_1_VEC4) && defined(HAS_JOINTS_1_VEC4)
    skin +=
        a_weights_1.x * u_jointNormalMatrix[int(a_joints_1.x)] +
        a_weights_1.y * u_jointNormalMatrix[int(a_joints_1.y)] +
        a_weights_1.z * u_jointNormalMatrix[int(a_joints_1.z)] +
        a_weights_1.w * u_jointNormalMatrix[int(a_joints_1.w)];
    #endif

    return skin;
}

#endif