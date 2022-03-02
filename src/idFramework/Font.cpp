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

#include "ft2build.h"
#include FT_FREETYPE_H

#include "freetype/ftsystem.h"
#include "freetype/ftimage.h"
#include "freetype/fterrors.h"
#include "freetype/ftsystem.h"
#include "freetype/ftimage.h"
#include "freetype/ftoutln.h"

#include "idlib/LangDict.h"
#include "FileSystem.h"
#include "idlib/Swap.h"
#include "Font.h"
#include "Common.h"
#include "idlib/lib.h"

const char * DEFAULT_FONT = "Arial_Narrow";

static const float old_scale2 = 0.6f;
static const float old_scale1 = 0.3f;


FT_Library ftLibrary;



#define _FLOOR(x)  ((x) & -64)
#define _CEIL(x)   (((x)+63) & -64)
#define _TRUNC(x)  ((x) >> 6)

idList<fontInfoEx_t>	idFont::fonts;
fontInfoEx_t	*		idFont::activeFont;
fontInfo_t		*		idFont::useFont;
idStr					idFont::fontName;
idStr					idFont::fontLang;
idHashIndex				idFont::fontInfoIndices;
idList<FontInfo>		idFont::fontInfos;
GlyphInfo				idFont::blackGlyph;


GlyphInfo &idFont::GetUnicodeGlyphInfo(int fontHash, unsigned int code )
{
	int charIndex  = unicodePoints.FindIndex(code);
	if ( charIndex == -1 )
		return blackGlyph;
	return glyphInfos[charIndex];
}

int idFont::GetFontHash(const char * name )
{
	return fontInfoIndices.GenerateKey( name );
}


FontInfo & idFont::GetFontInfo( int fontHash, bool create)
{
	int index = fontInfoIndices.First(fontHash);
	if (index == -1 && create )
	{
		index = fontInfos.Num();
		fontInfos.Alloc(  );
		fontInfoIndices.Add( fontHash, index );
	}
	return fontInfos[index];

}


//unsigned int &idFont::GetGlyphIndex( int fontHash, unsigned long code, bool create = false);
//
//	//int hash = glyphInfoIndices.GenerateKey( fontHash, code );
//	//int index = glyphInfoIndices.First( hash );
//	//if ( index == -1 ) 	{
//	//	index = glyphInfos.Append( GlyphInfo( ) );
//	//	glyphInfoIndices.Add( hash, index );
//	//}
//
//	return charIndices[0];
//}

/*
==============================
Old_SelectValueForScale
==============================
*/
ID_INLINE float Old_SelectValueForScale( float scale, float v0, float v1, float v2 ) {
	return ( scale >= old_scale2 ) ? v2 : ( scale >= old_scale1 ) ? v1 : v0;
}

/*
==============================
idFont::RemapFont
==============================
*/
idFont * idFont::RemapFont( const char * baseName ) {
	idStr cleanName = baseName;

	if ( cleanName == DEFAULT_FONT ) {
		return NULL;
	}

	const char * remapped = idLocalization::FindString( "#font_" + cleanName );
	if ( remapped != NULL ) {
		//return renderSystem->RegisterFont( remapped );
	}

	const char * wildcard = idLocalization::FindString( "#font_*" );
	if ( wildcard != NULL && cleanName.Icmp( wildcard ) != 0 ) {
		//return renderSystem->RegisterFont( wildcard );
	}

	// Note single | so both sides are always executed
	if ( cleanName.ReplaceChar( ' ', '_' ) | cleanName.ReplaceChar( '-', '_' ) ) {
		//return renderSystem->RegisterFont( cleanName );
	}

	return NULL;
}

/*
==============================
idFont::~idFont
==============================
*/
idFont::~idFont() {
	delete fontInfo;
}

void idFont::InitFreetype( ) 
{
		FT_Error ret = FT_Init_FreeType(&ftLibrary);
		if (ret)
			common->Warning("Cannot initialize freetype" );

}

void idFont::RegisterFont( const char *fontName, fontInfoEx_t &font ) 
{

}

