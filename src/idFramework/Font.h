/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").  

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#ifndef __FONT_H__
#define __FONT_H__
#include "../bgfx-stubs/cube_atlas.h"
#include "../bgfx-stubs/font/font_manager.h"
#include "idlib/containers/HashTable.h"

#define BUILD_FREETYPE



struct scaledGlyphInfo_t {
	float	top, left;
	float	width, height;
	float	xSkip;
	float	s1, t1, s2, t2;
	const class idMaterial * material;
};

// font support
const int GLYPH_START			= 0;
const int GLYPH_END				= 255;
const int GLYPH_CHARSTART		= 32;
const int GLYPH_CHAREND			= 127;
const int GLYPHS_PER_FONT		= GLYPH_END - GLYPH_START + 1;


typedef struct {
	int					height;			// number of scan lines
	int					top;			// top of glyph in buffer
	int					bottom;			// bottom of glyph in buffer
	int					pitch;			// width for copying
	int					xSkip;			// x adjustment
	int					imageWidth;		// width of actual image
	int					imageHeight;	// height of actual image
	float				s;				// x offset in image where glyph starts
	float				t;				// y offset in image where glyph starts
	float				s2;
	float				t2;
	const idMaterial *	glyph;			// shader with the glyph
	char				shaderName[32];
	uint16_t			regionIndex;
} glyphInfo_t;

typedef struct {
	glyphInfo_t			glyphs [GLYPHS_PER_FONT];
	float				glyphScale;
	char				name[64];
} fontInfo_t;

typedef struct {
	fontInfo_t			fontInfoSmall;
	fontInfo_t			fontInfoMedium;
	fontInfo_t			fontInfoLarge;
	int					maxHeight;
	int					maxWidth;
	int					maxHeightSmall;
	int					maxWidthSmall;
	int					maxHeightMedium;
	int					maxWidthMedium;
	int					maxHeightLarge;
	int					maxWidthLarge;
	char				name[64];
} fontInfoExas_t;

class idFont {
public:
	static void InitFreetype();
	static idFont * RegisterFont( const char *fontName );

	idFont( ):alias(nullptr),atlas(nullptr){};
	~idFont();
	
	void Touch();

	const char * GetName() const {return (alias != nullptr) ? alias->GetName() : this->name.c_str(); }
	void setName(const char * _name ) { name = _name; }
	float GetLineHeight( float scale ) const;
	float GetAscender( float scale ) const;
	float GetDecender( float scale ) const;
	float GetMaxCharWidth( float scale ) const;

	float GetGlyphWidth( float scale, uint32 idx ) const;
	void GetScaledGlyph( float scale, uint32 idx, scaledGlyphInfo_t & glyphInfo ) const;
	const Atlas *GetAtlas( ) const 	{ return (alias != nullptr) ? alias->GetAtlas() : atlas; }
	GlyphInfo & GetUnicodeGlyphInfo(int fontHash, unsigned int code);
private:
	void Init(const char * n);
	static idFont * RemapFont( const char * baseName );

	int	GetGlyphIndex( uint32 idx ) const;

	bool LoadFont();
	bool RenderFont();

	struct glyphInfo_t {
		byte	width;	// width of glyph in pixels
		byte	height;	// height of glyph in pixels
		char	top;	// distance in pixels from the base line to the top of the glyph
		char	left;	// distance in pixels from the pen to the left edge of the glyph
		byte	xSkip;	// x adjustment after rendering this glyph
		uint16	s;		// x offset in image where glyph starts (in pixels)
		uint16	t;		// y offset in image where glyph starts (in pixels)
		uint16_t regionIndex;
	};
	struct fontInfo_t {
		struct oldInfo_t {
			float maxWidth;
			float maxHeight;
		} oldInfo[3];

		short		ascender;
		short		descender;

		short		numGlyphs;
		idFont::glyphInfo_t * glyphData;

		// This is a sorted array of all characters in the font
		// This maps directly to glyphData, so if charIndex[0] is 42 then glyphData[0] is character 42
		uint32 *	charIndex;

		// As an optimization, provide a direct mapping for the ascii character set
		char		ascii[128];
	};

	// base name of the font (minus "fonts/" and ".dat")
	idStr					name;

	// Fonts can be aliases to other fonts
	idFont *				alias;

	// If the font is NOT an alias, this is where the font data is located
	idFont::fontInfo_t * fontInfo;
	int				fontInfoIndex;

	//general interface
	
	static idStr			fontLang;
	Atlas					*atlas;
	idList<unsigned long>	unicodePoints;
	idList<GlyphInfo>		glyphInfos;
public:
	static GlyphInfo		blackGlyph;
	static inline int		GetFontHash(const char * name);
	static FontInfo &		GetFontInfo(int fontHash, bool create = false );
	static idList<FontInfo> fontInfos;
	static idHashIndex		fontInfoIndices;
	static idList<idFont*>	fonts;
	static idHashIndex		fontIndices;
};


#endif
