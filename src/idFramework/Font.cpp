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

const char * DEFAULT_FONT = "open-sans-regular.ttf";
idCVar font_glyph_ondemand_render( "font_glyph_ondemand_render", "0", CVAR_ARCHIVE | CVAR_GUI| CVAR_INIT| CVAR_BOOL, " 0: All glyphs within font are created on load.\n 1 : Font glyphs are rendered to the fontatlas when needed \n" );

static const float old_scale2 = 0.6f;
static const float old_scale1 = 0.3f;

FT_Library ftLibrary;

#define _FLOOR(x)  ((x) & -64)
#define _CEIL(x)   (((x)+63) & -64)
#define _TRUNC(x)  ((x) >> 6)

idStr					idFont::fontLang;
idHashIndex				idFont::fontInfoIndices;
idList<FontInfo>		idFont::fontInfos;
GlyphInfo				idFont::blackGlyph;
idList<idFont *>		idFont::fonts;
idHashIndex				idFont::fontIndices;

GlyphInfo &idFont::GetUnicodeGlyphInfo(int fontHash, unsigned int code )
{
	if (alias!=nullptr )
		 return alias->GetUnicodeGlyphInfo(fontHash,code);
	
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

/*
==============================
idFont::RemapFont
==============================
*/
idFont * idFont::RemapFont( const char * baseName ) {
	idStr cleanName;
	cleanName += baseName;

	if ( cleanName == DEFAULT_FONT ) {
		return NULL;
	}

	const char * remapped = idLocalization::FindString( "#font_" + cleanName );
	if ( remapped != NULL ) {
		return idFont::RegisterFont( remapped );
	}

	const char * wildcard = idLocalization::FindString( "#font_*" );
	if ( wildcard != NULL && cleanName.Icmp( wildcard ) != 0 ) {
		return idFont::RegisterFont( wildcard );
	}

	// Note single | so both sides are always executed
	if ( cleanName.ReplaceChar( ' ', '_' ) | cleanName.ReplaceChar( '-', '_' ) ) {
		return idFont::RegisterFont( cleanName );
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
	common->DWarning("DELETE ALL OF ME PLS");
}

void idFont::InitFreetype( ) 
{
	FT_Error ret = FT_Init_FreeType( &ftLibrary );
	if ( ret )
		common->Warning( "idFont: Cannot initialize freetype" );

}

idFont * idFont::RegisterFont( const char *fontName ) 
{
	int fontHash = GetFontHash( fontName );
	int index = fontIndices.First( fontHash );
	if ( index == -1 ) 	{
		index = fonts.Num();
		fonts.AssureSizeAlloc( fonts.Num()+1,idListNewElement<idFont>); 
		fontIndices.Add( fontHash, index );
		fonts[index]->Init(fontName);
	}
	return fonts[index];
}


void idFont::Init(const char * n)
{
	setName(n);
	fontInfo = NULL;
	alias = RemapFont( n );

	if ( alias != NULL ) {
		// Make sure we don't have a circular reference
		for ( idFont * f = alias; f != NULL; f = f->alias ) {
			if ( f == this  ) {
				common->FatalError( "Font alias \"%s\" is a circular reference!", n );
			}
		}
		
		return;
	}

	if ( !LoadFont() ) {
		if ( idStr::Icmp(n,( DEFAULT_FONT )) == 0 ) {
			common->FatalError( "idFont: Could not load default font \"%s\"", DEFAULT_FONT );
		} else {
			idLib::Warning( "idFont: Could not load font %s , trying default %s", n, DEFAULT_FONT);
			alias = RegisterFont( DEFAULT_FONT );
		}
	}
}

/*
==============================
idFont::LoadFont
==============================
*/
bool idFont::LoadFont() {
	name =  va( "fonts/%s", GetName() );
	bool ret = RenderFont();
	
	return ret;
}

/*
==============================
idFont::GetGlyphIndex
==============================
*/
int	idFont::GetGlyphIndex( uint32 idx ) const {
	
	return unicodePoints.FindIndex( idx );
	
	// hvg todo
	// 	   bring back ascii optimize
	//if ( idx < 128 ) {
	//	return fontInfo->ascii[idx];
	//}
	//if ( fontInfo->numGlyphs == 0 ) {
	//	return -1;
	//}
	//if ( fontInfo->charIndex == NULL ) {
	//	return idx;
	//}
	//int len = fontInfo->numGlyphs;
	//int mid = fontInfo->numGlyphs;
	//int offset = 0;
	//while ( mid > 0 ) {
	//	mid = len >> 1;
	//	if ( fontInfo->charIndex[offset+mid] <= idx ) {
	//		offset += mid;
	//	}
	//	len -= mid;
	//}
	//return ( fontInfo->charIndex[offset] == idx ) ? offset : -1;
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
	if ( fontInfos.Num() >= fontInfoIndex ) {
		return scale * fontInfos[fontInfoIndex].maxHeight;
	}

	return 0.0f;
}


/*
==============================
idFont::GetDecender
==============================
*/
float idFont::GetDecender( float scale ) const {
	if ( alias != NULL ) {
		return alias->GetDecender( scale );
	}
	if ( fontInfos.Num( ) >= fontInfoIndex ) {
		return scale * fontInfos[fontInfoIndex].descender;
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
	if ( fontInfos.Num() >= fontInfoIndex ) {
		return scale *  fontInfos[fontInfoIndex].ascender;
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
		return scale *  fontInfos[fontInfoIndex].maxAdvanceWidth;
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
	if ( fontInfos.Num() >= fontInfoIndex ) {
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
	//common->Warning("idFont: GetScaledGlyph WILL FAIL! fontInfo->material->GetImageWidth()" );
	if ( alias != NULL ) {
		return alias->GetScaledGlyph( scale, idx, glyphInfo );
	}
	if ( glyphInfos.Num() ) {
		int i = GetGlyphIndex( idx );
		const int asterisk = 42;
		if ( i == -1 && idx != asterisk ) {
			i = GetGlyphIndex( asterisk );
		}
		if ( i >= 0 ) {
			float invMaterialWidth = 1.0f / 1;//fontInfo->material->GetImageWidth();
			float invMaterialHeight = 1.0f / 1;//fontInfo->material->GetImageHeight();
			const GlyphInfo & gi = glyphInfos[i];
			glyphInfo.xSkip = scale * gi.advance_x;
			glyphInfo.top = scale * gi.offset_y;
			glyphInfo.left = scale *  gi.offset_x;
			glyphInfo.width = scale * gi.width;
			glyphInfo.height = scale * gi.height;
			//glyphInfo.s1 = ( gi.s - 0.5f ) * invMaterialWidth;
			//glyphInfo.t1 = ( gi.t - 0.5f ) * invMaterialHeight;
			//glyphInfo.s2 = ( gi.s + gi.width + 0.5f ) * invMaterialWidth;
			//glyphInfo.t2 = ( gi.t + gi.height + 0.5f ) * invMaterialHeight;
			//glyphInfo.material = fontInfo->material;
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

bool idFont::RenderFont()
{
	FT_Face face;

	void *faceData;
	ID_TIME_T ftime;
	int len, fontCount;

	if (ftLibrary == NULL) {
		common->Warning( "RenderFont: FreeType not initialized." );
		return false;
	}

	len = fileSystem->ReadFile(name, &faceData, &ftime);
	if ( len <= 0 ) {
		common->Warning( "RenderFont: Unable to read font file" );
		return false;
	}

	// allocate on the stack first in case we fail
	if ( FT_New_Memory_Face( ftLibrary, (FT_Byte*)faceData, len, 0, &face ) ) {
		common->Warning( "RenderFont: FreeType2, unable to allocate new face." );
		return false;
	}

	int pointSize = 24;
	int dpi = 144;
	if ( FT_Set_Char_Size( face, 0, pointSize << 6, dpi, dpi) ) {
		common->Warning( "RenderFont: FreeType2, Unable to set face char size." );
		return false;
	}

	int fontHash = GetFontHash(name);
	int tmpFontInfoCnt = fontInfos.Num();
	FontInfo & font = GetFontInfo(fontHash,true);
	if (tmpFontInfoCnt != fontInfos.Num() )
		fontInfoIndex = tmpFontInfoCnt;

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

		// Create tofu character ;P
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
		atlas = new Atlas( );
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

			if (glyphInfo.regionIndex == UINT16_MAX )
				common->Warning("idFont :  %s : Failed to add region %i to Atlas \nloaded %i glyphs , rendered %i , atlas usage %f",name.c_str(),charcode,charCount,charRenderCount,atlas->getTotalRegionUsage());

			maxHeight = Max((float)maxHeight, glyphInfo.height);
		}

		charcode = FT_Get_Next_Char( face, charcode, &gindex );

	}
	font.maxHeight = maxHeight;

	common->Printf("IdFont :  %s : loaded %i glyphs , rendered %i , atlas usage %g %% \n",name.c_str(),charCount,charRenderCount,atlas->getTotalRegionUsage());

	fileSystem->FreeFile( faceData );
	return true;
}