/*
==============================
idFont::idFont
==============================
*/
idFont::idFont( const char * n ) : name( n ) {
	fontInfo = NULL;
	alias = RemapFont( n );

	if ( alias != NULL ) {
		// Make sure we don't have a circular reference
		for ( idFont * f = alias; f != NULL; f = f->alias ) {
			if ( f == this ) {
				common->FatalError( "Font alias \"%s\" is a circular reference!", n );
			}
		}
		return;
	}

	if ( !LoadFont() ) {
		if ( name.Icmp( DEFAULT_FONT ) == 0 ) {
			common->FatalError( "Could not load default font \"%s\"", DEFAULT_FONT );
		} else {
			idLib::Warning( "Could not load font %s", n );
			//alias = renderSystem->RegisterFont( DEFAULT_FONT );
		}
	}
}

struct oldGlyphInfo_t {
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
	int					junk;
	char				materialName[32];
};

/*
==============================
LoadOldGlyphData
==============================
*/
bool LoadOldGlyphData( const char * filename, oldGlyphInfo_t glyphInfo[GLYPHS_PER_FONT] ) {
	idFile * fd = fileSystem->OpenFileRead( filename );
	if ( fd == NULL ) {
		return false;
	}
	fd->Read( glyphInfo, GLYPHS_PER_FONT * sizeof( oldGlyphInfo_t ) );
	for ( int i = 0; i < GLYPHS_PER_FONT; i++ ) {
		idSwap::Little( glyphInfo[i].height );
		idSwap::Little( glyphInfo[i].top );
		idSwap::Little( glyphInfo[i].bottom );
		idSwap::Little( glyphInfo[i].pitch );
		idSwap::Little( glyphInfo[i].xSkip );
		idSwap::Little( glyphInfo[i].imageWidth );
		idSwap::Little( glyphInfo[i].imageHeight );
		idSwap::Little( glyphInfo[i].s );
		idSwap::Little( glyphInfo[i].t );
		idSwap::Little( glyphInfo[i].s2 );
		idSwap::Little( glyphInfo[i].t2 );
		assert( glyphInfo[i].imageWidth == glyphInfo[i].pitch );
		assert( glyphInfo[i].imageHeight == glyphInfo[i].height );
		assert( glyphInfo[i].imageWidth == ( glyphInfo[i].s2 - glyphInfo[i].s ) * 256 );
		assert( glyphInfo[i].imageHeight == ( glyphInfo[i].t2 - glyphInfo[i].t ) * 256 );
		assert( glyphInfo[i].junk == 0 );
	}
	delete fd;
	return true;
}

/*
==============================
idFont::LoadFont
==============================
*/
bool idFont::LoadFont() {
	fontName =  va( "fonts/%s", GetName() );
	RenderFont();
	
	return true;
}

/*
==============================
idFont::GetGlyphIndex
==============================
*/
int	idFont::GetGlyphIndex( uint32 idx ) const {
	if ( idx < 128 ) {
		return fontInfo->ascii[idx];
	}
	if ( fontInfo->numGlyphs == 0 ) {
		return -1;
	}
	if ( fontInfo->charIndex == NULL ) {
		return idx;
	}
	int len = fontInfo->numGlyphs;
	int mid = fontInfo->numGlyphs;
	int offset = 0;
	while ( mid > 0 ) {
		mid = len >> 1;
		if ( fontInfo->charIndex[offset+mid] <= idx ) {
			offset += mid;
		}
		len -= mid;
	}
	return ( fontInfo->charIndex[offset] == idx ) ? offset : -1;
}

/*
==============================
idFont::GetLineHeight
==============================
*/
float idFont::GetLineHeight( float scale ) const {
	if ( alias != NULL ) {
		return alias->GetLineHeight( scale );
	}
	if ( fontInfo != NULL ) {
		return scale * Old_SelectValueForScale( scale, fontInfo->oldInfo[0].maxHeight, fontInfo->oldInfo[1].maxHeight, fontInfo->oldInfo[2].maxHeight );
	}
	return 0.0f;
}

