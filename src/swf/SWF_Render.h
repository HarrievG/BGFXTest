/*
* Copyright 2013 Jeremie Roy. All rights reserved.
* License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
*/

#ifndef	__SWF_RENDER_H__
#define	__SWF_RENDER_H__
#include "bx/handlealloc.h"

class swfRenderer
{
public:
	swfRenderer();
	~swfRenderer(){};
private:

	bgfx::VertexLayout m_vertexLayout;
	bgfx::UniformHandle s_texColor;
	bgfx::UniformHandle u_dropShadowColor;
	bgfx::UniformHandle u_params;
};

#endif // __SWF_RENDER_H__
