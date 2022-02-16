#pragma once

#include <bgfx/bgfx.h>

class gltfData;

class LightShader
{
public:
    void initialize();
    void shutdown();

    void bindLights(const gltfData* sceneData) const;

private:
    bgfx::UniformHandle lightCountVecUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle ambientLightIrradianceUniform = BGFX_INVALID_HANDLE;
};