/*
==============================
idFont::GetAscender
==============================
*/
float idFont::GetAscender( float scale ) const {
	if ( alias != NULL ) {
		return alias->GetAscender( scale );
	}
	if ( fontInfo != NULL ) {
		return scale * fontInfo->ascender;
	}
	return 0.0f;
}

/*
==============================
idFont::GetMaxCharWidth
==============================
*/
float idFont::GetMaxCharWidth( float scale ) const {
	if ( alias != NULL ) {
		return alias->GetMaxCharWidth( scale );
	}
	if ( fontInfo != NULL ) {
		return scale * Old_SelectValueForScale( scale, fontInfo->oldInfo[0].maxWidth, fontInfo->oldInfo[1].maxWidth, fontInfo->oldInfo[2].maxWidth );
	}
	return 0.0f;
}

/*
==============================
idFont::GetGlyphWidth
==============================
*/
float idFont::GetGlyphWidth( float scale, uint32 idx ) const {
	if ( alias != NULL ) {
		return alias->GetGlyphWidth( scale, idx );
	}
	if ( fontInfo != NULL ) {
		int i = GetGlyphIndex( idx );
		const int asterisk = 42;
		if ( i == -1 && idx != asterisk ) {
			i = GetGlyphIndex( asterisk );
		}
		if ( i >= 0 ) {
			return scale * fontInfo->glyphData[i].xSkip;
		}
	}
	return 0.0f;
}

/*
==============================
idFont::GetScaledGlyph
==============================
*/
void idFont::GetScaledGlyph( float scale, uint32 idx, scaledGlyphInfo_t & glyphInfo ) const {
	common->Warning("GetScaledGlyph WILL FAIL! fontInfo->material->GetImageWidth()" );
	if ( alias != NULL ) {
		return alias->GetScaledGlyph( scale, idx, glyphInfo );
	}
	if ( fontInfo != NULL ) {
		int i = GetGlyphIndex( idx );
		const int asterisk = 42;
		if ( i == -1 && idx != asterisk ) {
			i = GetGlyphIndex( asterisk );
		}
		if ( i >= 0 ) {
			float invMaterialWidth = 1.0f / 1;//fontInfo->material->GetImageWidth();
			float invMaterialHeight = 1.0f / 1;//fontInfo->material->GetImageHeight();
			glyphInfo_t & gi = fontInfo->glyphData[i];
			glyphInfo.xSkip = scale * gi.xSkip;
			glyphInfo.top = scale * gi.top;
			glyphInfo.left = scale * gi.left;
			glyphInfo.width = scale * gi.width;
			glyphInfo.height = scale * gi.height;
			glyphInfo.s1 = ( gi.s - 0.5f ) * invMaterialWidth;
			glyphInfo.t1 = ( gi.t - 0.5f ) * invMaterialHeight;
			glyphInfo.s2 = ( gi.s + gi.width + 0.5f ) * invMaterialWidth;
			glyphInfo.t2 = ( gi.t + gi.height + 0.5f ) * invMaterialHeight;
			glyphInfo.material = fontInfo->material;
			return;
		}
	}
	memset( &glyphInfo, 0, sizeof( glyphInfo ) );
}

/*
==============================
idFont::Touch
==============================
*/
void idFont::Touch() {
	if ( alias != NULL ) {
		alias->Touch();
	}
	if ( fontInfo != NULL ) {
		//const_cast<idMaterial *>( fontInfo->material )->EnsureNotPurged();
		//fontInfo->material->SetSort( SS_GUI );
	}
}

