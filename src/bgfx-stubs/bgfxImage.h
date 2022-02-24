#pragma once
#include "bgfxRender.h"
#include "../idFramework/FileSystem.h"

struct imageLoad_t {
	byte *data;
	size_t length;
	bgfxTextureHandle * targetHandle;
};


class idStretchPic {
public:
	struct Vertex
{
	idVec3 pos;
	uint32_t padding;
	idVec3 normal;
	idVec4b color; // Linear space.
	idVec4 texCoord;

	void setColor(idVec4 c)
	{
		setColor(c.x, c.y, c.z, c.w);
	}

	void setColor(float r, float g, float b, float a)
	{
		color.r = uint8_t( Min( r, 1.0f ) * 255.0f );
		color.g = uint8_t( Min( g, 1.0f ) * 255.0f );
		color.b = uint8_t( Min( b, 1.0f ) * 255.0f );
		color.a = uint8_t( Min( a, 1.0f ) * 255.0f );
	}

	static void init()
	{
		layout.begin();
		layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
		layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
		layout.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
		layout.add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float);
		layout.m_stride = sizeof(Vertex);
		layout.m_offset[bgfx::Attrib::Position] = offsetof(Vertex, pos);
		layout.m_offset[bgfx::Attrib::Normal] = offsetof(Vertex, normal);
		layout.m_offset[bgfx::Attrib::TexCoord0] = offsetof(Vertex, texCoord);
		layout.m_offset[bgfx::Attrib::Color0] = offsetof(Vertex, color);
		layout.end();
	}

	static bgfx::VertexLayout layout;
};
	idStretchPic(){ }

	idVec4 stretchPicColor;
	//Material *stretchPicMaterial = nullptr;
	bgfx::ViewId stretchPicViewId = UINT8_MAX;
	idList<Vertex> stretchPicVertices;
	idList<uint16_t> stretchPicIndices;
};


//
//struct Image {
//	int width = 0;
//	int height = 0;
//	int nComponents = 0;
//	int nMips = 1;
//	uint8_t *data = nullptr;
//	uint32_t dataSize = 0;
//	bgfx::ReleaseFn release = nullptr;
//	int flags = 0;
//};
//
//class Texture {
//public:
//	void resize( int width, int height );
//	void update( const bgfx::Memory *mem, int x, int y, int width, int height );
//	int getFlags( ) const { return flags_; }
//	bgfx::TextureHandle getHandle( ) const { return handle_; }
//	const char *getName( ) const { return name_; }
//	int getWidth( ) const { return width_; }
//	int getHeight( ) const { return height_; }
//
//private:
//	void initialize( const char *name, const Image &image, int flags, bgfx::TextureFormat::Enum format );
//	void initialize( const char *name, bgfx::TextureHandle handle );
//	uint32_t calculateBgfxFlags( ) const;
//
//	char name_[MAX_OSPATH];
//	int flags_;
//	int width_, height_;
//	int nMips_;
//	bgfx::TextureFormat::Enum format_;
//	bgfx::TextureHandle handle_;
//	Texture *next_;
//
//	friend class TextureCache;
//};

void bgfxStartImageLoadThread();
bgfxTextureHandle bgfxImageLoad( byte *data, size_t length );
void bgfxImageLoadAsync( byte *data, size_t length, bgfxTextureHandle * targetHandle );