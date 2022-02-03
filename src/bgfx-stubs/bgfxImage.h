#pragma once
#include "bgfxRender.h"

struct imageLoad_t {
	byte *data;
	size_t length;
	bgfxTextureHandle * targetHandle;
};

void bgfxStartImageLoadThread();
bgfxTextureHandle bgfxImageLoad( byte *data, size_t length );
void bgfxImageLoadAsync( byte *data, size_t length, bgfxTextureHandle *targetHandle );