void idFont::RenderFont()
{
	FT_Face face;

	void *faceData;
	ID_TIME_T ftime;
	int len, fontCount;

	if (ftLibrary == NULL) {
		common->Warning( "RegisterFont: FreeType not initialized." );
		return;
	}
	name = fontName;
	len = fileSystem->ReadFile(fontName, &faceData, &ftime);
	if ( len <= 0 ) {
		common->Warning( "RegisterFont: Unable to read font file" );
		return;
	}

	// allocate on the stack first in case we fail
	if ( FT_New_Memory_Face( ftLibrary, (FT_Byte*)faceData, len, 0, &face ) ) {
		common->Warning( "RegisterFont: FreeType2, unable to allocate new face." );
		return;
	}

	int pointSize = 24;
	int dpi = 144;
	if ( FT_Set_Char_Size( face, 0, pointSize << 6, dpi, dpi) ) {
		common->Warning( "RegisterFont: FreeType2, Unable to set face char size." );
		return;
	}

	if (useFont == nullptr )
		useFont = new ::fontInfo_t;

	int fontHash = GetFontHash(fontName);
	FontInfo & font = GetFontInfo(fontHash,true);

	font.ascender = (float)(face->size->metrics.ascender  >> 6);
	font.descender = (float)(face->size->metrics.descender >> 6);
	font.maxAdvanceWidth = face->size->metrics.max_advance >> 6;
	font.underlineThickness = 1.0;
	font.underlinePosition = 0.0;
	font.lineGap = 1.0f;
	font.scale = 1.0f;
	font.fontType = FONT_TYPE_ALPHA;

	 if (atlas == nullptr )
	 {
		const uint32_t W = 3;

		// Create filler rectangle , its called a tofu character ;P
		uint8_t buffer[W * W * 4];
		bx::memSet( buffer, 255, W * W * 4 );

		blackGlyph.width = 0;
		blackGlyph.height = 0;
		blackGlyph.advance_x = 0.0f;
		blackGlyph.advance_y= 0.0f;
		blackGlyph.bitmapScale= 1.0f;
		blackGlyph.glyphIndex=-1;
		blackGlyph.offset_x=0.0f;
		blackGlyph.offset_y=0.0f;
		atlas = new Atlas( 512 );
		///make sure the black glyph doesn't bleed by using a one pixel inner outline
		blackGlyph.regionIndex = atlas->addRegion( W, W, buffer, AtlasRegion::TYPE_GRAY, 1 );
	 }

	FT_ULong  charcode;
	FT_UInt   gindex;

	if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
		common->FatalError("Could not select charmap");

	glyphInfos.Resize(face->num_glyphs);
	unicodePoints.Resize(face->num_glyphs);

	int maxHeight = 0;
	charcode = FT_Get_First_Char( face, &gindex );
	int charCount = 0;
	int charRenderCount = 0;
	while ( gindex != 0 )
	{
		charCount++;
		FT_Load_Glyph(face, gindex, FT_LOAD_COLOR );
		
		FT_Render_Glyph(face->glyph,FT_RENDER_MODE_NORMAL );

		auto &glyphInfo = glyphInfos.Alloc( );
		auto &up = unicodePoints.Alloc( );
		up = ( int ) charcode;

		glyphInfo.bitmapScale = 1.0f;

		glyphInfo.width = ( short ) ( ( face->glyph->metrics.width ) >> 6 );
		glyphInfo.height = ( short ) ( ( face->glyph->metrics.height ) >> 6 );
		glyphInfo.offset_x = ( short ) ( ( face->glyph->metrics.horiBearingX ) >> 6 );
		glyphInfo.offset_y = ( short ) ( ( face->size->metrics.ascender - face->glyph->metrics.horiBearingY ) >> 6 );
		glyphInfo.advance_x = ( short ) ( ( face->glyph->metrics.horiAdvance ) >> 6 );
		glyphInfo.advance_y = ( short ) ( ( face->glyph->metrics.vertAdvance ) >> 6 );
		glyphInfo.regionIndex = 0; // should always be tofu

		if (face->glyph->bitmap.buffer != NULL )
		{
			charRenderCount++;
			//common->DPrintf("Codepoint: {%llu}, gid: {%i}\n", charcode, gindex);
			glyphInfo.regionIndex = atlas->addRegion( glyphInfo.width, glyphInfo.height, face->glyph->bitmap.buffer, AtlasRegion::TYPE_GRAY );
			maxHeight = Max((float)maxHeight, glyphInfo.height);
		}

		charcode = FT_Get_Next_Char( face, charcode, &gindex );

	}
	common->Printf("IdFont  %s : loaded %i glyphs , rendererd %i\n",fontName.c_str(),charCount,charRenderCount);

	fileSystem->FreeFile( faceData );
}