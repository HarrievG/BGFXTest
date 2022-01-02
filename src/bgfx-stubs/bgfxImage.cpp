#include "bgfxImage.h"
#include "stb_image.h"
#include "gltf-edit/gltfParser.h"

bgfxTextureHandle bgfxImageLoad( byte *data, int length ) {

	int width, height, channels;
	bgfxTextureHandle ret;
	ret.handle.idx = -1;

	stbi_uc * imageData = stbi_load_from_memory( ( stbi_uc const *) data, length,&width,&height,&channels, STBI_rgb_alpha );
	if (imageData != NULL)
	{
		uint32_t tex_flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;//add point and repeat
		ret.handle = bgfx::createTexture2D( width, height, false, 1, bgfx::TextureFormat::RGBA8, tex_flags, bgfx::copy( imageData, width * height * 4 ) );
		ret.dim.x = width;
		ret.dim.y = height;
		stbi_image_free(imageData);
	}
	else
		common->Warning(" FAILED TO LOAD TEXTURE " );

	return ret;
}