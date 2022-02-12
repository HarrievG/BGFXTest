#pragma once

#include "Renderer.h"

class ForwardRenderer : public Renderer
{
public:
    ForwardRenderer(const Scene* scene);

    static bool supported();

    virtual void onInitialize() override;
    virtual void onRender(float dt) override;
    virtual void onShutdown() override;

private:
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
};
